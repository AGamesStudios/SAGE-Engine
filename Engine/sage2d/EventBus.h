#pragma once

#include "EventTypes.h"

#include <array>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace sage2d {

    class Stage;

    class EventBus {
    public:
        using ListenerId = std::uint32_t;
        using Callback = std::function<void(Stage&, const Event&)>;

        EventBus() = default;

        ListenerId subscribe(EventType type, Callback callback);
        bool unsubscribe(EventType type, ListenerId id);

        void queue(Event event);
        void queue(EventType type, const EventPayload& payload);

        void process(Stage& stage);
        void clear();

    private:
        struct Listener {
            ListenerId id{ 0 };
            Callback callback;
        };

        static constexpr std::size_t kEventTypeCount = static_cast<std::size_t>(EventType::Count);

        std::array<std::vector<Listener>, kEventTypeCount> m_Listeners{};
        std::vector<Event> m_Current{};
        std::vector<Event> m_Pending{};
        ListenerId m_NextId{ 1 };
    };

} // namespace sage2d
