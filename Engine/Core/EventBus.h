#pragma once

#include "Event.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SAGE {

    class EventBus {
    public:
        using HandlerId = std::uint64_t;

        EventBus();
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&) noexcept = default;
        EventBus& operator=(EventBus&&) noexcept = default;

        template<typename EventT, typename Callable>
        HandlerId Subscribe(Callable&& callback);

        template<typename EventT>
        void Unsubscribe(HandlerId id);

        template<typename EventT>
        void Publish(EventT& event);

        void Publish(Event& event);
        void Clear();

    private:
        struct HandlerRecord {
            HandlerId Id;
            std::function<void(Event&)> Invoker;
        };

        using HandlerList = std::vector<HandlerRecord>;
        std::unordered_map<std::type_index, HandlerList> m_Handlers;
        HandlerId m_NextId;
        mutable std::shared_mutex m_Mutex; // Для многопоточности
    };

    inline EventBus::EventBus()
        : m_NextId(1) {}

    template<typename EventT, typename Callable>
    EventBus::HandlerId EventBus::Subscribe(Callable&& callback) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        std::unique_lock lock(m_Mutex);
        
        const std::type_index typeIndex(typeid(EventT));
        auto& handlers = m_Handlers[typeIndex];

        HandlerRecord record;
        record.Id = m_NextId++;
        record.Invoker = [fn = std::forward<Callable>(callback)](Event& baseEvent) {
            fn(static_cast<EventT&>(baseEvent));
        };

        handlers.emplace_back(std::move(record));
        return handlers.back().Id;
    }

    template<typename EventT>
    void EventBus::Unsubscribe(HandlerId id) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        std::unique_lock lock(m_Mutex);
        
        const std::type_index typeIndex(typeid(EventT));
        auto it = m_Handlers.find(typeIndex);
        if (it == m_Handlers.end()) {
            return;
        }

        auto& handlers = it->second;
        handlers.erase(std::remove_if(handlers.begin(), handlers.end(), [id](const HandlerRecord& record) {
            return record.Id == id;
        }), handlers.end());

        if (handlers.empty()) {
            m_Handlers.erase(it);
        }
    }

    template<typename EventT>
    void EventBus::Publish(EventT& event) {
        static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
        std::shared_lock lock(m_Mutex); // Читаем, не блокируем других читателей
        
        const std::type_index typeIndex(typeid(EventT));
        auto it = m_Handlers.find(typeIndex);
        if (it == m_Handlers.end()) {
            return;
        }

        for (auto& handler : it->second) {
            handler.Invoker(event);
            if (event.Handled) {
                break;
            }
        }
    }

} // namespace SAGE
