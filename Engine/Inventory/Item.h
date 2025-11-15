#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Types/Color.h"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

namespace SAGE {

    using json = nlohmann::json;

    // Forward declarations
    class GameObject;
    class Entity;

    /**
     * @brief ItemType - category of item
     */
    enum class ItemType {
        Consumable,      // Health potions, food, buffs
        Equipment,       // Weapons, armor, accessories
        QuestItem,       // Story items
        Material,        // Crafting materials
        Misc             // Junk, sellable items
    };

    /**
     * @brief ItemRarity - visual/value tier
     */
    enum class ItemRarity {
        Common,
        Uncommon,
        Rare,
        Epic,
        Legendary
    };

    /**
     * @brief EquipmentSlot - where equipment can be worn
     */
    enum class EquipmentSlot {
        None,
        Weapon,
        Head,
        Chest,
        Legs,
        Feet,
        Accessory1,
        Accessory2
    };

    /**
     * @brief Item - base item definition
     * 
     * Design: Items are templates (definitions) stored in ItemDatabase.
     * When added to inventory, we store ItemID + quantity, not full Item copies.
     */
    struct Item {
        std::string id;                          // Unique identifier (e.g., "health_potion")
        std::string name;                        // Display name
        std::string description;                 // Tooltip description
        Ref<Texture> icon;                       // Inventory icon
        std::string iconPath;                    // For serialization
        
        ItemType type = ItemType::Misc;
        ItemRarity rarity = ItemRarity::Common;
        EquipmentSlot equipSlot = EquipmentSlot::None;
        
        int maxStack = 1;                        // Max stack size (1 = non-stackable)
        float weight = 0.0f;                     // For weight-based inventory limits
        int value = 0;                           // Sell price in gold
        
        bool isQuestItem = false;                // Quest items can't be dropped/sold
        bool isConsumable = false;               // Can be used from inventory
        
        // Equipment stats (if type == Equipment)
        int attackBonus = 0;
        int defenseBonus = 0;
        int healthBonus = 0;
        int manaBonus = 0;
        
        // Callbacks
        std::function<void(Entity*)> onUse;      // Called when item used
        std::function<void(Entity*)> onEquip;    // Called when equipped
        std::function<void(Entity*)> onUnequip;  // Called when unequipped
        
        Item() = default;
        
        Item(const std::string& itemId, const std::string& itemName)
            : id(itemId), name(itemName) {}
        
        // JSON serialization
        json ToJson() const {
            json j;
            j["id"] = id;
            j["name"] = name;
            j["description"] = description;
            j["iconPath"] = iconPath;
            j["type"] = static_cast<int>(type);
            j["rarity"] = static_cast<int>(rarity);
            j["equipSlot"] = static_cast<int>(equipSlot);
            j["maxStack"] = maxStack;
            j["weight"] = weight;
            j["value"] = value;
            j["isQuestItem"] = isQuestItem;
            j["isConsumable"] = isConsumable;
            j["attackBonus"] = attackBonus;
            j["defenseBonus"] = defenseBonus;
            j["healthBonus"] = healthBonus;
            j["manaBonus"] = manaBonus;
            return j;
        }
        
        static Item FromJson(const json& j) {
            Item item;
            item.id = j.value("id", "");
            item.name = j.value("name", "");
            item.description = j.value("description", "");
            item.iconPath = j.value("iconPath", "");
            
            item.type = static_cast<ItemType>(j.value("type", 0));
            item.rarity = static_cast<ItemRarity>(j.value("rarity", 0));
            item.equipSlot = static_cast<EquipmentSlot>(j.value("equipSlot", 0));
            
            item.maxStack = j.value("maxStack", 1);
            item.weight = j.value("weight", 0.0f);
            item.value = j.value("value", 0);
            
            item.isQuestItem = j.value("isQuestItem", false);
            item.isConsumable = j.value("isConsumable", false);
            
            item.attackBonus = j.value("attackBonus", 0);
            item.defenseBonus = j.value("defenseBonus", 0);
            item.healthBonus = j.value("healthBonus", 0);
            item.manaBonus = j.value("manaBonus", 0);
            
            return item;
        }
        
        // Helper: get rarity color
        Color GetRarityColor() const {
            switch (rarity) {
                case ItemRarity::Common:    return Color(0.7f, 0.7f, 0.7f, 1.0f);  // Gray
                case ItemRarity::Uncommon:  return Color(0.3f, 1.0f, 0.3f, 1.0f);  // Green
                case ItemRarity::Rare:      return Color(0.3f, 0.5f, 1.0f, 1.0f);  // Blue
                case ItemRarity::Epic:      return Color(0.8f, 0.3f, 1.0f, 1.0f);  // Purple
                case ItemRarity::Legendary: return Color(1.0f, 0.6f, 0.1f, 1.0f);  // Orange
                default:                    return Color::White();
            }
        }
    };

    /**
     * @brief ItemStack - item instance in inventory
     */
    struct ItemStack {
        std::string itemID;
        int quantity = 0;
        
        ItemStack() = default;
        ItemStack(const std::string& id, int qty)
            : itemID(id), quantity(qty) {}
        
        bool IsEmpty() const { return quantity <= 0; }
        
        json ToJson() const {
            json j;
            j["itemID"] = itemID;
            j["quantity"] = quantity;
            return j;
        }
        
        static ItemStack FromJson(const json& j) {
            ItemStack stack;
            stack.itemID = j.value("itemID", "");
            stack.quantity = j.value("quantity", 0);
            return stack;
        }
    };

} // namespace SAGE
