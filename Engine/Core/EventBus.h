#pragma once

#include "Event.h"
#include "Logger.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <exception>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <limits>
#include <memory>
#include <thread>
#include <chrono>
#include <condition_variable>

namespace SAGE {

    enum class EventPriority : std::uint8_t {
        Low = 0,
        Normal = 1,
        High = 2
    };

    class EventBus {
    public:
        using HandlerId = std::uint64_t;

        class SubscriptionHandle {
        public:
            SubscriptionHandle() = default;
            SubscriptionHandle(const SubscriptionHandle&) = delete;
            SubscriptionHandle& operator=(const SubscriptionHandle&) = delete;
            SubscriptionHandle(SubscriptionHandle&& other) noexcept;
            SubscriptionHandle& operator=(SubscriptionHandle&& other) noexcept;
            ~SubscriptionHandle();

            void Reset();
            void Release();
            bool IsActive() const;
            HandlerId GetId() const { return m_Id; }

        private:
            friend class EventBus;
            SubscriptionHandle(EventBus* bus, HandlerId id, const std::type_info& typeInfo);

            EventBus* m_Bus = nullptr;
            const std::type_info* m_TypeInfo = nullptr;
            HandlerId m_Id = 0;
        };

        struct DispatchStatistics {
            std::uint64_t TotalPublished = 0;
            std::uint64_t HandlersInvoked = 0;
        };

        EventBus();
        ~EventBus();

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&) noexcept = default;
        EventBus& operator=(EventBus&&) noexcept = default;

        template<typename EventT, typename Callable>
        HandlerId Subscribe(Callable&& callback, std::uint32_t groupId = 0);

        template<typename EventT, typename Callable>
        SubscriptionHandle SubscribeScoped(Callable&& callback, std::uint32_t groupId = 0);

        template<typename EventT, typename Owner, typename Callable>
        HandlerId SubscribeWeak(std::shared_ptr<Owner> owner, Callable&& callback, std::uint32_t groupId = 0);

        template<typename EventT>
        void Unsubscribe(HandlerId id);

        void Unsubscribe(HandlerId id);
        void UnsubscribeGroup(std::uint32_t groupId);

        template<typename EventT>
        void Publish(EventT& event);

        template<typename EventT>
        void Publish(const EventT& event);

        void Publish(Event& event);

        DispatchStatistics GetStatistics() const;
        void ResetStatistics();
        void EnableTracing(bool enable);
        bool IsTracingEnabled() const;

    void SetEnabledCategories(std::uint32_t mask);
    void EnableCategories(std::uint32_t mask);
    void DisableCategories(std::uint32_t mask);
    std::uint32_t GetEnabledCategories() const;

    void Enqueue(std::unique_ptr<Event> event, EventPriority priority = EventPriority::Normal);

    template<typename EventT, typename... Args>
    void Enqueue(EventPriority priority, Args&&... args);

    template<typename EventT, typename... Args>
    void Enqueue(Args&&... args);

    void Flush();

        void StartWorker(std::chrono::milliseconds interval = std::chrono::milliseconds(16));
        void StopWorker();
        bool IsWorkerRunning() const;

        void Clear();

    private:
        struct HandlerRecord {
            HandlerId Id = 0;
            std::function<void(Event&)> Invoker;
            std::uint32_t GroupId = 0;
            std::weak_ptr<void> WeakOwner; // Optional weak reference to owner
        };

        using HandlerList = std::vector<HandlerRecord>;

        void PublishInternal(Event& event, const std::type_index& typeIndex);
        void RecordDispatch(std::size_t handlerCount, const Event& event);
        bool ShouldTrace() const;
        void Unsubscribe(HandlerId id, const std::type_index& typeIndex);
        bool ShouldDeliver(const Event& event) const;

        std::unordered_map<std::type_index, HandlerList> m_Handlers;
        std::unordered_map<HandlerId, std::type_index> m_HandlerLookup;
        std::atomic<std::uint64_t> m_TotalPublished;
        std::atomic<std::uint64_t> m_TotalHandlersInvoked;
        std::atomic<bool> m_TracingEnabled;
        std::atomic<std::uint32_t> m_EnabledCategories;
        HandlerId m_NextId;
        /**
         * @brief Thread-safety mutex (mutable for const methods)
         * 
         * Protects event subscription and dispatch operations.
         * Uses reader-writer pattern for concurrent event processing.
         */
        mutable std::shared_mutex m_Mutex;
        std::vector<std::unique_ptr<Event>> m_ActiveQueue;
        std::vector<std::unique_ptr<Event>> m_PendingQueue;
        std::unordered_map<EventPriority, std::vector<std::unique_ptr<Event>>> m_PriorityActiveQueues;
        std::unordered_map<EventPriority, std::vector<std::unique_ptr<Event>>> m_PriorityPendingQueues;
        std::unordered_map<EventPriority, std::unordered_map<std::size_t, std::size_t>> m_CoalescingMaps; // key → queue index
        mutable std::mutex m_QueueMutex;
        std::atomic<bool> m_WorkerRunning;
        std::thread m_WorkerThread;
        std::condition_variable m_WorkerCV;
        std::mutex m_WorkerMutex;
    };

    inline EventBus::SubscriptionHandle::SubscriptionHandle(EventBus* bus, HandlerId id, const std::type_info& typeInfo)
        : m_Bus(bus)
        , m_TypeInfo(&typeInfo)
        , m_Id(id) {}

    inline EventBus::SubscriptionHandle::SubscriptionHandle(SubscriptionHandle&& other) noexcept
        : m_Bus(other.m_Bus)
        , m_TypeInfo(other.m_TypeInfo)
        , m_Id(other.m_Id) {
        other.Release();
    }

    inline EventBus::SubscriptionHandle& EventBus::SubscriptionHandle::operator=(SubscriptionHandle&& other) noexcept {
        if (this != &other) {
            Reset();
            m_Bus = other.m_Bus;
            m_TypeInfo = other.m_TypeInfo;
            m_Id = other.m_Id;
            other.Release();
        }
        return *this;
    }

    inline EventBus::SubscriptionHandle::~SubscriptionHandle() {
        Reset();
    }

    inline void EventBus::SubscriptionHandle::Reset() {
        if (m_Bus && m_TypeInfo && m_Id != 0) {
            m_Bus->Unsubscribe(m_Id, std::type_index(*m_TypeInfo));
        }
        Release();
    }

    inline void EventBus::SubscriptionHandle::Release() {
        m_Bus = nullptr;
        m_TypeInfo = nullptr;
        m_Id = 0;
    }

    inline bool EventBus::SubscriptionHandle::IsActive() const {
        return m_Bus != nullptr && m_Id != 0;
    }

    inline EventBus::EventBus()
        : m_TotalPublished(0)
        , m_TotalHandlersInvoked(0)
        , m_TracingEnabled(false)
        , m_EnabledCategories(std::numeric_limits<std::uint32_t>::max())
        , m_NextId(1)
        , m_WorkerRunning(false) {}

    template<typename EventT, typename Callable>
    EventBus::HandlerId EventBus::Subscribe(Callable&& callback, std::uint32_t groupId) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        std::unique_lock lock(m_Mutex);
        
        const std::type_index typeIndex(typeid(EventT));
        auto& handlers = m_Handlers[typeIndex];
        const bool firstRegistration = handlers.empty();
        if (firstRegistration) {
            handlers.reserve(4); // Small initial reserve avoids reallocation during burst subscriptions.
        }

        HandlerRecord record;
        record.Id = m_NextId++;
        record.Invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
            fn(static_cast<EventT&>(baseEvent));
        };
        record.GroupId = groupId;

        const HandlerId handlerId = record.Id;
        handlers.emplace_back(std::move(record));
        m_HandlerLookup.emplace(handlerId, typeIndex);

        if (ShouldTrace()) {
            SAGE_TRACE("EventBus: handler {0} subscribed for type {1} (group {2})", handlerId, typeid(EventT).name(), groupId);
        }

        return handlerId;
    }

    template<typename EventT, typename Callable>
    EventBus::SubscriptionHandle EventBus::SubscribeScoped(Callable&& callback, std::uint32_t groupId) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        const HandlerId id = Subscribe<EventT>(std::forward<Callable>(callback), groupId);
        return SubscriptionHandle(this, id, typeid(EventT));
    }

    template<typename EventT, typename Owner, typename Callable>
    EventBus::HandlerId EventBus::SubscribeWeak(std::shared_ptr<Owner> owner, Callable&& callback, std::uint32_t groupId) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        
        if (!owner) {
            return 0; // Invalid subscription
        }

        std::unique_lock lock(m_Mutex);
        
        const std::type_index typeIndex(typeid(EventT));
        auto& handlers = m_Handlers[typeIndex];
        const bool firstRegistration = handlers.empty();
        if (firstRegistration) {
            handlers.reserve(4);
        }

        HandlerRecord record;
        record.Id = m_NextId++;
        record.Invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
            fn(static_cast<EventT&>(baseEvent));
        };
        record.GroupId = groupId;
        record.WeakOwner = owner; // Store weak reference

        const HandlerId handlerId = record.Id;
        handlers.emplace_back(std::move(record));
        m_HandlerLookup.emplace(handlerId, typeIndex);

        if (ShouldTrace()) {
            SAGE_TRACE("EventBus: handler {0} (weak) subscribed for type {1} (group {2})", handlerId, typeid(EventT).name(), groupId);
        }

        return handlerId;
    }

    template<typename EventT>
    void EventBus::Unsubscribe(HandlerId id) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        Unsubscribe(id, std::type_index(typeid(EventT)));
    }

    template<typename EventT>
    void EventBus::Publish(EventT& event) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        PublishInternal(event, std::type_index(typeid(EventT)));
    }

    template<typename EventT>
    void EventBus::Publish(const EventT& event) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        static_assert(std::is_copy_constructible_v<EventT>, "Const publish requires copyable EventT");
        auto eventCopy = event;
        Publish(eventCopy);
    }

    template<typename EventT, typename... Args>
    void EventBus::Enqueue(EventPriority priority, Args&&... args) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        Enqueue(std::make_unique<EventT>(std::forward<Args>(args)...), priority);
    }

    template<typename EventT, typename... Args>
    void EventBus::Enqueue(Args&&... args) {
        Enqueue<EventT>(EventPriority::Normal, std::forward<Args>(args)...);
    }

    inline void EventBus::Enqueue(std::unique_ptr<Event> event, EventPriority priority) {
        if (!event) {
            return;
        }

        {
            std::lock_guard queueLock(m_QueueMutex);
            auto& queue = m_PriorityPendingQueues[priority];
            
            if (event->CanCoalesce()) {
                const std::size_t key = event->GetCoalescingKey();
                auto& coalescingMap = m_CoalescingMaps[priority];
                
                auto it = coalescingMap.find(key);
                if (it != coalescingMap.end()) {
                    // Replace old event with new one at same position
                    queue[it->second] = std::move(event);
                    if (ShouldTrace()) {
                        SAGE_TRACE("EventBus: coalesced event at key {} (priority={})", key, static_cast<int>(priority));
                    }
                    return;
                }
                
                // First instance of this key
                coalescingMap[key] = queue.size();
            }
            
            queue.emplace_back(std::move(event));
        }

        if (ShouldTrace()) {
            SAGE_TRACE("EventBus: enqueued event for deferred publish (priority={})", static_cast<int>(priority));
        }
    }

    inline void EventBus::Flush() {
        {
            std::lock_guard queueLock(m_QueueMutex);
            bool anyPending = false;
            for (const auto& [prio, queue] : m_PriorityPendingQueues) {
                if (!queue.empty()) {
                    anyPending = true;
                    break;
                }
            }
            if (!anyPending) {
                return;
            }
            for (auto& [prio, queue] : m_PriorityPendingQueues) {
                auto& active = m_PriorityActiveQueues[prio];
                std::swap(active, queue);
            }
            // Clear coalescing maps after swap
            m_CoalescingMaps.clear();
        }

        // Process High → Normal → Low
        for (auto prio : {EventPriority::High, EventPriority::Normal, EventPriority::Low}) {
            auto& queue = m_PriorityActiveQueues[prio];
            for (auto& eventPtr : queue) {
                if (eventPtr) {
                    Publish(*eventPtr);
                }
            }
            queue.clear();
        }
    }

    inline void EventBus::Unsubscribe(HandlerId id) {
        std::type_index typeIndex(typeid(void));
        {
            std::shared_lock lock(m_Mutex);
            auto lookupIt = m_HandlerLookup.find(id);
            if (lookupIt == m_HandlerLookup.end()) {
                return;
            }
            typeIndex = lookupIt->second;
        }

        Unsubscribe(id, typeIndex);
    }

    inline void EventBus::UnsubscribeGroup(std::uint32_t groupId) {
        std::size_t removedCount = 0;

        {
            std::unique_lock lock(m_Mutex);
            for (auto it = m_Handlers.begin(); it != m_Handlers.end(); ) {
                auto& handlers = it->second;
                const auto newEnd = std::remove_if(handlers.begin(), handlers.end(), [&](const HandlerRecord& record) {
                    if (record.GroupId == groupId) {
                        m_HandlerLookup.erase(record.Id);
                        ++removedCount;
                        return true;
                    }
                    return false;
                });

                if (newEnd != handlers.end()) {
                    handlers.erase(newEnd, handlers.end());
                }

                if (handlers.empty()) {
                    it = m_Handlers.erase(it);
                } else {
                    ++it;
                }
            }
        }

        if (removedCount > 0 && ShouldTrace()) {
            SAGE_TRACE("EventBus: unsubscribed {0} handlers from group {1}", removedCount, groupId);
        }
    }

    inline void EventBus::Unsubscribe(HandlerId id, const std::type_index& typeIndex) {
        bool removed = false;
        std::uint32_t removedGroup = 0;

        {
            std::unique_lock lock(m_Mutex);
            auto handlersIt = m_Handlers.find(typeIndex);
            if (handlersIt == m_Handlers.end()) {
                m_HandlerLookup.erase(id);
                return;
            }

            auto& handlers = handlersIt->second;
            const auto beforeSize = handlers.size();
            handlers.erase(std::remove_if(handlers.begin(), handlers.end(), [&](const HandlerRecord& record) {
                if (record.Id == id) {
                    removedGroup = record.GroupId;
                    return true;
                }
                return false;
            }), handlers.end());

            removed = handlers.size() != beforeSize;

            if (removed) {
                m_HandlerLookup.erase(id);
            }

            if (handlers.empty()) {
                m_Handlers.erase(handlersIt);
            }
        }

        if (removed && ShouldTrace()) {
            SAGE_TRACE("EventBus: handler {0} unsubscribed from type {1} (group {2})", id, typeIndex.name(), removedGroup);
        }
    }

    inline void EventBus::PublishInternal(Event& event, const std::type_index& typeIndex) {
        if (!ShouldDeliver(event)) {
            if (ShouldTrace()) {
                SAGE_TRACE("EventBus: skipping event {0} due to category filter", event.GetName());
            }
            RecordDispatch(0, event);
            return;
        }

        HandlerList handlersCopy;
        {
            std::shared_lock lock(m_Mutex);
            auto it = m_Handlers.find(typeIndex);
            if (it == m_Handlers.end()) {
                RecordDispatch(0, event);
                return;
            }
            handlersCopy = it->second;
        }

        if (handlersCopy.empty()) {
            RecordDispatch(0, event);
            return;
        }

        std::size_t invokedCount = 0;
        for (auto& handler : handlersCopy) {
            // Check if weak owner is still alive (if one exists)
            if (auto owner = handler.WeakOwner.lock()) {
                // Owner is alive or no weak owner set, proceed
            } else if (!handler.WeakOwner.owner_before(std::weak_ptr<void>{}) && !std::weak_ptr<void>{}.owner_before(handler.WeakOwner)) {
                // No weak owner was set (default-constructed weak_ptr), proceed
            } else {
                // Weak owner was set but expired
                if (ShouldTrace()) {
                    SAGE_TRACE("EventBus: skipping handler {0} (owner expired)", handler.Id);
                }
                continue;
            }

            if (ShouldTrace()) {
                SAGE_TRACE("EventBus: dispatching {0} to handler {1}", event.GetName(), handler.Id);
            }

            ++invokedCount;

            try {
                handler.Invoker(event);
            } catch (const std::exception& ex) {
                SAGE_ERROR("EventBus: handler {0} threw exception: {1}", handler.Id, ex.what());
            } catch (...) {
                SAGE_ERROR("EventBus: handler {0} threw unknown exception", handler.Id);
            }

            if (event.Handled) {
                if (ShouldTrace()) {
                    SAGE_TRACE("EventBus: event {0} handled by handler {1}", event.GetName(), handler.Id);
                }
                break;
            }
        }

        RecordDispatch(invokedCount, event);
    }

    inline void EventBus::RecordDispatch(std::size_t handlerCount, const Event& event) {
        m_TotalPublished.fetch_add(1, std::memory_order_relaxed);
        m_TotalHandlersInvoked.fetch_add(handlerCount, std::memory_order_relaxed);

        if (ShouldTrace() && handlerCount == 0) {
            SAGE_TRACE("EventBus: event {0} had no handlers", event.GetName());
        }
    }

    inline EventBus::DispatchStatistics EventBus::GetStatistics() const {
        return DispatchStatistics{
            m_TotalPublished.load(std::memory_order_relaxed),
            m_TotalHandlersInvoked.load(std::memory_order_relaxed)
        };
    }

    inline void EventBus::ResetStatistics() {
        m_TotalPublished.store(0, std::memory_order_relaxed);
        m_TotalHandlersInvoked.store(0, std::memory_order_relaxed);
    }

    inline void EventBus::EnableTracing(bool enable) {
        m_TracingEnabled.store(enable, std::memory_order_relaxed);
    }

    inline bool EventBus::IsTracingEnabled() const {
        return m_TracingEnabled.load(std::memory_order_relaxed);
    }

    inline void EventBus::SetEnabledCategories(std::uint32_t mask) {
        m_EnabledCategories.store(mask, std::memory_order_relaxed);
    }

    inline void EventBus::EnableCategories(std::uint32_t mask) {
        m_EnabledCategories.fetch_or(mask, std::memory_order_relaxed);
    }

    inline void EventBus::DisableCategories(std::uint32_t mask) {
        m_EnabledCategories.fetch_and(~mask, std::memory_order_relaxed);
    }

    inline std::uint32_t EventBus::GetEnabledCategories() const {
        return m_EnabledCategories.load(std::memory_order_relaxed);
    }

    inline bool EventBus::ShouldTrace() const {
        return m_TracingEnabled.load(std::memory_order_relaxed);
    }

    inline bool EventBus::ShouldDeliver(const Event& event) const {
        const auto mask = m_EnabledCategories.load(std::memory_order_relaxed);
        const auto categoryFlags = event.GetCategoryFlags();
        if (categoryFlags == 0) {
            return true;
        }
        return (categoryFlags & mask) != 0;
    }

} // namespace SAGE
