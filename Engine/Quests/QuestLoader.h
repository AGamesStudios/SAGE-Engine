#pragma once

#include "Quest.h"
#include "QuestObjective.h"
#include "Core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace SAGE {
namespace Quests {

using json = nlohmann::json;

/**
 * @brief Quest Loader - loads quests from JSON files
 * 
 * JSON Format:
 * {
 *   "quests": [
 *     {
 *       "id": "quest_main_001",
 *       "title": "The Lost Artifact",
 *       "description": "Find the ancient artifact in the dungeon",
 *       "rewardGold": 100,
 *       "rewardExperience": 50,
 *       "rewardItems": ["health_potion", "iron_sword"],
 *       "prerequisites": ["quest_intro_001"],
 *       "objectives": [
 *         {
 *           "id": "obj_1",
 *           "type": "collect",
 *           "description": "Collect 5 ancient coins",
 *           "targetItem": "ancient_coin",
 *           "requiredCount": 5
 *         },
 *         {
 *           "id": "obj_2",
 *           "type": "kill",
 *           "description": "Defeat the dungeon boss",
 *           "targetEntity": "boss_skeleton_king",
 *           "requiredCount": 1
 *         }
 *       ]
 *     }
 *   ]
 * }
 */
class QuestLoader {
public:
    /**
     * @brief Load quests from JSON file
     */
    static std::vector<Quest> LoadFromFile(const std::string& filepath) {
        std::vector<Quest> quests;

        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("QuestLoader: Failed to open file: {}", filepath);
                return quests;
            }

            json data;
            file >> data;

            if (!data.contains("quests") || !data["quests"].is_array()) {
                SAGE_ERROR("QuestLoader: Invalid format - missing 'quests' array");
                return quests;
            }

            for (const auto& questJson : data["quests"]) {
                Quest quest = ParseQuest(questJson);
                quests.push_back(quest);
            }

            SAGE_INFO("QuestLoader: Loaded {} quests from {}", quests.size(), filepath);
        }
        catch (const std::exception& e) {
            SAGE_ERROR("QuestLoader: Failed to load quests: {}", e.what());
        }

        return quests;
    }

    /**
     * @brief Parse single quest from JSON
     */
    static Quest ParseQuest(const json& j) {
        std::string id = j.value("id", "");
        std::string title = j.value("title", "Untitled Quest");
        std::string description = j.value("description", "");

        Quest quest(id, title, description);

        // Rewards
        if (j.contains("rewardGold")) {
            quest.SetRewardGold(j["rewardGold"].get<int>());
        }

        if (j.contains("rewardExperience")) {
            quest.SetRewardExperience(j["rewardExperience"].get<int>());
        }

        if (j.contains("rewardItems") && j["rewardItems"].is_array()) {
            for (const auto& itemID : j["rewardItems"]) {
                quest.AddRewardItem(itemID.get<std::string>());
            }
        }

        // Prerequisites
        if (j.contains("prerequisites") && j["prerequisites"].is_array()) {
            for (const auto& prereqID : j["prerequisites"]) {
                quest.AddPrerequisite(prereqID.get<std::string>());
            }
        }

        // Category
        if (j.contains("category")) {
            quest.SetCategory(j["category"].get<std::string>());
        }

        // Level requirement
        if (j.contains("minLevel")) {
            quest.SetMinLevel(j["minLevel"].get<int>());
        }

        // Objectives
        if (j.contains("objectives") && j["objectives"].is_array()) {
            for (const auto& objJson : j["objectives"]) {
                QuestObjective objective = ParseObjective(objJson);
                quest.AddObjective(objective);
            }
        }

        return quest;
    }

    /**
     * @brief Parse quest objective from JSON
     */
    static QuestObjective ParseObjective(const json& j) {
        std::string id = j.value("id", "");
        std::string description = j.value("description", "");
        
        // Parse type
        ObjectiveType type = ObjectiveType::Custom;
        std::string typeStr = j.value("type", "custom");
        
        if (typeStr == "collect") type = ObjectiveType::CollectItems;
        else if (typeStr == "kill") type = ObjectiveType::KillEnemies;
        else if (typeStr == "reach") type = ObjectiveType::ReachLocation;
        else if (typeStr == "interact") type = ObjectiveType::InteractWithObject;
        else if (typeStr == "talk") type = ObjectiveType::TalkToNPC;
        else if (typeStr == "escort") type = ObjectiveType::EscortNPC;

        int requiredCount = j.value("requiredCount", 1);

        QuestObjective objective(id, type, description, requiredCount);

        // Type-specific data
        if (j.contains("targetItem")) {
            objective.SetTargetItem(j["targetItem"].get<std::string>());
        }

        if (j.contains("targetEntity")) {
            objective.SetTargetEntity(j["targetEntity"].get<std::string>());
        }

        if (j.contains("targetLocation")) {
            // TODO: Parse location coordinates
        }

        if (j.contains("isOptional")) {
            objective.SetOptional(j["isOptional"].get<bool>());
        }

        return objective;
    }
};

} // namespace Quests
} // namespace SAGE
