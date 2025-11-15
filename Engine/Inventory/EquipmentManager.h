#pragma once

#include "Item.h"
#include "Inventory.h"
#include "Core/Logger.h"
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Equipment Manager - handles equipped items and stat calculations
 * 
 * Features:
 * - Equipment slots (weapon, armor, accessories)
 * - Stat bonuses from equipment
 * - Equipment requirements (level, stats)
 * - Set bonuses (2-piece, 4-piece, etc.)
 * - Visual equipment display
 * 
 * Usage:
 *   EquipmentManager equipment;
 *   equipment.EquipItem(EquipmentSlot::Weapon, "iron_sword");
 *   int totalAttack = equipment.GetTotalAttackBonus();
 */
class EquipmentManager {
public:
    struct EquipmentStats {
        int attack = 0;
        int defense = 0;
        int health = 0;
        int mana = 0;
        int speed = 0;
        int critChance = 0;
        int critDamage = 0;

        EquipmentStats& operator+=(const EquipmentStats& other) {
            attack += other.attack;
            defense += other.defense;
            health += other.health;
            mana += other.mana;
            speed += other.speed;
            critChance += other.critChance;
            critDamage += other.critDamage;
            return *this;
        }
    };

    struct SetBonus {
        std::string setName;
        int piecesRequired;
        EquipmentStats bonus;
        std::string bonusDescription;
    };

    EquipmentManager() = default;

    /**
     * @brief Equip item to slot
     */
    bool EquipItem(EquipmentSlot slot, const std::string& itemID, Entity* entity = nullptr) {
        const Item* item = ItemDatabase::Get().GetItem(itemID);
        if (!item) {
            SAGE_ERROR("EquipmentManager: Item not found: {}", itemID);
            return false;
        }

        if (item->type != ItemType::Equipment) {
            SAGE_ERROR("EquipmentManager: Item is not equipment: {}", itemID);
            return false;
        }

        if (item->equipSlot != slot) {
            SAGE_ERROR("EquipmentManager: Item cannot be equipped in this slot");
            return false;
        }

        // Check level requirements
        if (entity && m_CheckRequirements) {
            // TODO: Check entity level/stats requirements
        }

        // Unequip current item if any
        std::string currentItem = GetEquippedItem(slot);
        if (!currentItem.empty()) {
            UnequipItem(slot, entity);
        }

        // Equip new item
        SetSlot(slot, itemID);

        // Call onEquip callback
        if (item->onEquip && entity) {
            item->onEquip(entity);
        }

        // Recalculate stats
        RecalculateStats();

        SAGE_INFO("EquipmentManager: Equipped '{}' in slot {}", itemID, (int)slot);

        if (m_OnEquipmentChanged) {
            m_OnEquipmentChanged(slot, itemID);
        }

        return true;
    }

    /**
     * @brief Unequip item from slot
     */
    bool UnequipItem(EquipmentSlot slot, Entity* entity = nullptr) {
        std::string itemID = GetEquippedItem(slot);
        if (itemID.empty()) return false;

        const Item* item = ItemDatabase::Get().GetItem(itemID);
        
        // Call onUnequip callback
        if (item && item->onUnequip && entity) {
            item->onUnequip(entity);
        }

        SetSlot(slot, "");
        RecalculateStats();

        SAGE_INFO("EquipmentManager: Unequipped from slot {}", (int)slot);

        if (m_OnEquipmentChanged) {
            m_OnEquipmentChanged(slot, "");
        }

        return true;
    }

    /**
     * @brief Get equipped item in slot
     */
    std::string GetEquippedItem(EquipmentSlot slot) const {
        auto it = m_EquippedItems.find(slot);
        return (it != m_EquippedItems.end()) ? it->second : "";
    }

    /**
     * @brief Check if slot is equipped
     */
    bool IsSlotEquipped(EquipmentSlot slot) const {
        return !GetEquippedItem(slot).empty();
    }

    /**
     * @brief Get total stats from all equipment
     */
    const EquipmentStats& GetTotalStats() const {
        return m_TotalStats;
    }

    /**
     * @brief Get stats from specific slot
     */
    EquipmentStats GetSlotStats(EquipmentSlot slot) const {
        std::string itemID = GetEquippedItem(slot);
        if (itemID.empty()) return EquipmentStats();

        const Item* item = ItemDatabase::Get().GetItem(itemID);
        if (!item) return EquipmentStats();

        EquipmentStats stats;
        stats.attack = item->attackBonus;
        stats.defense = item->defenseBonus;
        stats.health = item->healthBonus;
        stats.mana = item->manaBonus;
        return stats;
    }

    /**
     * @brief Register set bonus
     */
    void RegisterSetBonus(const SetBonus& bonus) {
        m_SetBonuses.push_back(bonus);
    }

    /**
     * @brief Get active set bonuses
     */
    std::vector<SetBonus> GetActiveSetBonuses() const {
        std::vector<SetBonus> activeBonuses;

        // Count pieces per set
        std::unordered_map<std::string, int> setCounts;
        for (const auto& [slot, itemID] : m_EquippedItems) {
            if (itemID.empty()) continue;

            const Item* item = ItemDatabase::Get().GetItem(itemID);
            if (item && !item->name.empty()) {
                // Extract set name from item (e.g., "Knight's Helmet" -> "Knight's")
                // TODO: Add explicit setName field to Item
                std::string setName = ExtractSetName(item->name);
                if (!setName.empty()) {
                    setCounts[setName]++;
                }
            }
        }

        // Check which bonuses are active
        for (const auto& bonus : m_SetBonuses) {
            auto it = setCounts.find(bonus.setName);
            if (it != setCounts.end() && it->second >= bonus.piecesRequired) {
                activeBonuses.push_back(bonus);
            }
        }

        return activeBonuses;
    }

    /**
     * @brief Unequip all items
     */
    void UnequipAll(Entity* entity = nullptr) {
        std::vector<EquipmentSlot> slots = {
            EquipmentSlot::Weapon,
            EquipmentSlot::Head,
            EquipmentSlot::Chest,
            EquipmentSlot::Legs,
            EquipmentSlot::Feet,
            EquipmentSlot::Accessory1,
            EquipmentSlot::Accessory2
        };

        for (auto slot : slots) {
            UnequipItem(slot, entity);
        }
    }

    /**
     * @brief Save/Load
     */
    json ToJson() const {
        json j;
        for (const auto& [slot, itemID] : m_EquippedItems) {
            j[std::to_string((int)slot)] = itemID;
        }
        return j;
    }

    void FromJson(const json& j) {
        m_EquippedItems.clear();
        if (j.is_object()) {
            for (const auto& pair : j.object_items()) {
                int slotInt = std::stoi(pair.first);
                EquipmentSlot slot = static_cast<EquipmentSlot>(slotInt);
                m_EquippedItems[slot] = pair.second.get<std::string>();
            }
        }
        RecalculateStats();
    }

    // Settings
    void SetCheckRequirements(bool check) { m_CheckRequirements = check; }
    
    // Callbacks
    void SetOnEquipmentChanged(std::function<void(EquipmentSlot, const std::string&)> callback) {
        m_OnEquipmentChanged = callback;
    }

private:
    void SetSlot(EquipmentSlot slot, const std::string& itemID) {
        if (itemID.empty()) {
            m_EquippedItems.erase(slot);
        }
        else {
            m_EquippedItems[slot] = itemID;
        }
    }

    void RecalculateStats() {
        m_TotalStats = EquipmentStats();

        // Add stats from each equipped item
        for (const auto& [slot, itemID] : m_EquippedItems) {
            if (itemID.empty()) continue;

            const Item* item = ItemDatabase::Get().GetItem(itemID);
            if (!item) continue;

            EquipmentStats slotStats;
            slotStats.attack = item->attackBonus;
            slotStats.defense = item->defenseBonus;
            slotStats.health = item->healthBonus;
            slotStats.mana = item->manaBonus;

            m_TotalStats += slotStats;
        }

        // Add set bonuses
        auto activeBonuses = GetActiveSetBonuses();
        for (const auto& bonus : activeBonuses) {
            m_TotalStats += bonus.bonus;
        }
    }

    std::string ExtractSetName(const std::string& itemName) const {
        // Simple extraction: take first word
        size_t pos = itemName.find(' ');
        if (pos != std::string::npos) {
            return itemName.substr(0, pos);
        }
        return "";
    }

    std::unordered_map<EquipmentSlot, std::string> m_EquippedItems;
    EquipmentStats m_TotalStats;
    std::vector<SetBonus> m_SetBonuses;
    
    bool m_CheckRequirements = true;
    
    std::function<void(EquipmentSlot, const std::string&)> m_OnEquipmentChanged;
};

} // namespace SAGE
