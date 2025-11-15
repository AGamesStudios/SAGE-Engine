#pragma once

#include "Event.h"
#include "Core/Logger.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SAGE {

/// @brief Центральная шина событий с поддержкой приоритетов и фильтрации
/// @note Thread-safe для одновременной подписки/публикации
class EventBus {
public:
    using HandlerId = uint64_t;
    using HookId = uint64_t;

    /// Hook callback signature. Pre-publish receives const Event&, post-publish receives const Event& and processing duration in microseconds.
    using PrePublishHook = std::function<void(const Event&)>;
    using PostPublishHook = std::function<void(const Event&, uint64_t)>; // duration (µs)

    EventBus();
    ~EventBus() = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) noexcept = default;
    EventBus& operator=(EventBus&&) noexcept = default;

    // ===== Subscription API =====

    /// @brief Подписаться на событие с приоритетом
    /// @tparam EventT Тип события
    /// @param callback Функция-обработчик
    /// @param priority Приоритет (больше = раньше)
    /// @param debugName Имя для отладки (опционально)
    /// @return ID подписки для последующей отписки
    template<typename EventT, typename Callable>
    HandlerId Subscribe(
        Callable&& callback,
        int priority = 0,
        const char* debugName = nullptr
    );

    /// @brief Условная подписка (с фильтром)
    /// @tparam EventT Тип события
    /// @param filter Функция-фильтр (возвращает true если обрабатывать)
    /// @param callback Функция-обработчик
    /// @param priority Приоритет
    /// @param debugName Имя для отладки
    /// @return ID подписки
    template<typename EventT, typename FilterFn, typename Callable>
    HandlerId SubscribeIf(
        FilterFn&& filter,
        Callable&& callback,
        int priority = 0,
        const char* debugName = nullptr
    );

    /// @brief Временная подписка (один раз)
    /// @tparam EventT Тип события
    /// @param callback Функция-обработчик
    /// @param priority Приоритет
    /// @param debugName Имя для отладки
    /// @return ID подписки
    template<typename EventT, typename Callable>
    HandlerId SubscribeOnce(
        Callable&& callback,
        int priority = 0,
        const char* debugName = nullptr
    );

    /// @brief Отписаться от события
    /// @tparam EventT Тип события
    /// @param id ID подписки
    template<typename EventT>
    void Unsubscribe(HandlerId id);

    /// @brief Отписаться от всех событий данного типа
    /// @tparam EventT Тип события
    template<typename EventT>
    void UnsubscribeAll();

    // ===== Publishing API =====

    /// @brief Опубликовать событие (немедленная обработка)
    /// @tparam EventT Тип события
    /// @param event Событие
    template<typename EventT>
    void Publish(EventT& event);

    /// @brief Опубликовать событие (полиморфная версия)
    /// @param event Событие
    void Publish(Event& event);

    // ===== Management =====

    /// @brief Очистить все подписки
    void Clear();

    /// @brief Получить количество подписчиков для типа события
    /// @tparam EventT Тип события
    /// @return Количество подписчиков
    template<typename EventT>
    size_t GetSubscriberCount() const;

    // ===== Statistics =====

    /// @brief Статистика EventBus
    struct Stats {
        size_t totalSubscribers = 0;    // Всего подписчиков
        size_t totalPublished = 0;      // Всего опубликовано событий
        size_t totalHandled = 0;        // Всего обработано событий
        double averageProcessingTime = 0.0; // Среднее время обработки (мс)
    };

    /// @brief Получить статистику
    Stats GetStats() const;

    /// @brief Сбросить статистику
    void ResetStats();

    // ===== Debugging =====

    /// @brief Включить логирование событий
    void EnableLogging(bool enable) { m_LoggingEnabled = enable; }

    /// @brief Проверить включено ли логирование
    bool IsLoggingEnabled() const { return m_LoggingEnabled; }

    // ===== Hook API =====
    /// Register a hook called before every event is dispatched. Returns hook id for removal.
    HookId AddPrePublishHook(PrePublishHook hook);
    /// Register a hook called after every event is dispatched. Receives processing duration for that event publish.
    HookId AddPostPublishHook(PostPublishHook hook);
    /// Remove a previously registered pre-publish hook.
    void RemovePrePublishHook(HookId id);
    /// Remove a previously registered post-publish hook.
    void RemovePostPublishHook(HookId id);

private:
    /// @brief Запись обработчика
    struct HandlerRecord {
        HandlerId id;                               // Уникальный ID
        std::function<void(Event&)> invoker;        // Обработчик
        std::function<bool(const Event&)> filter;   // Фильтр (nullptr = нет фильтра)
        int priority;                               // Приоритет
        const char* debugName;                      // Имя для отладки
        bool oneShot;                               // Удалить после первого вызова
        
        // Для сортировки по приоритету (больше = раньше)
        bool operator<(const HandlerRecord& other) const {
            return priority > other.priority;
        }
    };

    using HandlerList = std::vector<HandlerRecord>;
    using TypeIndex = std::type_index;

    /// @brief Получить обработчики для типа события
    template<typename EventT>
    HandlerList* GetHandlers();

    /// @brief Получить обработчики для типа события (const)
    template<typename EventT>
    const HandlerList* GetHandlers() const;

    /// @brief Сортировать обработчики по приоритету
    void SortHandlers(HandlerList& handlers);

    /// @brief Логировать событие
    void LogEvent(const Event& event, const char* action);

    // ===== Member Variables =====

    std::unordered_map<TypeIndex, HandlerList> m_Handlers;
    HandlerId m_NextId;
    /**
     * @brief Thread-safety mutex for subscriptions (mutable for const methods)
     * 
     * Protects subscription/unsubscription operations.
     * Uses reader-writer pattern with std::shared_mutex.
     */
    mutable std::shared_mutex m_Mutex;

    // Statistics
    mutable Stats m_Stats;
    mutable std::mutex m_StatsMutex;

    // Debugging
    bool m_LoggingEnabled = false;

    // Hooks
    HookId m_NextHookId = 1;
    struct PreHookRecord { HookId id; PrePublishHook fn; };
    struct PostHookRecord { HookId id; PostPublishHook fn; };
    std::vector<PreHookRecord> m_PreHooks;
    std::vector<PostHookRecord> m_PostHooks;
    /**
     * @brief Separate mutex for event hooks (mutable for const methods)
     * 
     * Uses separate mutex from m_Mutex to avoid contention between:
     * - Hook registration/invocation (m_HookMutex)
     * - Subscription operations (m_Mutex)
     * This reduces lock contention and improves concurrency.
     */
    mutable std::shared_mutex m_HookMutex;

public:
    // ===== Generic unsubscribe (non-templated) =====
    void UnsubscribeByType(const std::type_index& typeIndex, HandlerId id) {
        std::unique_lock lock(m_Mutex);
        auto it = m_Handlers.find(typeIndex);
        if (it == m_Handlers.end()) return;
        auto& handlers = it->second;
        auto handlerIt = std::find_if(handlers.begin(), handlers.end(), [id](const HandlerRecord& r){ return r.id == id; });
        if (handlerIt != handlers.end()) {
            if (m_LoggingEnabled && handlerIt->debugName) {
                SAGE_INFO("EventBus: Unsubscribed '{}' from {} (generic)", handlerIt->debugName, typeIndex.name());
            }
            handlers.erase(handlerIt);
            {
                std::lock_guard statsLock(m_StatsMutex);
                if (m_Stats.totalSubscribers > 0) m_Stats.totalSubscribers--; // prevent underflow
            }
            if (handlers.empty()) {
                m_Handlers.erase(it);
            }
        }
    }
};

// ===== IMPLEMENTATION =====

inline EventBus::EventBus()
    : m_NextId(1)
    , m_LoggingEnabled(false)
    , m_NextHookId(1)
{}

template<typename EventT, typename Callable>
EventBus::HandlerId EventBus::Subscribe(
    Callable&& callback,
    int priority,
    const char* debugName
) {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::unique_lock lock(m_Mutex);

    const TypeIndex typeIndex(typeid(EventT));
    auto& handlers = m_Handlers[typeIndex];

    HandlerRecord record;
    record.id = m_NextId++;
    record.invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
        fn(static_cast<EventT&>(baseEvent));
    };
    record.filter = nullptr;
    record.priority = priority;
    record.debugName = debugName;
    record.oneShot = false;

    handlers.push_back(std::move(record));
    SortHandlers(handlers);

    // Update stats
    {
        std::lock_guard statsLock(m_StatsMutex);
        m_Stats.totalSubscribers++;
    }

    if (m_LoggingEnabled && debugName) {
        SAGE_INFO("EventBus: Subscribed '{}' to {} (priority: {})", 
                  debugName, typeid(EventT).name(), priority);
    }

    return record.id;
}

template<typename EventT, typename FilterFn, typename Callable>
EventBus::HandlerId EventBus::SubscribeIf(
    FilterFn&& filter,
    Callable&& callback,
    int priority,
    const char* debugName
) {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::unique_lock lock(m_Mutex);

    const TypeIndex typeIndex(typeid(EventT));
    auto& handlers = m_Handlers[typeIndex];

    HandlerRecord record;
    record.id = m_NextId++;
    record.invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
        fn(static_cast<EventT&>(baseEvent));
    };
    record.filter = [filterFn = std::forward<FilterFn>(filter)](const Event& baseEvent) {
        return filterFn(static_cast<const EventT&>(baseEvent));
    };
    record.priority = priority;
    record.debugName = debugName;
    record.oneShot = false;

    handlers.push_back(std::move(record));
    SortHandlers(handlers);

    {
        std::lock_guard statsLock(m_StatsMutex);
        m_Stats.totalSubscribers++;
    }

    if (m_LoggingEnabled && debugName) {
        SAGE_INFO("EventBus: Subscribed '{}' (with filter) to {} (priority: {})", 
                  debugName, typeid(EventT).name(), priority);
    }

    return record.id;
}

template<typename EventT, typename Callable>
EventBus::HandlerId EventBus::SubscribeOnce(
    Callable&& callback,
    int priority,
    const char* debugName
) {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::unique_lock lock(m_Mutex);

    const TypeIndex typeIndex(typeid(EventT));
    auto& handlers = m_Handlers[typeIndex];

    HandlerRecord record;
    record.id = m_NextId++;
    record.invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
        fn(static_cast<EventT&>(baseEvent));
    };
    record.filter = nullptr;
    record.priority = priority;
    record.debugName = debugName;
    record.oneShot = true;  // Удалить после первого вызова

    handlers.push_back(std::move(record));
    SortHandlers(handlers);

    {
        std::lock_guard statsLock(m_StatsMutex);
        m_Stats.totalSubscribers++;
    }

    if (m_LoggingEnabled && debugName) {
        SAGE_INFO("EventBus: Subscribed '{}' (once) to {} (priority: {})", 
                  debugName, typeid(EventT).name(), priority);
    }

    return record.id;
}

template<typename EventT>
void EventBus::Unsubscribe(HandlerId id) {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::unique_lock lock(m_Mutex);

    const TypeIndex typeIndex(typeid(EventT));
    auto it = m_Handlers.find(typeIndex);
    if (it == m_Handlers.end()) {
        return;
    }

    auto& handlers = it->second;
    auto handlerIt = std::find_if(handlers.begin(), handlers.end(),
        [id](const HandlerRecord& record) { return record.id == id; });

    if (handlerIt != handlers.end()) {
        if (m_LoggingEnabled && handlerIt->debugName) {
            SAGE_INFO("EventBus: Unsubscribed '{}' from {}", 
                      handlerIt->debugName, typeid(EventT).name());
        }

        handlers.erase(handlerIt);

        {
            std::lock_guard statsLock(m_StatsMutex);
            m_Stats.totalSubscribers--;
        }

        if (handlers.empty()) {
            m_Handlers.erase(it);
        }
    }
}

template<typename EventT>
void EventBus::UnsubscribeAll() {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::unique_lock lock(m_Mutex);

    const TypeIndex typeIndex(typeid(EventT));
    auto it = m_Handlers.find(typeIndex);
    if (it != m_Handlers.end()) {
        size_t count = it->second.size();
        m_Handlers.erase(it);

        {
            std::lock_guard statsLock(m_StatsMutex);
            m_Stats.totalSubscribers -= count;
        }

        if (m_LoggingEnabled) {
            SAGE_INFO("EventBus: Unsubscribed all ({}) from {}", count, typeid(EventT).name());
        }
    }
}

template<typename EventT>
void EventBus::Publish(EventT& event) {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    HandlerList handlersCopy;
    {
        std::shared_lock lock(m_Mutex);
        const TypeIndex typeIndex(typeid(EventT));
        auto it = m_Handlers.find(typeIndex);
        if (it != m_Handlers.end()) {
            handlersCopy = it->second;
        }
    }

    if (handlersCopy.empty()) {
        return;
    }

    if (m_LoggingEnabled) {
        LogEvent(event, "Publishing");
    }

    // Invoke pre-publish hooks (read lock)
    {
        std::shared_lock hookLock(m_HookMutex);
        for (auto& rec : m_PreHooks) {
            if (rec.fn) rec.fn(event);
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<HandlerId> oneShotHandlers;

    for (auto& handler : handlersCopy) {
        // Проверка фильтра
        if (handler.filter && !handler.filter(event)) {
            continue;
        }

        // Вызов обработчика
        handler.invoker(event);

        // Собираем one-shot обработчики для удаления
        if (handler.oneShot) {
            oneShotHandlers.push_back(handler.id);
        }

        // Остановка распространения
        if (event.IsPropagationStopped()) {
            break;
        }
    }

    // Удаляем one-shot обработчики
    if (!oneShotHandlers.empty()) {
        std::unique_lock lock(m_Mutex);
        const TypeIndex typeIndex(typeid(EventT));
        auto it = m_Handlers.find(typeIndex);
        if (it != m_Handlers.end()) {
            auto& handlers = it->second;
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(),
                    [&oneShotHandlers](const HandlerRecord& record) {
                        return std::find(oneShotHandlers.begin(), oneShotHandlers.end(), 
                                       record.id) != oneShotHandlers.end();
                    }),
                handlers.end()
            );
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // Update stats
    {
        std::lock_guard statsLock(m_StatsMutex);
        m_Stats.totalPublished++;
        if (event.IsHandled()) {
            m_Stats.totalHandled++;
        }
        
        // Rolling average
        double newTime = duration.count() / 1000.0; // В миллисекундах
        m_Stats.averageProcessingTime = 
            (m_Stats.averageProcessingTime * (m_Stats.totalPublished - 1) + newTime) 
            / m_Stats.totalPublished;
    }
    // Invoke post-publish hooks
    {
        std::shared_lock hookLock(m_HookMutex);
        for (auto& rec : m_PostHooks) {
            if (rec.fn) rec.fn(event, static_cast<uint64_t>(duration.count()));
        }
    }
}

inline void EventBus::Publish(Event& event) {
    // Generic polymorphic dispatch path. Mirrors templated Publish logic.

    // Copy handlers under shared lock
    HandlerList handlersCopy;
    std::type_index typeIndex(typeid(event));
    {
        std::shared_lock lock(m_Mutex);
        auto it = m_Handlers.find(typeIndex);
        if (it != m_Handlers.end()) {
            handlersCopy = it->second;
        }
    }
    if (handlersCopy.empty()) {
        return; // no handlers
    }

    if (m_LoggingEnabled) {
        LogEvent(event, "Publishing(poly)");
    }

    // Pre-publish hooks
    {
        std::shared_lock hookLock(m_HookMutex);
        for (auto& rec : m_PreHooks) {
            if (rec.fn) rec.fn(event);
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<HandlerId> oneShotHandlers;

    for (auto& handler : handlersCopy) {
        if (handler.filter && !handler.filter(event)) continue;
        handler.invoker(event);
        if (handler.oneShot) oneShotHandlers.push_back(handler.id);
        if (event.IsPropagationStopped()) break;
    }

    // Remove one-shot handlers
    if (!oneShotHandlers.empty()) {
        std::unique_lock lock(m_Mutex);
        auto it = m_Handlers.find(typeIndex);
        if (it != m_Handlers.end()) {
            auto& handlers = it->second;
            handlers.erase(std::remove_if(handlers.begin(), handlers.end(), [&](const HandlerRecord& r){ return std::find(oneShotHandlers.begin(), oneShotHandlers.end(), r.id) != oneShotHandlers.end(); }), handlers.end());
            if (handlers.empty()) m_Handlers.erase(it);
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // Stats
    {
        std::lock_guard statsLock(m_StatsMutex);
        m_Stats.totalPublished++;
        if (event.IsHandled()) m_Stats.totalHandled++;
        double newTimeMs = duration.count() / 1000.0;
        m_Stats.averageProcessingTime = (m_Stats.averageProcessingTime * (m_Stats.totalPublished - 1) + newTimeMs) / m_Stats.totalPublished;
    }

    // Post hooks
    {
        std::shared_lock hookLock(m_HookMutex);
        for (auto& rec : m_PostHooks) {
            if (rec.fn) rec.fn(event, static_cast<uint64_t>(duration.count()));
        }
    }
}

inline void EventBus::Clear() {
    std::unique_lock lock(m_Mutex);
    m_Handlers.clear();

    std::lock_guard statsLock(m_StatsMutex);
    m_Stats.totalSubscribers = 0;
}

template<typename EventT>
size_t EventBus::GetSubscriberCount() const {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");

    std::shared_lock lock(m_Mutex);
    const TypeIndex typeIndex(typeid(EventT));
    auto it = m_Handlers.find(typeIndex);
    return (it != m_Handlers.end()) ? it->second.size() : 0;
}

inline EventBus::Stats EventBus::GetStats() const {
    std::lock_guard lock(m_StatsMutex);
    return m_Stats;
}

inline void EventBus::ResetStats() {
    std::lock_guard lock(m_StatsMutex);
    m_Stats = Stats{};
    m_Stats.totalSubscribers = 0;
    
    // Count current subscribers
    std::shared_lock handlersLock(m_Mutex);
    for (const auto& [type, handlers] : m_Handlers) {
        m_Stats.totalSubscribers += handlers.size();
    }
}

inline void EventBus::SortHandlers(HandlerList& handlers) {
    std::sort(handlers.begin(), handlers.end());
}

inline void EventBus::LogEvent(const Event& event, const char* action) {
    SAGE_TRACE("EventBus: {} event '{}' (ID: {}, Priority: {}, Age: {:.3f}ms)",
               action, event.GetName(), event.GetEventId(), 
               event.GetPriority(), event.GetAge() * 1000.0);
}

inline EventBus::HookId EventBus::AddPrePublishHook(PrePublishHook hook) {
    std::unique_lock lock(m_HookMutex);
    HookId id = m_NextHookId++;
    m_PreHooks.push_back({id, std::move(hook)});
    return id;
}

inline EventBus::HookId EventBus::AddPostPublishHook(PostPublishHook hook) {
    std::unique_lock lock(m_HookMutex);
    HookId id = m_NextHookId++;
    m_PostHooks.push_back({id, std::move(hook)});
    return id;
}

inline void EventBus::RemovePrePublishHook(HookId id) {
    std::unique_lock lock(m_HookMutex);
    m_PreHooks.erase(std::remove_if(m_PreHooks.begin(), m_PreHooks.end(), [id](const PreHookRecord& r){ return r.id == id; }), m_PreHooks.end());
}

inline void EventBus::RemovePostPublishHook(HookId id) {
    std::unique_lock lock(m_HookMutex);
    m_PostHooks.erase(std::remove_if(m_PostHooks.begin(), m_PostHooks.end(), [id](const PostHookRecord& r){ return r.id == id; }), m_PostHooks.end());
}

} // namespace SAGE
