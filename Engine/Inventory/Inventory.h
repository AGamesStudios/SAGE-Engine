#pragma once

#include "Inventory/Item.h"
#include "Inventory/ItemDatabase.h"
#include "Core/Logger.h"
#include <vector>
#include <algorithm>
#include <functional>

namespace SAGE {

    /**
     * @brief InventorySlot - single slot in inventory grid
     */
    struct InventorySlot {
        ItemStack stack;
        bool isLocked = false;  // Locked slots can't be modified
        
        bool IsEmpty() const { return stack.IsEmpty(); }
        
        json ToJson() const {
            json j;
            j["stack"] = stack.ToJson();
            j["isLocked"] = isLocked;
            return j;
        }
        
        static InventorySlot FromJson(const json& j) {
            InventorySlot slot;
            if (j.contains("stack")) {
                slot.stack = ItemStack::FromJson(j["stack"]);
            }
            slot.isLocked = j.value("isLocked", false);
            return slot;
        }
    };

    /**
     * @brief EquipmentSlots - worn equipment
     */
    struct EquipmentSlots {
        std::string weapon;
        std::string head;
        std::string chest;
        std::string legs;
        std::string feet;
        std::string accessory1;
        std::string accessory2;
        
        std::string* GetSlot(EquipmentSlot slot) {
            switch (slot) {
                case EquipmentSlot::Weapon:     return &weapon;
                case EquipmentSlot::Head:       return &head;
                case EquipmentSlot::Chest:      return &chest;
                case EquipmentSlot::Legs:       return &legs;
                case EquipmentSlot::Feet:       return &feet;
                case EquipmentSlot::Accessory1: return &accessory1;
                case EquipmentSlot::Accessory2: return &accessory2;
                default: return nullptr;
            }
        }
        
        json ToJson() const {
            json j;
            j["weapon"] = weapon;
            j["head"] = head;
            j["chest"] = chest;
            j["legs"] = legs;
            j["feet"] = feet;
            j["accessory1"] = accessory1;
            j["accessory2"] = accessory2;
            return j;
        }
        
        static EquipmentSlots FromJson(const json& j) {
            EquipmentSlots eq;
            eq.weapon = j.value("weapon", "");
            eq.head = j.value("head", "");
            eq.chest = j.value("chest", "");
            eq.legs = j.value("legs", "");
            eq.feet = j.value("feet", "");
            eq.accessory1 = j.value("accessory1", "");
            eq.accessory2 = j.value("accessory2", "");
            return eq;
        }
    };

    /**
     * @brief Inventory - item storage and management
     * 
     * Features:
     * - Slot-based storage (fixed capacity)
     * - Item stacking (respects maxStack)
     * - Equipment management
     * - Weight/capacity limits (optional)
     * - Sort/filter helpers
     * - Event callbacks
     * 
     * Usage:
     *   Inventory inv(20); // 20 slots
     *   inv.AddItem("health_potion", 5);
     *   inv.UseItem(0, player);
     *   inv.EquipItem(3, player);
     */
    class Inventory {
    public:
        // Callbacks
        using ItemAddedCallback = std::function<void(const std::string&, int)>;
        using ItemRemovedCallback = std::function<void(const std::string&, int)>;
        using ItemUsedCallback = std::function<void(const std::string&, Entity*)>;
        using ItemEquippedCallback = std::function<void(const std::string&, Entity*)>;
        
        explicit Inventory(size_t capacity = 20)
            : m_Capacity(capacity)
            , m_MaxWeight(0.0f) // 0 = no weight limit
        {
            m_Slots.resize(capacity);
        }
        
        // Item management
        bool AddItem(const std::string& itemID, int quantity = 1) {
            const Item* item = ItemDatabase::Get().GetItem(itemID);
            if (!item) {
                SAGE_ERROR("Item not found in database: {0}", itemID);
                return false;
            }
            
            // Check weight limit
            if (m_MaxWeight > 0.0f) {
                float newWeight = GetCurrentWeight() + (item->weight * quantity);
                if (newWeight > m_MaxWeight) {
                    SAGE_WARN("Inventory weight limit exceeded!");
                    return false;
                }
            }
            
            int remaining = quantity;
            
            // Try to stack with existing items
            if (item->maxStack > 1) {
                for (auto& slot : m_Slots) {
                    if (slot.isLocked) continue;
                    if (slot.stack.itemID == itemID) {
                        int space = item->maxStack - slot.stack.quantity;
                        if (space > 0) {
                            int toAdd = std::min(remaining, space);
                            slot.stack.quantity += toAdd;
                            remaining -= toAdd;
                            
                            if (remaining <= 0) {
                                break;
                            }
                        }
                    }
                }
            }
            
            // Fill empty slots
            while (remaining > 0) {
                int emptySlot = FindEmptySlot();
                if (emptySlot == -1) {
                    SAGE_WARN("Inventory full! Could only add {0}/{1} of {2}", 
                              quantity - remaining, quantity, itemID);
                    return false;
                }
                
                int toAdd = std::min(remaining, item->maxStack);
                m_Slots[emptySlot].stack = ItemStack(itemID, toAdd);
                remaining -= toAdd;
            }
            
            // Callback
            if (m_OnItemAdded) {
                m_OnItemAdded(itemID, quantity);
            }
            
            SAGE_INFO("Added {0}x {1} to inventory", quantity, item->name);
            return true;
        }
        
        bool RemoveItem(const std::string& itemID, int quantity = 1) {
            int remaining = quantity;
            
            for (auto& slot : m_Slots) {
                if (slot.isLocked) continue;
                if (slot.stack.itemID == itemID && slot.stack.quantity > 0) {
                    int toRemove = std::min(remaining, slot.stack.quantity);
                    slot.stack.quantity -= toRemove;
                    remaining -= toRemove;
                    
                    if (slot.stack.quantity <= 0) {
                        slot.stack = ItemStack(); // Clear slot
                    }
                    
                    if (remaining <= 0) {
                        break;
                    }
                }
            }
            
            if (remaining > 0) {
                SAGE_WARN("Could not remove {0}x {1} (not enough in inventory)", quantity, itemID);
                return false;
            }
            
            // Callback
            if (m_OnItemRemoved) {
                m_OnItemRemoved(itemID, quantity);
            }
            
            return true;
        }
        
        bool UseItem(int slotIndex, Entity* user) {
            if (slotIndex < 0 || slotIndex >= static_cast<int>(m_Slots.size())) {
                return false;
            }
            
            InventorySlot& slot = m_Slots[slotIndex];
            if (slot.IsEmpty()) return false;
            
            const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
            if (!item || !item->isConsumable) {
                SAGE_WARN("Item is not consumable: {0}", slot.stack.itemID);
                return false;
            }
            
            // Call onUse callback
            if (item->onUse) {
                item->onUse(user);
            }
            
            // Callback
            if (m_OnItemUsed) {
                m_OnItemUsed(slot.stack.itemID, user);
            }
            
            // Consume one
            slot.stack.quantity--;
            if (slot.stack.quantity <= 0) {
                slot.stack = ItemStack();
            }
            
            return true;
        }
        
        bool EquipItem(int slotIndex, Entity* user) {
            if (slotIndex < 0 || slotIndex >= static_cast<int>(m_Slots.size())) {
                return false;
            }
            
            InventorySlot& slot = m_Slots[slotIndex];
            if (slot.IsEmpty()) return false;
            
            const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
            if (!item || item->type != ItemType::Equipment) {
                SAGE_WARN("Item is not equipment: {0}", slot.stack.itemID);
                return false;
            }
            
            std::string* equipSlot = m_Equipment.GetSlot(item->equipSlot);
            if (!equipSlot) {
                SAGE_ERROR("Invalid equipment slot for item: {0}", slot.stack.itemID);
                return false;
            }
            
            // Unequip existing item
            if (!equipSlot->empty()) {
                UnequipItem(item->equipSlot, user);
            }
            
            // Equip new item
            *equipSlot = item->id;
            
            // Call onEquip callback
            if (item->onEquip) {
                item->onEquip(user);
            }
            
            // Callback
            if (m_OnItemEquipped) {
                m_OnItemEquipped(item->id, user);
            }
            
            // Remove from inventory
            slot.stack.quantity--;
            if (slot.stack.quantity <= 0) {
                slot.stack = ItemStack();
            }
            
            return true;
        }
        
        bool UnequipItem(EquipmentSlot equipSlot, Entity* user) {
            std::string* slot = m_Equipment.GetSlot(equipSlot);
            if (!slot || slot->empty()) {
                return false;
            }
            
            const Item* item = ItemDatabase::Get().GetItem(*slot);
            if (!item) return false;
            
            // Call onUnequip callback
            if (item->onUnequip) {
                item->onUnequip(user);
            }
            
            // Add back to inventory
            if (!AddItem(*slot, 1)) {
                SAGE_WARN("Inventory full! Cannot unequip {0}", item->name);
                return false;
            }
            
            *slot = "";
            return true;
        }
        
        // Slot operations
        bool MoveItem(int fromSlot, int toSlot) {
            if (fromSlot < 0 || fromSlot >= static_cast<int>(m_Slots.size())) return false;
            if (toSlot < 0 || toSlot >= static_cast<int>(m_Slots.size())) return false;
            if (m_Slots[fromSlot].isLocked || m_Slots[toSlot].isLocked) return false;
            
            std::swap(m_Slots[fromSlot].stack, m_Slots[toSlot].stack);
            return true;
        }
        
        bool SwapItems(int slotA, int slotB) {
            return MoveItem(slotA, slotB);
        }
        
        void SortByType() {
            std::sort(m_Slots.begin(), m_Slots.end(), [](const InventorySlot& a, const InventorySlot& b) {
                if (a.isLocked != b.isLocked) return !a.isLocked;
                if (a.IsEmpty() && !b.IsEmpty()) return false;
                if (!a.IsEmpty() && b.IsEmpty()) return true;
                
                const Item* itemA = ItemDatabase::Get().GetItem(a.stack.itemID);
                const Item* itemB = ItemDatabase::Get().GetItem(b.stack.itemID);
                if (!itemA || !itemB) return false;
                
                return static_cast<int>(itemA->type) < static_cast<int>(itemB->type);
            });
        }
        
        void SortByRarity() {
            std::sort(m_Slots.begin(), m_Slots.end(), [](const InventorySlot& a, const InventorySlot& b) {
                if (a.isLocked != b.isLocked) return !a.isLocked;
                if (a.IsEmpty() && !b.IsEmpty()) return false;
                if (!a.IsEmpty() && b.IsEmpty()) return true;
                
                const Item* itemA = ItemDatabase::Get().GetItem(a.stack.itemID);
                const Item* itemB = ItemDatabase::Get().GetItem(b.stack.itemID);
                if (!itemA || !itemB) return false;
                
                return static_cast<int>(itemA->rarity) > static_cast<int>(itemB->rarity);
            });
        }
        
        // Queries
        bool HasItem(const std::string& itemID, int quantity = 1) const {
            int total = 0;
            for (const auto& slot : m_Slots) {
                if (slot.stack.itemID == itemID) {
                    total += slot.stack.quantity;
                }
            }
            return total >= quantity;
        }
        
        int GetItemCount(const std::string& itemID) const {
            int total = 0;
            for (const auto& slot : m_Slots) {
                if (slot.stack.itemID == itemID) {
                    total += slot.stack.quantity;
                }
            }
            return total;
        }

        int GetItemQuantity(const std::string& itemID) const {
            return GetItemCount(itemID);
        }
        
        int FindEmptySlot() const {
            for (size_t i = 0; i < m_Slots.size(); ++i) {
                if (!m_Slots[i].isLocked && m_Slots[i].IsEmpty()) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }
        
        int GetEmptySlotCount() const {
            int count = 0;
            for (const auto& slot : m_Slots) {
                if (!slot.isLocked && slot.IsEmpty()) {
                    count++;
                }
            }
            return count;
        }
        
        float GetCurrentWeight() const {
            float weight = 0.0f;
            for (const auto& slot : m_Slots) {
                if (!slot.IsEmpty()) {
                    const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
                    if (item) {
                        weight += item->weight * slot.stack.quantity;
                    }
                }
            }
            return weight;
        }
        
        int GetTotalValue() const {
            int value = 0;
            for (const auto& slot : m_Slots) {
                if (!slot.IsEmpty()) {
                    const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
                    if (item) {
                        value += item->value * slot.stack.quantity;
                    }
                }
            }
            return value;
        }
        
        // Access
        const std::vector<InventorySlot>& GetSlots() const { return m_Slots; }
        std::vector<InventorySlot>& GetSlots() { return m_Slots; }
        const InventorySlot& GetSlot(int index) const { return m_Slots[index]; }
        
        const EquipmentSlots& GetEquipment() const { return m_Equipment; }
        EquipmentSlots& GetEquipment() { return m_Equipment; }
        
        size_t GetCapacity() const { return m_Capacity; }
        float GetMaxWeight() const { return m_MaxWeight; }
        void SetMaxWeight(float weight) { m_MaxWeight = weight; }
        
        // Callbacks
        void SetOnItemAdded(ItemAddedCallback callback) { m_OnItemAdded = callback; }
        void SetOnItemRemoved(ItemRemovedCallback callback) { m_OnItemRemoved = callback; }
        void SetOnItemUsed(ItemUsedCallback callback) { m_OnItemUsed = callback; }
        void SetOnItemEquipped(ItemEquippedCallback callback) { m_OnItemEquipped = callback; }
        
        // Serialization
        json ToJson() const {
            json j;
            json slotsArray = json::array();
            for (const auto& slot : m_Slots) {
                slotsArray.push_back(slot.ToJson());
            }
            j["slots"] = slotsArray;
            j["equipment"] = m_Equipment.ToJson();
            j["capacity"] = m_Capacity;
            j["maxWeight"] = m_MaxWeight;
            return j;
        }
        
        void FromJson(const json& j) {
            if (j.contains("capacity")) {
                m_Capacity = j["capacity"].get<size_t>();
                m_Slots.resize(m_Capacity);
            }
            
            if (j.contains("slots") && j["slots"].is_array()) {
                size_t i = 0;
                for (const auto& slotJson : j["slots"]) {
                    if (i >= m_Slots.size()) break;
                    m_Slots[i] = InventorySlot::FromJson(slotJson);
                    i++;
                }
            }
            
            if (j.contains("equipment")) {
                m_Equipment = EquipmentSlots::FromJson(j["equipment"]);
            }
            
            m_MaxWeight = j.value("maxWeight", 0.0f);
        }
        
    private:
        std::vector<InventorySlot> m_Slots;
        EquipmentSlots m_Equipment;
        size_t m_Capacity;
        float m_MaxWeight;
        
        // Callbacks
        ItemAddedCallback m_OnItemAdded;
        ItemRemovedCallback m_OnItemRemoved;
        ItemUsedCallback m_OnItemUsed;
        ItemEquippedCallback m_OnItemEquipped;
    };

} // namespace SAGE
