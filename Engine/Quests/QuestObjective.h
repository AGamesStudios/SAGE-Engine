#pragma once

#include <string>
#include <functional>
#include "Core/Logger.h"

namespace SAGE {
namespace Quests {

/// @brief Тип квестовой цели
enum class ObjectiveType {
    Kill,           // Убить N врагов типа X
    Collect,        // Собрать N предметов
    TalkTo,         // Поговорить с NPC
    Reach,          // Достичь локации
    Trigger,        // Активировать триггер/событие
    Custom          // Пользовательская логика (через Lua)
};

/// @brief Состояние квестовой цели
enum class ObjectiveState {
    NotStarted,
    InProgress,
    Completed,
    Failed
};

/// @brief Одна цель квеста с прогрессом
class QuestObjective {
public:
    QuestObjective() = default;
    
    QuestObjective(const std::string& description, ObjectiveType type, int requiredCount = 1)
        : m_Description(description)
        , m_Type(type)
        , m_RequiredCount(requiredCount)
        , m_CurrentCount(0)
        , m_State(ObjectiveState::NotStarted)
        , m_Optional(false)
        , m_Hidden(false)
    {}
    
    // ========== Getters ==========
    
    const std::string& GetDescription() const { return m_Description; }
    const std::string& GetTargetID() const { return m_TargetID; }
    ObjectiveType GetType() const { return m_Type; }
    ObjectiveState GetState() const { return m_State; }
    
    int GetCurrentCount() const { return m_CurrentCount; }
    int GetRequiredCount() const { return m_RequiredCount; }
    
    bool IsOptional() const { return m_Optional; }
    bool IsHidden() const { return m_Hidden; }
    bool IsCompleted() const { return m_State == ObjectiveState::Completed; }
    bool IsFailed() const { return m_State == ObjectiveState::Failed; }
    
    /// @brief Прогресс в процентах (0.0 - 1.0)
    float GetProgress() const {
        if (m_RequiredCount <= 0) return 1.0f;
        return static_cast<float>(m_CurrentCount) / static_cast<float>(m_RequiredCount);
    }
    
    // ========== Setters ==========
    
    void SetDescription(const std::string& desc) { m_Description = desc; }
    void SetTargetID(const std::string& id) { m_TargetID = id; }
    void SetOptional(bool optional) { m_Optional = optional; }
    void SetHidden(bool hidden) { m_Hidden = hidden; }
    
    // ========== Progress Tracking ==========
    
    /// @brief Обновить прогресс (+1)
    bool IncrementProgress() {
        return UpdateProgress(m_CurrentCount + 1);
    }
    
    /// @brief Обновить прогресс с указанным значением
    bool UpdateProgress(int newCount) {
        if (m_State == ObjectiveState::Completed || m_State == ObjectiveState::Failed) {
            return false; // Уже завершена
        }
        
        m_CurrentCount = newCount;
        
        // Проверка завершения
        if (m_CurrentCount >= m_RequiredCount) {
            m_CurrentCount = m_RequiredCount; // Clamp
            m_State = ObjectiveState::Completed;
            SAGE_INFO("[Quest] Objective completed: {}", m_Description);
            return true;
        }
        
        m_State = ObjectiveState::InProgress;
        return false;
    }
    
    /// @brief Принудительное завершение (для Talk/Reach objectives)
    void Complete() {
        m_State = ObjectiveState::Completed;
        m_CurrentCount = m_RequiredCount;
        SAGE_INFO("[Quest] Objective completed: {}", m_Description);
    }
    
    /// @brief Провалить цель
    void Fail() {
        m_State = ObjectiveState::Failed;
        SAGE_WARNING("[Quest] Objective failed: {}", m_Description);
    }
    
    /// @brief Сбросить прогресс
    void Reset() {
        m_CurrentCount = 0;
        m_State = ObjectiveState::NotStarted;
    }
    
private:
    std::string m_Description;      // "Kill 10 Goblins"
    std::string m_TargetID;         // "goblin" (для Kill/Collect), "npc_elder" (для TalkTo)
    ObjectiveType m_Type;
    ObjectiveState m_State;
    
    int m_CurrentCount = 0;         // Текущий прогресс
    int m_RequiredCount = 1;        // Требуемое количество
    
    bool m_Optional = false;        // Опциональная цель
    bool m_Hidden = false;          // Скрыта до активации
};

} // namespace Quests
} // namespace SAGE
