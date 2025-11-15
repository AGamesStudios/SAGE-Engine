#include "QuestManager.h"
#include <fstream>
#include <json/json.hpp>

using json = nlohmann::json;

namespace SAGE {
namespace Quests {

// ========== JSON Parsing ==========

static ObjectiveType ParseObjectiveType(const std::string& typeStr) {
    if (typeStr == "kill") return ObjectiveType::Kill;
    if (typeStr == "collect") return ObjectiveType::Collect;
    if (typeStr == "talk") return ObjectiveType::TalkTo;
    if (typeStr == "reach") return ObjectiveType::Reach;
    if (typeStr == "trigger") return ObjectiveType::Trigger;
    return ObjectiveType::Custom;
}

static QuestObjective ParseObjective(const json& objJson) {
    std::string description = objJson.value("description", "");
    std::string typeStr = objJson.value("type", "custom");
    int requiredCount = objJson.value("count", 1);
    
    ObjectiveType type = ParseObjectiveType(typeStr);
    
    QuestObjective objective(description, type, requiredCount);
    objective.SetTargetID(objJson.value("target", ""));
    objective.SetOptional(objJson.value("optional", false));
    objective.SetHidden(objJson.value("hidden", false));
    
    return objective;
}

static QuestReward ParseReward(const json& rewardJson) {
    QuestReward reward;
    reward.experience = rewardJson.value("xp", 0);
    reward.gold = rewardJson.value("gold", 0);
    
    if (rewardJson.contains("items")) {
        for (const auto& item : rewardJson["items"]) {
            reward.items.push_back(item.get<std::string>());
        }
    }
    
    if (rewardJson.contains("unlock_quests")) {
        for (const auto& quest : rewardJson["unlock_quests"]) {
            reward.unlockedQuests.push_back(quest.get<std::string>());
        }
    }
    
    if (rewardJson.contains("unlock_dialogues")) {
        for (const auto& dialogue : rewardJson["unlock_dialogues"]) {
            reward.unlockedDialogues.push_back(dialogue.get<std::string>());
        }
    }
    
    return reward;
}

static Quest ParseQuest(const json& questJson) {
    std::string id = questJson.value("id", "");
    std::string title = questJson.value("title", "");
    
    Quest quest(id, title);
    quest.SetDescription(questJson.value("description", ""));
    quest.SetGiverNPC(questJson.value("giver_npc", ""));
    quest.SetCompletionNPC(questJson.value("completion_npc", ""));
    quest.SetLevel(questJson.value("level", 1));
    
    // Objectives
    if (questJson.contains("objectives")) {
        for (const auto& objJson : questJson["objectives"]) {
            quest.AddObjective(ParseObjective(objJson));
        }
    }
    
    // Reward
    if (questJson.contains("reward")) {
        quest.SetReward(ParseReward(questJson["reward"]));
    }
    
    return quest;
}

// ========== QuestManager Implementation ==========

bool QuestManager::LoadQuestsFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SAGE_ERROR("[QuestManager] Failed to open quest file: {}", filepath);
        return false;
    }

    json data = json::parse(file);

    if (!data.contains("quests") || !data["quests"].is_array()) {
        SAGE_ERROR("[QuestManager] Invalid quest file format (missing 'quests' array)");
        return false;
    }

    int loadedCount = 0;
    for (const auto& questJson : data["quests"]) {
        Quest quest = ParseQuest(questJson);
        RegisterQuest(quest);
        loadedCount++;
    }

    SAGE_INFO("[QuestManager] Loaded {} quests from {}", loadedCount, filepath);
    return true;
}

// ========== Save/Load State ==========

bool QuestManager::SaveToFile(const std::string& filepath) {
    json data;
    
    // Save active quests state
    json activeQuestsJson = json::array();
    for (const auto& questID : m_ActiveQuests) {
        Quest* quest = GetQuest(questID);
        if (!quest) continue;
        
        json questData;
        questData["id"] = quest->GetID();
        questData["state"] = static_cast<int>(quest->GetState());
        
        // Save objective progress
        json objectivesJson = json::array();
        for (const auto& obj : quest->GetObjectives()) {
            json objData;
            objData["current_count"] = obj.GetCurrentCount();
            objData["state"] = static_cast<int>(obj.GetState());
            objectivesJson.push_back(objData);
        }
        questData["objectives"] = objectivesJson;
        
        activeQuestsJson.push_back(questData);
    }
    data["active_quests"] = activeQuestsJson;
    
    // Save completed quests
    json completedArray = json::array();
    for (const auto& questID : m_CompletedQuests) {
        completedArray.push_back(questID);
    }
    data["completed_quests"] = completedArray;
    
    // Write to file
    std::ofstream file(filepath);
    if (!file.is_open()) {
        SAGE_ERROR("[QuestManager] Failed to open save file: {}", filepath);
        return false;
    }
    
    file << data.dump(2);
    SAGE_INFO("[QuestManager] Saved quest state to {}", filepath);
    return true;
}

bool QuestManager::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SAGE_WARNING("[QuestManager] Save file not found: {}", filepath);
        return false;
    }
    
    json data = json::parse(file);

    // Load active quests
    if (data.contains("active_quests") && data["active_quests"].is_array()) {
        for (const auto& questData : data["active_quests"]) {
            std::string questID = questData.value("id", "");
            Quest* quest = GetQuest(questID);
            if (!quest) {
                continue;
            }

            // Restore objectives
            if (questData.contains("objectives") && questData["objectives"].is_array()) {
                const auto& objectivesJson = questData["objectives"];
                for (size_t i = 0; i < objectivesJson.size() && i < quest->GetObjectives().size(); ++i) {
                    QuestObjective* obj = quest->GetObjective(i);
                    if (obj) {
                        int currentCount = objectivesJson[i].value("current_count", 0);
                        obj->UpdateProgress(currentCount);
                    }
                }
            }

            // Force start (bypass normal start logic)
            if (!IsQuestActive(questID)) {
                m_ActiveQuests.push_back(questID);
            }
        }
    }

    // Load completed quests
    if (data.contains("completed_quests") && data["completed_quests"].is_array()) {
        m_CompletedQuests.clear();
        for (const auto& value : data["completed_quests"]) {
            m_CompletedQuests.push_back(value.get<std::string>());
        }
    }

    SAGE_INFO("[QuestManager] Loaded quest state from {}", filepath);
    return true;
}

} // namespace Quests
} // namespace SAGE
