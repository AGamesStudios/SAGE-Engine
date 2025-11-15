#pragma once

#include "Inventory/Item.h"
#include "Core/ResourceManager.h"
#include "Core/Logger.h"
#include <unordered_map>
#include <fstream>

namespace SAGE {

    /**
     * @brief ItemDatabase - global registry of all item definitions
     * 
     * Singleton pattern - load item definitions once, reference by ID.
     * 
     * Usage:
     *   ItemDatabase::Get().LoadFromFile("assets/items/items.json");
     *   const Item* potion = ItemDatabase::Get().GetItem("health_potion");
     *   if (potion) {
     *       inventory.AddItem(potion->id, 5);
     *   }
     */
    class ItemDatabase {
    public:
        static ItemDatabase& Get() {
            static ItemDatabase instance;
            return instance;
        }
        
        ItemDatabase(const ItemDatabase&) = delete;
        ItemDatabase& operator=(const ItemDatabase&) = delete;
        
        // Load items from JSON file
        bool LoadFromFile(const std::string& filepath) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to open item database file: {0}", filepath);
                return false;
            }
            
            json data = json::parse(file);
            return LoadFromJson(data);
        }
        
        bool LoadFromJson(const json& data) {
            if (!data.contains("items") || !data["items"].is_array()) {
                SAGE_ERROR("Item database JSON missing 'items' array");
                return false;
            }
            
            int loadedCount = 0;
            for (const auto& itemJson : data["items"]) {
                Item item = Item::FromJson(itemJson);
                
                // Load icon texture if path specified
                if (!item.iconPath.empty()) {
                    item.icon = ResourceManager::Get().Load<Texture>(item.iconPath);
                }
                
                // Register callbacks if specified (this would be done via scripting in real game)
                RegisterDefaultCallbacks(item);
                
                m_Items[item.id] = item;
                loadedCount++;
            }
            
            SAGE_INFO("Loaded {0} items into ItemDatabase", loadedCount);
            return true;
        }
        
        // Save database to JSON
        bool SaveToFile(const std::string& filepath) const {
            json data;
            json itemsArray = json::array();
            
            for (const auto& [id, item] : m_Items) {
                itemsArray.push_back(item.ToJson());
            }
            
            data["items"] = itemsArray;
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to save item database: {0}", filepath);
                return false;
            }
            
            file << data.dump(4);
            return true;
        }
        
        // Item queries
        const Item* GetItem(const std::string& itemID) const {
            auto it = m_Items.find(itemID);
            return (it != m_Items.end()) ? &it->second : nullptr;
        }
        
        Item* GetItemMutable(const std::string& itemID) {
            auto it = m_Items.find(itemID);
            return (it != m_Items.end()) ? &it->second : nullptr;
        }
        
        bool HasItem(const std::string& itemID) const {
            return m_Items.find(itemID) != m_Items.end();
        }
        
        // Manual item registration
        void RegisterItem(const Item& item) {
            m_Items[item.id] = item;
        }
        
        void UnregisterItem(const std::string& itemID) {
            m_Items.erase(itemID);
        }
        
        void Clear() {
            m_Items.clear();
        }
        
        // Batch queries
        std::vector<const Item*> GetItemsByType(ItemType type) const {
            std::vector<const Item*> result;
            for (const auto& [id, item] : m_Items) {
                if (item.type == type) {
                    result.push_back(&item);
                }
            }
            return result;
        }
        
        std::vector<const Item*> GetItemsByRarity(ItemRarity rarity) const {
            std::vector<const Item*> result;
            for (const auto& [id, item] : m_Items) {
                if (item.rarity == rarity) {
                    result.push_back(&item);
                }
            }
            return result;
        }
        
        const std::unordered_map<std::string, Item>& GetAllItems() const {
            return m_Items;
        }
        
        size_t GetItemCount() const {
            return m_Items.size();
        }
        
    private:
        ItemDatabase() = default;
        
        // Register default callbacks for common items
        void RegisterDefaultCallbacks(Item& item) {
            // Example: Health potion
            if (item.id == "health_potion" && item.isConsumable) {
                item.onUse = [](Entity* user) {
                    (void)user; // Unused parameter
                    // Heal logic would go here
                    SAGE_INFO("Used health potion!");
                };
            }
            
            // Example: Weapon equip
            if (item.type == ItemType::Equipment && item.equipSlot == EquipmentSlot::Weapon) {
                item.onEquip = [item](Entity* user) {
                    (void)user; // Unused parameter
                    SAGE_INFO("Equipped weapon: {0} (+{1} ATK)", item.name, item.attackBonus);
                };
                
                item.onUnequip = [item](Entity* user) {
                    (void)user; // Unused parameter
                    SAGE_INFO("Unequipped weapon: {0}", item.name);
                };
            }
        }
        
        std::unordered_map<std::string, Item> m_Items;
    };

} // namespace SAGE
