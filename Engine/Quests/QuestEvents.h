#pragma once

#include "../Core/Event.h"
#include "Quest.h"
#include <string>

namespace SAGE {
namespace Quests {

/// @brief Событие: квест начат
class QuestStartedEvent : public Event {
public:
    QuestStartedEvent(const std::string& questID, const std::string& questTitle)
        : m_QuestID(questID), m_QuestTitle(questTitle) {}
    
    const std::string& GetQuestID() const { return m_QuestID; }
    const std::string& GetQuestTitle() const { return m_QuestTitle; }
    
    EVENT_CLASS_TYPE(Custom)
    EVENT_CLASS_CATEGORY(EventCategory::Gameplay)
    
    std::string ToString() const override {
        return "QuestStartedEvent: " + m_QuestTitle + " (" + m_QuestID + ")";
    }

private:
    std::string m_QuestID;
    std::string m_QuestTitle;
};

/// @brief Событие: цель квеста обновлена
class ObjectiveUpdatedEvent : public Event {
public:
    ObjectiveUpdatedEvent(const std::string& questID, size_t objectiveIndex, 
                          int currentCount, int requiredCount, bool completed)
        : m_QuestID(questID)
        , m_ObjectiveIndex(objectiveIndex)
        , m_CurrentCount(currentCount)
        , m_RequiredCount(requiredCount)
        , m_Completed(completed)
    {}
    
    const std::string& GetQuestID() const { return m_QuestID; }
    size_t GetObjectiveIndex() const { return m_ObjectiveIndex; }
    int GetCurrentCount() const { return m_CurrentCount; }
    int GetRequiredCount() const { return m_RequiredCount; }
    bool IsCompleted() const { return m_Completed; }
    
    EVENT_CLASS_TYPE(Custom)
    EVENT_CLASS_CATEGORY(EventCategory::Gameplay)
    
    std::string ToString() const override {
        return "ObjectiveUpdatedEvent: Quest=" + m_QuestID + 
               " Progress=" + std::to_string(m_CurrentCount) + "/" + std::to_string(m_RequiredCount);
    }

private:
    std::string m_QuestID;
    size_t m_ObjectiveIndex;
    int m_CurrentCount;
    int m_RequiredCount;
    bool m_Completed;
};

/// @brief Событие: квест завершен
class QuestCompletedEvent : public Event {
public:
    QuestCompletedEvent(const std::string& questID, const std::string& questTitle, const QuestReward& reward)
        : m_QuestID(questID), m_QuestTitle(questTitle), m_Reward(reward) {}
    
    const std::string& GetQuestID() const { return m_QuestID; }
    const std::string& GetQuestTitle() const { return m_QuestTitle; }
    const QuestReward& GetReward() const { return m_Reward; }
    
    EVENT_CLASS_TYPE(Custom)
    EVENT_CLASS_CATEGORY(EventCategory::Gameplay)
    
    std::string ToString() const override {
        return "QuestCompletedEvent: " + m_QuestTitle + " (+" + std::to_string(m_Reward.experience) + " XP)";
    }

private:
    std::string m_QuestID;
    std::string m_QuestTitle;
    QuestReward m_Reward;
};

/// @brief Событие: квест провален
class QuestFailedEvent : public Event {
public:
    QuestFailedEvent(const std::string& questID, const std::string& questTitle)
        : m_QuestID(questID), m_QuestTitle(questTitle) {}
    
    const std::string& GetQuestID() const { return m_QuestID; }
    const std::string& GetQuestTitle() const { return m_QuestTitle; }
    
    EVENT_CLASS_TYPE(Custom)
    EVENT_CLASS_CATEGORY(EventCategory::Gameplay)
    
    std::string ToString() const override {
        return "QuestFailedEvent: " + m_QuestTitle;
    }

private:
    std::string m_QuestID;
    std::string m_QuestTitle;
};

/// @brief Событие: квест сдан (награда получена)
class QuestTurnedInEvent : public Event {
public:
    QuestTurnedInEvent(const std::string& questID, const std::string& questTitle, const QuestReward& reward)
        : m_QuestID(questID), m_QuestTitle(questTitle), m_Reward(reward) {}
    
    const std::string& GetQuestID() const { return m_QuestID; }
    const std::string& GetQuestTitle() const { return m_QuestTitle; }
    const QuestReward& GetReward() const { return m_Reward; }
    
    EVENT_CLASS_TYPE(Custom)
    EVENT_CLASS_CATEGORY(EventCategory::Gameplay)
    
    std::string ToString() const override {
        return "QuestTurnedInEvent: " + m_QuestTitle;
    }

private:
    std::string m_QuestID;
    std::string m_QuestTitle;
    QuestReward m_Reward;
};

} // namespace Quests
} // namespace SAGE
