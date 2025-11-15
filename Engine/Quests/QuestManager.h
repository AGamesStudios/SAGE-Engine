#pragma once

#include "Quest.h"
#include "QuestEvents.h"
#include "Core/EventBus.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace SAGE {
namespace Quests {

/// @brief Менеджер квестов (singleton)
class QuestManager {
public:
    static QuestManager& Get() {
        static QuestManager instance;
        return instance;
    }
    
    QuestManager(const QuestManager&) = delete;
    QuestManager& operator=(const QuestManager&) = delete;
    
    // ========== Initialization ==========
    
    /// @brief Инициализация (привязка к EventBus)
    void Init(EventBus* eventBus) {
        m_EventBus = eventBus;
        SAGE_INFO("[QuestManager] Initialized");
    }
    
    void Shutdown() {
        m_Quests.clear();
        m_ActiveQuests.clear();
        m_CompletedQuests.clear();
        SAGE_INFO("[QuestManager] Shutdown");
    }
    
    // ========== Quest Registration ==========
    
    /// @brief Зарегистрировать квест из кода
    void RegisterQuest(const Quest& quest) {
        const std::string& id = quest.GetID();
        if (m_Quests.find(id) != m_Quests.end()) {
            SAGE_WARNING("[QuestManager] Quest '{}' already registered", id);
            return;
        }
        
        m_Quests[id] = std::make_unique<Quest>(quest);
        SAGE_INFO("[QuestManager] Registered quest: {}", quest.GetTitle());
    }
    
    /// @brief Загрузить квесты из JSON файла
    bool LoadQuestsFromFile(const std::string& filepath);
    
    // ========== Quest Queries ==========
    
    Quest* GetQuest(const std::string& questID) {
        auto it = m_Quests.find(questID);
        return (it != m_Quests.end()) ? it->second.get() : nullptr;
    }
    
    const std::vector<std::string>& GetActiveQuestIDs() const {
        return m_ActiveQuests;
    }
    
    const std::vector<std::string>& GetCompletedQuestIDs() const {
        return m_CompletedQuests;
    }
    
    /// @brief Получить все активные квесты
    std::vector<Quest*> GetActiveQuests() {
        std::vector<Quest*> quests;
        for (const auto& id : m_ActiveQuests) {
            if (Quest* quest = GetQuest(id)) {
                quests.push_back(quest);
            }
        }
        return quests;
    }
    
    bool IsQuestActive(const std::string& questID) const {
        return std::find(m_ActiveQuests.begin(), m_ActiveQuests.end(), questID) != m_ActiveQuests.end();
    }
    
    bool IsQuestCompleted(const std::string& questID) const {
        return std::find(m_CompletedQuests.begin(), m_CompletedQuests.end(), questID) != m_CompletedQuests.end();
    }
    
    // ========== Quest Lifecycle ==========
    
    /// @brief Начать квест
    bool StartQuest(const std::string& questID) {
        Quest* quest = GetQuest(questID);
        if (!quest) {
            SAGE_ERROR("[QuestManager] Quest '{}' not found", questID);
            return false;
        }
        
        if (quest->IsActive()) {
            SAGE_WARNING("[QuestManager] Quest '{}' already active", questID);
            return false;
        }
        
        quest->Start();
        m_ActiveQuests.push_back(questID);
        
        // Publish event
        if (m_EventBus) {
            QuestStartedEvent event(questID, quest->GetTitle());
            m_EventBus->Publish(event);
        }
        
        return true;
    }
    
    /// @brief Завершить квест (проверка целей)
    bool CompleteQuest(const std::string& questID) {
        Quest* quest = GetQuest(questID);
        if (!quest) return false;
        
        if (!quest->TryComplete()) {
            return false;
        }
        
        // Переместить из активных в завершенные
        auto it = std::find(m_ActiveQuests.begin(), m_ActiveQuests.end(), questID);
        if (it != m_ActiveQuests.end()) {
            m_ActiveQuests.erase(it);
        }
        
        m_CompletedQuests.push_back(questID);
        
        // Publish event
        if (m_EventBus) {
            QuestCompletedEvent event(questID, quest->GetTitle(), quest->GetReward());
            m_EventBus->Publish(event);
        }
        
        return true;
    }
    
    /// @brief Провалить квест
    void FailQuest(const std::string& questID) {
        Quest* quest = GetQuest(questID);
        if (!quest) return;
        
        quest->Fail();
        
        // Удалить из активных
        auto it = std::find(m_ActiveQuests.begin(), m_ActiveQuests.end(), questID);
        if (it != m_ActiveQuests.end()) {
            m_ActiveQuests.erase(it);
        }
        
        // Publish event
        if (m_EventBus) {
            QuestFailedEvent event(questID, quest->GetTitle());
            m_EventBus->Publish(event);
        }
    }
    
    /// @brief Сдать квест (получить награду)
    bool TurnInQuest(const std::string& questID) {
        Quest* quest = GetQuest(questID);
        if (!quest || !quest->IsCompleted()) {
            return false;
        }
        
        quest->TurnIn();
        
        // Publish event
        if (m_EventBus) {
            QuestTurnedInEvent event(questID, quest->GetTitle(), quest->GetReward());
            m_EventBus->Publish(event);
        }
        
        return true;
    }
    
    // ========== Objective Tracking ==========
    
    /// @brief Обновить прогресс цели (Kill/Collect)
    bool UpdateObjective(const std::string& questID, ObjectiveType type, const std::string& targetID, int delta = 1) {
        Quest* quest = GetQuest(questID);
        if (!quest || !quest->IsActive()) {
            return false;
        }
        
        QuestObjective* objective = quest->FindObjective(type, targetID);
        if (!objective) {
            return false;
        }
        
        // Обновить прогресс
        int newCount = objective->GetCurrentCount() + delta;
        bool objectiveCompleted = objective->UpdateProgress(newCount);
        
        // Publish objective update event
        if (m_EventBus) {
            size_t objIndex = 0;
            for (size_t i = 0; i < quest->GetObjectives().size(); ++i) {
                if (&quest->GetObjectives()[i] == objective) {
                    objIndex = i;
                    break;
                }
            }
            
            ObjectiveUpdatedEvent event(
                questID,
                objIndex,
                objective->GetCurrentCount(),
                objective->GetRequiredCount(),
                objectiveCompleted);
            m_EventBus->Publish(event);
        }
        
        // Проверить завершение квеста
        if (quest->AreAllObjectivesCompleted()) {
            CompleteQuest(questID);
        }
        
        return true;
    }
    
    /// @brief Обновить прогресс цели "Kill Enemy"
    void OnEnemyKilled(const std::string& enemyType) {
        for (const auto& questID : m_ActiveQuests) {
            UpdateObjective(questID, ObjectiveType::Kill, enemyType, 1);
        }
    }
    
    /// @brief Обновить прогресс цели "Collect Item"
    void OnItemCollected(const std::string& itemID) {
        for (const auto& questID : m_ActiveQuests) {
            UpdateObjective(questID, ObjectiveType::Collect, itemID, 1);
        }
    }
    
    /// @brief Завершить цель "Talk To NPC"
    void OnNPCTalkedTo(const std::string& npcID) {
        for (const auto& questID : m_ActiveQuests) {
            Quest* quest = GetQuest(questID);
            if (!quest) continue;
            
            QuestObjective* objective = quest->FindObjective(ObjectiveType::TalkTo, npcID);
            if (objective && !objective->IsCompleted()) {
                objective->Complete();
                
                // Publish event
                if (m_EventBus) {
                    ObjectiveUpdatedEvent event(questID, 0, 1, 1, true);
                    m_EventBus->Publish(event);
                }
                
                // Проверить завершение квеста
                if (quest->AreAllObjectivesCompleted()) {
                    CompleteQuest(questID);
                }
            }
        }
    }
    
    /// @brief Завершить цель "Reach Location"
    void OnLocationReached(const std::string& locationID) {
        for (const auto& questID : m_ActiveQuests) {
            Quest* quest = GetQuest(questID);
            if (!quest) continue;
            
            QuestObjective* objective = quest->FindObjective(ObjectiveType::Reach, locationID);
            if (objective && !objective->IsCompleted()) {
                objective->Complete();
                
                if (quest->AreAllObjectivesCompleted()) {
                    CompleteQuest(questID);
                }
            }
        }
    }
    
    // ========== Save/Load ==========
    
    /// @brief Сохранить состояние квестов в JSON
    bool SaveToFile(const std::string& filepath);
    
    /// @brief Загрузить состояние квестов из JSON
    bool LoadFromFile(const std::string& filepath);
    
private:
    QuestManager() : m_EventBus(nullptr) {}
    
    EventBus* m_EventBus;
    
    std::unordered_map<std::string, std::unique_ptr<Quest>> m_Quests;  // Все зарегистрированные квесты
    std::vector<std::string> m_ActiveQuests;                            // ID активных квестов
    std::vector<std::string> m_CompletedQuests;                         // ID завершенных квестов
};

} // namespace Quests
} // namespace SAGE
