#pragma once

#include "Event.h"
#include "EventBus.h"
#include "EventPool.h"
#include "Core/Logger.h"

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <functional>

namespace SAGE {

/// @brief Очередь событий для отложенной обработки
/// @note Поддерживает приоритеты и фильтрацию
class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    // Non-copyable
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    // Movable
    EventQueue(EventQueue&&) noexcept = default;
    EventQueue& operator=(EventQueue&&) noexcept = default;

    /// @brief Добавить событие в очередь
    /// @tparam EventT Тип события
    /// @param event Указатель на событие (владение НЕ передается)
    template<typename EventT>
    void Enqueue(const EventT* event);

    /// @brief Добавить событие в очередь с задержкой
    /// @tparam EventT Тип события
    /// @param event Указатель на событие
    /// @param delaySeconds Задержка в секундах
    template<typename EventT>
    void EnqueueDelayed(const EventT* event, double delaySeconds);

    /// @brief Обработать все события в очереди
    /// @param eventBus Шина событий для публикации
    void ProcessAll(EventBus& eventBus);

    /// @brief Обработать события с определенным приоритетом
    /// @param eventBus Шина событий
    /// @param minPriority Минимальный приоритет для обработки
    void ProcessByPriority(EventBus& eventBus, int minPriority);

    /// @brief Обработать события определенного типа
    /// @param eventBus Шина событий
    /// @param eventType Тип события
    void ProcessByType(EventBus& eventBus, EventType eventType);

    /// @brief Обработать только готовые отложенные события
    /// @param eventBus Шина событий
    /// @param currentTime Текущее время
    void ProcessReady(EventBus& eventBus, double currentTime);

    /// @brief Очистить очередь
    void Clear();

    /// @brief Проверить, пуста ли очередь
    bool IsEmpty() const;

    /// @brief Получить количество событий в очереди
    size_t GetCount() const;

    /// @brief Получить количество отложенных событий
    size_t GetDelayedCount() const;

    /// @brief Включить логирование
    void EnableLogging(bool enable = true);

    // ===== Statistics =====

    struct Stats {
        size_t totalEnqueued = 0;
        size_t totalProcessed = 0;
        size_t currentCount = 0;
        size_t delayedCount = 0;
        double averageProcessingTime = 0.0;
    };

    Stats GetStats() const;

private:
    /// @brief Внутреннее представление события
    struct QueuedEvent {
        std::unique_ptr<Event> event;   // Владение событием
        int priority;                   // Приоритет
        double processAfter;            // Время обработки (для отложенных)
        bool isDelayed;                 // Флаг отложенного события

        QueuedEvent(Event* evt, int prio, double delay = 0.0, bool delayed = false)
            : event(evt), priority(prio), processAfter(delay), isDelayed(delayed) {}

        // Для сортировки по приоритету (больший приоритет = раньше)
        bool operator<(const QueuedEvent& other) const {
            return priority < other.priority; // std::priority_queue - max heap
        }
    };

    std::priority_queue<QueuedEvent> m_Queue;           // Основная очередь (приоритетная)
    std::vector<QueuedEvent> m_DelayedQueue;            // Отложенные события
    mutable std::mutex m_Mutex;                         // Thread-safety
    bool m_LoggingEnabled = false;

    // Statistics
    Stats m_Stats;
};

// ===== IMPLEMENTATION =====

inline EventQueue::EventQueue() = default;

inline EventQueue::~EventQueue() {
    Clear();
}

template<typename EventT>
void EventQueue::Enqueue(const EventT* event) {
    if (!event) return;

    std::lock_guard lock(m_Mutex);

    // Копируем событие (владение передается очереди)
    auto copy = std::make_unique<EventT>(*event);
    int priority = copy->GetPriority();

    m_Queue.emplace(copy.release(), priority);

    m_Stats.totalEnqueued++;
    m_Stats.currentCount++;

    if (m_LoggingEnabled) {
        Logger::Get().Info("EventQueue", "Enqueued event: {}, Priority: {}, Queue Size: {}",
            copy->ToString(), priority, m_Stats.currentCount);
    }
}

template<typename EventT>
void EventQueue::EnqueueDelayed(const EventT* event, double delaySeconds) {
    if (!event || delaySeconds < 0.0) return;

    std::lock_guard lock(m_Mutex);

    // Копируем событие
    auto copy = std::make_unique<EventT>(*event);
    int priority = copy->GetPriority();
    double processAfter = Event::GetCurrentTime() + delaySeconds;

    m_DelayedQueue.emplace_back(copy.release(), priority, processAfter, true);

    m_Stats.totalEnqueued++;
    m_Stats.delayedCount++;

    if (m_LoggingEnabled) {
        Logger::Get().Info("EventQueue", "Enqueued delayed event: {}, Delay: {}s, Process After: {}",
            copy->ToString(), delaySeconds, processAfter);
    }
}

inline void EventQueue::ProcessAll(EventBus& eventBus) {
    std::lock_guard lock(m_Mutex);

    double startTime = Event::GetCurrentTime();
    size_t processed = 0;

    // Обрабатываем основную очередь
    while (!m_Queue.empty()) {
        auto& queuedEvent = const_cast<QueuedEvent&>(m_Queue.top());
        
        if (queuedEvent.event) {
            eventBus.PublishRaw(queuedEvent.event.get());
            processed++;
        }

        m_Queue.pop();
    }

    // Обновляем статистику
    m_Stats.totalProcessed += processed;
    m_Stats.currentCount = 0;

    double elapsed = Event::GetCurrentTime() - startTime;
    if (processed > 0) {
        m_Stats.averageProcessingTime = elapsed / processed;
    }

    if (m_LoggingEnabled && processed > 0) {
        Logger::Get().Info("EventQueue", "Processed {} events in {:.3f}ms",
            processed, elapsed * 1000.0);
    }
}

inline void EventQueue::ProcessByPriority(EventBus& eventBus, int minPriority) {
    std::lock_guard lock(m_Mutex);

    std::priority_queue<QueuedEvent> remaining;
    size_t processed = 0;

    while (!m_Queue.empty()) {
        auto& queuedEvent = const_cast<QueuedEvent&>(m_Queue.top());

        if (queuedEvent.priority >= minPriority && queuedEvent.event) {
            eventBus.PublishRaw(queuedEvent.event.get());
            processed++;
        } else {
            // Сохраняем для возврата в очередь
            remaining.push(std::move(const_cast<QueuedEvent&>(queuedEvent)));
        }

        m_Queue.pop();
    }

    // Возвращаем необработанные события
    m_Queue = std::move(remaining);

    m_Stats.totalProcessed += processed;
    m_Stats.currentCount = m_Queue.size();
}

inline void EventQueue::ProcessByType(EventBus& eventBus, EventType eventType) {
    std::lock_guard lock(m_Mutex);

    std::priority_queue<QueuedEvent> remaining;
    size_t processed = 0;

    while (!m_Queue.empty()) {
        auto& queuedEvent = const_cast<QueuedEvent&>(m_Queue.top());

        if (queuedEvent.event && queuedEvent.event->GetType() == eventType) {
            eventBus.PublishRaw(queuedEvent.event.get());
            processed++;
        } else {
            remaining.push(std::move(const_cast<QueuedEvent&>(queuedEvent)));
        }

        m_Queue.pop();
    }

    m_Queue = std::move(remaining);

    m_Stats.totalProcessed += processed;
    m_Stats.currentCount = m_Queue.size();
}

inline void EventQueue::ProcessReady(EventBus& eventBus, double currentTime) {
    std::lock_guard lock(m_Mutex);

    std::vector<QueuedEvent> stillDelayed;
    size_t processed = 0;

    // Проверяем отложенные события
    for (auto& queuedEvent : m_DelayedQueue) {
        if (currentTime >= queuedEvent.processAfter && queuedEvent.event) {
            eventBus.PublishRaw(queuedEvent.event.get());
            processed++;
        } else {
            stillDelayed.push_back(std::move(queuedEvent));
        }
    }

    m_DelayedQueue = std::move(stillDelayed);

    m_Stats.totalProcessed += processed;
    m_Stats.delayedCount = m_DelayedQueue.size();

    if (m_LoggingEnabled && processed > 0) {
        Logger::Get().Info("EventQueue", "Processed {} delayed events, {} remaining",
            processed, m_Stats.delayedCount);
    }
}

inline void EventQueue::Clear() {
    std::lock_guard lock(m_Mutex);

    while (!m_Queue.empty()) {
        m_Queue.pop();
    }

    m_DelayedQueue.clear();

    m_Stats.currentCount = 0;
    m_Stats.delayedCount = 0;
}

inline bool EventQueue::IsEmpty() const {
    std::lock_guard lock(m_Mutex);
    return m_Queue.empty() && m_DelayedQueue.empty();
}

inline size_t EventQueue::GetCount() const {
    std::lock_guard lock(m_Mutex);
    return m_Queue.size();
}

inline size_t EventQueue::GetDelayedCount() const {
    std::lock_guard lock(m_Mutex);
    return m_DelayedQueue.size();
}

inline void EventQueue::EnableLogging(bool enable) {
    m_LoggingEnabled = enable;
}

inline EventQueue::Stats EventQueue::GetStats() const {
    std::lock_guard lock(m_Mutex);
    return m_Stats;
}

} // namespace SAGE
