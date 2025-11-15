#pragma once

#include "Event.h"
#include "Core/Logger.h"

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace SAGE {

/// @brief Пул объектов для событий (Object Pool Pattern)
/// @tparam EventT Тип события
/// @note Переиспользует события для избежания аллокаций
template<typename EventT>
class EventPool {
public:
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    /// @brief Конструктор с начальной емкостью
    /// @param initialCapacity Начальное количество событий в пуле
    explicit EventPool(size_t initialCapacity = 128);

    ~EventPool() = default;

    // Non-copyable
    EventPool(const EventPool&) = delete;
    EventPool& operator=(const EventPool&) = delete;

    // Movable
    EventPool(EventPool&&) noexcept = default;
    EventPool& operator=(EventPool&&) noexcept = default;

    /// @brief Получить событие из пула
    /// @return Указатель на событие (необходимо вызвать Release после использования)
    EventT* Acquire();

    /// @brief Вернуть событие в пул
    /// @param event Указатель на событие
    void Release(EventT* event);

    /// @brief Очистить пул (удалить все события)
    void Clear();

    /// @brief Предварительное создание событий
    /// @param count Количество событий для создания
    void Preallocate(size_t count);

    // ===== Statistics =====

    /// @brief Получить количество активных событий
    size_t GetActiveCount() const;

    /// @brief Получить количество доступных событий
    size_t GetAvailableCount() const;

    /// @brief Получить общее количество событий
    size_t GetTotalCount() const;

    /// @brief Получить максимальное количество активных событий
    size_t GetPeakActiveCount() const;

private:
    std::vector<std::unique_ptr<EventT>> m_Pool;  // Владение событиями
    std::queue<EventT*> m_Available;              // Доступные события
    mutable std::mutex m_Mutex;                   // Thread-safety

    // Statistics
    size_t m_ActiveCount = 0;
    size_t m_PeakActiveCount = 0;
};

/// @brief Глобальный менеджер пулов событий
/// @note Singleton для управления всеми пулами событий
class EventPoolManager {
public:
    /// @brief Получить instance (singleton)
    static EventPoolManager& Get();

    /// @brief Получить событие из пула
    /// @tparam EventT Тип события
    /// @return Указатель на событие
    template<typename EventT>
    EventT* Acquire();

    /// @brief Вернуть событие в пул
    /// @tparam EventT Тип события
    /// @param event Указатель на событие
    template<typename EventT>
    void Release(EventT* event);

    /// @brief Очистить все пулы
    void ClearAll();

    /// @brief Предварительное создание событий
    /// @tparam EventT Тип события
    /// @param count Количество событий
    template<typename EventT>
    void Preallocate(size_t count);

    /// @brief Получить статистику пула
    /// @tparam EventT Тип события
    template<typename EventT>
    struct PoolStats {
        size_t activeCount;
        size_t availableCount;
        size_t totalCount;
        size_t peakActiveCount;
    };

    template<typename EventT>
    PoolStats<EventT> GetPoolStats();

private:
    EventPoolManager() = default;
    ~EventPoolManager() = default;

    EventPoolManager(const EventPoolManager&) = delete;
    EventPoolManager& operator=(const EventPoolManager&) = delete;

    /// @brief Получить пул для типа события
    /// @tparam EventT Тип события
    template<typename EventT>
    EventPool<EventT>& GetPool();

    std::unordered_map<std::type_index, std::unique_ptr<void, void(*)(void*)>> m_Pools;
    mutable std::mutex m_Mutex;
};

// ===== EventPool IMPLEMENTATION =====

template<typename EventT>
EventPool<EventT>::EventPool(size_t initialCapacity) {
    Preallocate(initialCapacity);
}

template<typename EventT>
EventT* EventPool<EventT>::Acquire() {
    std::lock_guard lock(m_Mutex);

    EventT* event = nullptr;

    if (m_Available.empty()) {
        // Создаем новое событие
        auto newEvent = std::make_unique<EventT>();
        event = newEvent.get();
        m_Pool.push_back(std::move(newEvent));
    } else {
        // Берем из доступных
        event = m_Available.front();
        m_Available.pop();
    }

    // Сбрасываем событие для переиспользования
    event->Reset();

    m_ActiveCount++;
    if (m_ActiveCount > m_PeakActiveCount) {
        m_PeakActiveCount = m_ActiveCount;
    }

    return event;
}

template<typename EventT>
void EventPool<EventT>::Release(EventT* event) {
    if (!event) return;

    std::lock_guard lock(m_Mutex);

    // Возвращаем в пул
    m_Available.push(event);
    m_ActiveCount--;
}

template<typename EventT>
void EventPool<EventT>::Clear() {
    std::lock_guard lock(m_Mutex);

    m_Pool.clear();
    while (!m_Available.empty()) {
        m_Available.pop();
    }
    m_ActiveCount = 0;
    m_PeakActiveCount = 0;
}

template<typename EventT>
void EventPool<EventT>::Preallocate(size_t count) {
    std::lock_guard lock(m_Mutex);

    for (size_t i = 0; i < count; ++i) {
        auto event = std::make_unique<EventT>();
        m_Available.push(event.get());
        m_Pool.push_back(std::move(event));
    }
}

template<typename EventT>
size_t EventPool<EventT>::GetActiveCount() const {
    std::lock_guard lock(m_Mutex);
    return m_ActiveCount;
}

template<typename EventT>
size_t EventPool<EventT>::GetAvailableCount() const {
    std::lock_guard lock(m_Mutex);
    return m_Available.size();
}

template<typename EventT>
size_t EventPool<EventT>::GetTotalCount() const {
    std::lock_guard lock(m_Mutex);
    return m_Pool.size();
}

template<typename EventT>
size_t EventPool<EventT>::GetPeakActiveCount() const {
    std::lock_guard lock(m_Mutex);
    return m_PeakActiveCount;
}

// ===== EventPoolManager IMPLEMENTATION =====

inline EventPoolManager& EventPoolManager::Get() {
    static EventPoolManager instance;
    return instance;
}

template<typename EventT>
EventT* EventPoolManager::Acquire() {
    return GetPool<EventT>().Acquire();
}

template<typename EventT>
void EventPoolManager::Release(EventT* event) {
    GetPool<EventT>().Release(event);
}

inline void EventPoolManager::ClearAll() {
    std::lock_guard lock(m_Mutex);
    m_Pools.clear();
}

template<typename EventT>
void EventPoolManager::Preallocate(size_t count) {
    GetPool<EventT>().Preallocate(count);
}

template<typename EventT>
typename EventPoolManager::PoolStats<EventT> EventPoolManager::GetPoolStats() {
    auto& pool = GetPool<EventT>();
    return {
        pool.GetActiveCount(),
        pool.GetAvailableCount(),
        pool.GetTotalCount(),
        pool.GetPeakActiveCount()
    };
}

template<typename EventT>
EventPool<EventT>& EventPoolManager::GetPool() {
    std::lock_guard lock(m_Mutex);

    const std::type_index typeIndex(typeid(EventT));
    auto it = m_Pools.find(typeIndex);

    if (it == m_Pools.end()) {
        // Создаем новый пул
        auto pool = std::make_unique<EventPool<EventT>>();
        auto* poolPtr = pool.get();

        // Deleter для type-erased хранения
        auto deleter = [](void* p) {
            delete static_cast<EventPool<EventT>*>(p);
        };

        m_Pools.emplace(typeIndex, 
            std::unique_ptr<void, void(*)(void*)>(pool.release(), deleter));

        return *poolPtr;
    }

    return *static_cast<EventPool<EventT>*>(it->second.get());
}

/// @brief RAII wrapper для автоматического Release события
/// @tparam EventT Тип события
template<typename EventT>
class PooledEvent {
public:
    PooledEvent() : m_Event(EventPoolManager::Get().Acquire<EventT>()) {}

    ~PooledEvent() {
        if (m_Event) {
            EventPoolManager::Get().Release(m_Event);
        }
    }

    // Non-copyable
    PooledEvent(const PooledEvent&) = delete;
    PooledEvent& operator=(const PooledEvent&) = delete;

    // Movable
    PooledEvent(PooledEvent&& other) noexcept : m_Event(other.m_Event) {
        other.m_Event = nullptr;
    }

    PooledEvent& operator=(PooledEvent&& other) noexcept {
        if (this != &other) {
            if (m_Event) {
                EventPoolManager::Get().Release(m_Event);
            }
            m_Event = other.m_Event;
            other.m_Event = nullptr;
        }
        return *this;
    }

    EventT* operator->() { return m_Event; }
    const EventT* operator->() const { return m_Event; }

    EventT& operator*() { return *m_Event; }
    const EventT& operator*() const { return *m_Event; }

    EventT* Get() { return m_Event; }
    const EventT* Get() const { return m_Event; }

private:
    EventT* m_Event;
};

} // namespace SAGE
