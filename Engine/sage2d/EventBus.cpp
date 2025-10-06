#include "EventBus.h"

#include "Stage.h"

#include <algorithm>

namespace sage2d {

EventBus::ListenerId EventBus::subscribe(EventType type, Callback callback) {
    if (!callback) {
        return 0;
    }

    const std::size_t index = static_cast<std::size_t>(type);
    Listener listener;
    listener.id = m_NextId++;
    listener.callback = std::move(callback);
    m_Listeners[index].push_back(std::move(listener));
    return m_Listeners[index].back().id;
}

bool EventBus::unsubscribe(EventType type, ListenerId id) {
    if (id == 0) {
        return false;
    }

    const std::size_t index = static_cast<std::size_t>(type);
    auto& listeners = m_Listeners[index];
    bool modified = false;
    for (auto& listener : listeners) {
        if (listener.id == id) {
            listener.callback = nullptr;
            modified = true;
            break;
        }
    }

    if (modified) {
        std::erase_if(listeners, [](const Listener& listener) {
            return !listener.callback;
        });
    }

    return modified;
}

void EventBus::queue(Event event) {
    m_Pending.push_back(event);
}

void EventBus::queue(EventType type, const EventPayload& payload) {
    Event event{};
    event.type = type;
    event.payload = payload;
    queue(event);
}

void EventBus::process(Stage& stage) {
    if (m_Pending.empty()) {
        return;
    }

    while (!m_Pending.empty()) {
        m_Current.clear();
        m_Current.swap(m_Pending);

        for (const Event& event : m_Current) {
            const std::size_t index = static_cast<std::size_t>(event.type);
            auto& listeners = m_Listeners[index];
            if (listeners.empty()) {
                continue;
            }

            for (const Listener& listener : listeners) {
                if (listener.callback) {
                    listener.callback(stage, event);
                }
            }
        }
    }
}

void EventBus::clear() {
    m_Pending.clear();
    m_Current.clear();
}

} // namespace sage2d
