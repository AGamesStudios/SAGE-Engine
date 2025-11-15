#pragma once

#include "Core/SaveManager.h"
#include "Inventory/Inventory.h"
#include "Inventory/EquipmentManager.h"
#include "Inventory/CraftingSystem.h"
#include "Quests/QuestManager.h"
#include "Dialogue/DialogueRunner.h"
#include "Core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Extended Save Manager with RPG system integration
 * 
 * Features:
 * - Save/Load inventory state
 * - Save/Load equipment state
 * - Save/Load discovered recipes
 * - Save/Load quest progress
 * - Save/Load dialogue state
 * - Integrated with SaveManager for versioning and CRC
 */
class RPGSaveManager {
public:
    struct PlayerData {
        // Player stats
        std::string playerName;
        int level = 1;
        int experience = 0;
        int gold = 0;
        float playtimeSeconds = 0.0f; // Total playtime tracking
        
        // Systems
        Inventory* inventory = nullptr;
        EquipmentManager* equipment = nullptr;
        CraftingSystem* crafting = nullptr;
        Quests::QuestManager* questManager = nullptr;
        DialogueRunner* dialogueRunner = nullptr;
        
        // Current location
        std::string currentScene;
        float positionX = 0.0f;
        float positionY = 0.0f;
    };

    static RPGSaveManager& Instance() {
        static RPGSaveManager instance;
        return instance;
    }

    /**
     * @brief Save full game state to slot
     */
    bool SaveGame(int slotIndex, const PlayerData& playerData, Scene* scene = nullptr) {
        try {
            // Create full save data
            json saveData;
            saveData["version"] = CURRENT_SAVE_VERSION;
            saveData["timestamp"] = std::time(nullptr);

            // Player data
            json playerJson;
            playerJson["name"] = playerData.playerName;
            playerJson["level"] = playerData.level;
            playerJson["experience"] = playerData.experience;
            playerJson["gold"] = playerData.gold;
            playerJson["playtime"] = playerData.playtimeSeconds;
            playerJson["currentScene"] = playerData.currentScene;
            playerJson["positionX"] = playerData.positionX;
            playerJson["positionY"] = playerData.positionY;
            saveData["player"] = playerJson;

            // Inventory
            if (playerData.inventory) {
                saveData["inventory"] = SerializeInventory(*playerData.inventory);
            }

            // Equipment
            if (playerData.equipment) {
                saveData["equipment"] = playerData.equipment->ToJson();
            }

            // Crafting (discovered recipes)
            if (playerData.crafting) {
                saveData["crafting"] = playerData.crafting->ToJson();
            }

            // Quests
            if (playerData.questManager) {
                saveData["quests"] = SerializeQuests(*playerData.questManager);
            }

            // Dialogue state
            if (playerData.dialogueRunner) {
                saveData["dialogue"] = SerializeDialogue(*playerData.dialogueRunner);
            }

            // Scene data (optional)
            if (scene) {
                json sceneJson;
                // Basic scene serialization - placeholder
                sceneJson["saved"] = true;
                saveData["scene"] = sceneJson;
                SAGE_INFO("RPGSaveManager: Saved scene data (basic)");
            }

            // Write to file
            std::string filepath = m_SaveManager.ResolveSlotFilePath(slotIndex);
            if (filepath.empty()) {
                SAGE_ERROR("RPGSaveManager: Invalid slot index {}", slotIndex);
                return false;
            }
            std::ofstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("RPGSaveManager: Failed to create save file");
                return false;
            }

            file << saveData.dump(4);
            file.close();

            // Update metadata
            SaveSlot metadata;
            metadata.playerName = playerData.playerName;
            metadata.playerLevel = playerData.level;
            metadata.currentLevel = playerData.currentScene;
            metadata.playtimeSeconds = playerData.playtimeSeconds; // Use actual playtime from PlayerData

            // Save with CRC validation
            return m_SaveManager.SaveToSlot(slotIndex, scene, metadata);
        }
        catch (const std::exception& e) {
            SAGE_ERROR("RPGSaveManager: Save failed: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Load full game state from slot
     */
    bool LoadGame(int slotIndex, PlayerData& playerData, Scene* scene = nullptr) {
        try {
            // Load with integrity check
            if (!m_SaveManager.LoadFromSlot(slotIndex, scene)) {
                return false;
            }

            // Read save data
            std::string filepath = m_SaveManager.ResolveSlotFilePath(slotIndex);
            if (filepath.empty()) {
                SAGE_ERROR("RPGSaveManager: Invalid slot index {}", slotIndex);
                return false;
            }
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("RPGSaveManager: Failed to open save file");
                return false;
            }

            json saveData;
            file >> saveData;

            // Player data
            if (saveData.contains("player")) {
                const auto& playerJson = saveData["player"];
                playerData.playerName = playerJson.value("name", "Player");
                playerData.level = playerJson.value("level", 1);
                playerData.experience = playerJson.value("experience", 0);
                playerData.gold = playerJson.value("gold", 0);
                playerData.playtimeSeconds = playerJson.value("playtime", 0.0f);
                playerData.currentScene = playerJson.value("currentScene", "");
                playerData.positionX = playerJson.value("positionX", 0.0f);
                playerData.positionY = playerJson.value("positionY", 0.0f);
            }

            // Inventory
            if (saveData.contains("inventory") && playerData.inventory) {
                DeserializeInventory(saveData["inventory"], *playerData.inventory);
            }

            // Equipment
            if (saveData.contains("equipment") && playerData.equipment) {
                playerData.equipment->FromJson(saveData["equipment"]);
            }

            // Crafting
            if (saveData.contains("crafting") && playerData.crafting) {
                playerData.crafting->FromJson(saveData["crafting"]);
            }

            // Quests
            if (saveData.contains("quests") && playerData.questManager) {
                DeserializeQuests(saveData["quests"], *playerData.questManager);
            }

            // Dialogue
            if (saveData.contains("dialogue") && playerData.dialogueRunner) {
                DeserializeDialogue(saveData["dialogue"], *playerData.dialogueRunner);
            }

            SAGE_INFO("RPGSaveManager: Loaded game from slot {}", slotIndex);
            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("RPGSaveManager: Load failed: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Quick save to autosave slot
     */
    bool QuickSave(const PlayerData& playerData, Scene* scene = nullptr) {
        const int AUTOSAVE_SLOT = 99; // Reserved slot for autosaves
        SAGE_INFO("RPGSaveManager: Quick saving to slot {}...", AUTOSAVE_SLOT);
        return SaveGame(AUTOSAVE_SLOT, playerData, scene);
    }

    /**
     * @brief Quick load from autosave slot
     */
    bool QuickLoad(PlayerData& playerData, Scene* scene = nullptr) {
        const int AUTOSAVE_SLOT = 99;
        SAGE_INFO("RPGSaveManager: Quick loading from slot {}...", AUTOSAVE_SLOT);
        return LoadGame(AUTOSAVE_SLOT, playerData, scene);
    }

    SaveManager& GetSaveManager() { return m_SaveManager; }

private:
    RPGSaveManager() = default;

    json SerializeInventory(const Inventory& inventory) {
        json j;
        j["capacity"] = inventory.GetCapacity();
        j["maxWeight"] = inventory.GetMaxWeight();
        j["currentWeight"] = inventory.GetCurrentWeight();
        
        // Serialize inventory slots
        json slotsArray = json::array();
        const auto& slots = inventory.GetSlots();
        for (const auto& slot : slots) {
            slotsArray.push_back(slot.ToJson());
        }
        j["slots"] = slotsArray;
        
        return j;
    }

    void DeserializeInventory(const json& j, Inventory& inventory) {
        if (!j.contains("slots")) {
            SAGE_WARNING("RPGSaveManager: No slots data in inventory save");
            return;
        }
        
        // Note: Inventory doesn't have Clear() method, manually clear slots
        auto& slots = inventory.GetSlots();
        for (auto& slot : slots) {
            slot = InventorySlot(); // Reset to empty slot
        }
        
        // Restore weight limit if saved
        if (j.contains("maxWeight")) {
            inventory.SetMaxWeight(j["maxWeight"].get<float>());
        }
        
        // Restore slots
        const auto& slotsArray = j["slots"];
        size_t slotIndex = 0;
        
        for (const auto& slotJson : slotsArray) {
            if (slotIndex >= slots.size()) break;
            slots[slotIndex] = InventorySlot::FromJson(slotJson);
            slotIndex++;
        }
        
        SAGE_INFO("RPGSaveManager: Restored {} inventory slots", slotIndex);
    }

    json SerializeQuests(const Quests::QuestManager& questManager) {
        json j;

        // Serialize active quests (simplified - without objectives details)
        json activeArray = json::array();
        for (const auto& questID : questManager.GetActiveQuestIDs()) {
            activeArray.push_back(questID);
        }
        j["activeQuests"] = activeArray;

        // Serialize completed quests
        json completedArray = json::array();
        for (const auto& questID : questManager.GetCompletedQuestIDs()) {
            completedArray.push_back(questID);
        }
        j["completedQuests"] = completedArray;
        
        SAGE_INFO("RPGSaveManager: Serialized {} active, {} completed quests", 
                  activeArray.size(), completedArray.size());
        
        return j;
    }

    void DeserializeQuests(const json& j, Quests::QuestManager& questManager) {
        // Restore active quests
        if (j.contains("activeQuests")) {
            for (const auto& questID : j["activeQuests"]) {
                questManager.StartQuest(questID.get<std::string>());
            }
        }
        
        // Restore completed quests
        if (j.contains("completedQuests")) {
            for (const auto& questID : j["completedQuests"]) {
                // Mark quest as completed
                questManager.CompleteQuest(questID.get<std::string>());
            }
        }
        
        SAGE_INFO("RPGSaveManager: Restored quest progress");
    }

    json SerializeDialogue(const DialogueRunner& dialogueRunner) {
        (void)dialogueRunner;  // Suppress unused parameter warning
        json j;
        
        // Save dialogue state (basic implementation)
        // Note: DialogueRunner may not have direct access to variables/history
        // This is a placeholder for future implementation
        j["savedState"] = true;
        
        SAGE_INFO("RPGSaveManager: Serialized dialogue state (basic)");
        
        return j;
    }

    void DeserializeDialogue(const json& j, DialogueRunner& dialogueRunner) {
        (void)j;                // Suppress unused parameter warning
        (void)dialogueRunner;   // Suppress unused parameter warning
        // Restore dialogue state (basic implementation)
        // Note: Full implementation depends on DialogueRunner API
        (void)dialogueRunner; // Suppress warning
        
        if (j.contains("savedState") && j["savedState"].get<bool>()) {
            SAGE_INFO("RPGSaveManager: Restored dialogue state (basic)");
        }
    }

    SaveManager m_SaveManager;
};

} // namespace SAGE
