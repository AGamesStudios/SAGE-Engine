#pragma once

#include "QuestObjective.h"
#include "Core/Logger.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace SAGE {
namespace Quests {

/// @brief Состояние квеста
enum class QuestState {
    NotStarted,     // Квест не начат
    InProgress,     // Активен (цели в процессе выполнения)
    Completed,      // Успешно завершен
    Failed,         // Провален
    TurnedIn        // Сдан (получена награда)
};

/// @brief Награда за квест
struct QuestReward {
    int experience = 0;                         // XP
    int gold = 0;                               // Деньги
    std::vector<std::string> items;             // ID предметов
    std::vector<std::string> unlockedQuests;    // ID разблокированных квестов
    std::vector<std::string> unlockedDialogues; // ID разблокированных диалогов
    
    bool IsEmpty() const {
        return experience == 0 && gold == 0 && items.empty() 
            && unlockedQuests.empty() && unlockedDialogues.empty();
    }
};

/// @brief Квест с целями и наградами
class Quest {
public:
    Quest() = default;
    
    Quest(const std::string& id, const std::string& title)
        : m_ID(id)
        , m_Title(title)
        , m_State(QuestState::NotStarted)
        , m_Level(1)
    {}
    
    // ========== Getters ==========
    
    const std::string& GetID() const { return m_ID; }
    const std::string& GetTitle() const { return m_Title; }
    const std::string& GetDescription() const { return m_Description; }
    const std::string& GetGiverNPC() const { return m_GiverNPC; }
    const std::string& GetCompletionNPC() const { return m_CompletionNPC; }
    
    QuestState GetState() const { return m_State; }
    int GetLevel() const { return m_Level; }
    
    const std::vector<QuestObjective>& GetObjectives() const { return m_Objectives; }
    const QuestReward& GetReward() const { return m_Reward; }
    
    bool IsActive() const { return m_State == QuestState::InProgress; }
    bool IsCompleted() const { return m_State == QuestState::Completed || m_State == QuestState::TurnedIn; }
    bool IsFailed() const { return m_State == QuestState::Failed; }
    bool IsTurnedIn() const { return m_State == QuestState::TurnedIn; }
    
    // ========== Setters ==========
    
    void SetID(const std::string& id) { m_ID = id; }
    void SetTitle(const std::string& title) { m_Title = title; }
    void SetDescription(const std::string& desc) { m_Description = desc; }
    void SetGiverNPC(const std::string& npc) { m_GiverNPC = npc; }
    void SetCompletionNPC(const std::string& npc) { m_CompletionNPC = npc; }
    void SetLevel(int level) { m_Level = level; }
    void SetReward(const QuestReward& reward) { m_Reward = reward; }
    
    // ========== Objectives Management ==========
    
    void AddObjective(const QuestObjective& objective) {
        m_Objectives.push_back(objective);
    }
    
    QuestObjective* GetObjective(size_t index) {
        if (index >= m_Objectives.size()) return nullptr;
        return &m_Objectives[index];
    }
    
    /// @brief Найти цель по типу и targetID
    QuestObjective* FindObjective(ObjectiveType type, const std::string& targetID) {
        for (auto& obj : m_Objectives) {
            if (obj.GetType() == type && obj.GetTargetID() == targetID) {
                return &obj;
            }
        }
        return nullptr;
    }
    
    /// @brief Проверка завершения всех обязательных целей
    bool AreAllObjectivesCompleted() const {
        for (const auto& obj : m_Objectives) {
            if (!obj.IsOptional() && !obj.IsCompleted()) {
                return false;
            }
        }
        return true;
    }
    
    /// @brief Получить прогресс квеста (0.0 - 1.0)
    float GetProgress() const {
        if (m_Objectives.empty()) return 1.0f;
        
        int totalObjectives = 0;
        int completedObjectives = 0;
        
        for (const auto& obj : m_Objectives) {
            if (!obj.IsOptional()) {
                totalObjectives++;
                if (obj.IsCompleted()) {
                    completedObjectives++;
                }
            }
        }
        
        if (totalObjectives == 0) return 1.0f;
        return static_cast<float>(completedObjectives) / static_cast<float>(totalObjectives);
    }
    
    // ========== Quest Lifecycle ==========
    
    /// @brief Начать квест
    void Start() {
        if (m_State != QuestState::NotStarted) {
            SAGE_WARNING("[Quest] Cannot start quest '{}' - already in state {}", m_Title, static_cast<int>(m_State));
            return;
        }
        
        m_State = QuestState::InProgress;
        SAGE_INFO("[Quest] Started: {}", m_Title);
        
        // Вызвать callback
        if (m_OnStarted) {
            m_OnStarted(*this);
        }
    }
    
    /// @brief Попытка завершить квест (проверка всех целей)
    bool TryComplete() {
        if (m_State != QuestState::InProgress) {
            return false;
        }
        
        if (!AreAllObjectivesCompleted()) {
            SAGE_WARNING("[Quest] Cannot complete '{}' - objectives not finished", m_Title);
            return false;
        }
        
        m_State = QuestState::Completed;
        SAGE_INFO("[Quest] Completed: {}", m_Title);
        
        if (m_OnCompleted) {
            m_OnCompleted(*this);
        }
        
        return true;
    }
    
    /// @brief Принудительное завершение (для скриптов)
    void ForceComplete() {
        m_State = QuestState::Completed;
        SAGE_INFO("[Quest] Force completed: {}", m_Title);
        
        if (m_OnCompleted) {
            m_OnCompleted(*this);
        }
    }
    
    /// @brief Провалить квест
    void Fail() {
        if (m_State != QuestState::InProgress) {
            return;
        }
        
        m_State = QuestState::Failed;
        SAGE_WARNING("[Quest] Failed: {}", m_Title);
        
        if (m_OnFailed) {
            m_OnFailed(*this);
        }
    }
    
    /// @brief Сдать квест (получить награду)
    void TurnIn() {
        if (m_State != QuestState::Completed) {
            SAGE_WARNING("[Quest] Cannot turn in '{}' - not completed", m_Title);
            return;
        }
        
        m_State = QuestState::TurnedIn;
        SAGE_INFO("[Quest] Turned in: {} (Reward: {} XP, {} gold, {} items)", 
                  m_Title, m_Reward.experience, m_Reward.gold, m_Reward.items.size());
        
        if (m_OnTurnedIn) {
            m_OnTurnedIn(*this);
        }
    }
    
    /// @brief Сбросить квест (для повторного прохождения)
    void Reset() {
        m_State = QuestState::NotStarted;
        for (auto& obj : m_Objectives) {
            obj.Reset();
        }
    }
    
    // ========== Callbacks ==========
    
    using QuestCallback = std::function<void(Quest&)>;
    
    void SetOnStarted(QuestCallback callback) { m_OnStarted = std::move(callback); }
    void SetOnCompleted(QuestCallback callback) { m_OnCompleted = std::move(callback); }
    void SetOnFailed(QuestCallback callback) { m_OnFailed = std::move(callback); }
    void SetOnTurnedIn(QuestCallback callback) { m_OnTurnedIn = std::move(callback); }
    
private:
    std::string m_ID;               // Уникальный ID ("quest_find_sword")
    std::string m_Title;            // "Find the Lost Sword"
    std::string m_Description;      // Описание квеста
    std::string m_GiverNPC;         // NPC, дающий квест
    std::string m_CompletionNPC;    // NPC для сдачи квеста
    
    QuestState m_State;
    int m_Level;                    // Рекомендуемый уровень
    
    std::vector<QuestObjective> m_Objectives;
    QuestReward m_Reward;
    
    // Callbacks
    QuestCallback m_OnStarted;
    QuestCallback m_OnCompleted;
    QuestCallback m_OnFailed;
    QuestCallback m_OnTurnedIn;
};

} // namespace Quests
} // namespace SAGE
