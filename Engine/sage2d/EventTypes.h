#pragma once

#include "Types.h"

#include <cstdint>
#include <type_traits>

namespace sage2d {

    enum class EventType : std::uint8_t {
        Start = 0,
        Tick,
        Hit,
        Enter,
        Exit,
        Use,
        Timer,
        Count
    };

    struct EventPayload {
        std::uint32_t sender;
        std::uint32_t target;
        std::uint32_t other;
        float value;
        float aux;
        std::uint32_t data;
    };

    struct Event {
        EventType type;
        EventPayload payload;
    };

    static_assert(std::is_standard_layout_v<EventPayload> && std::is_trivial_v<EventPayload>, "EventPayload must remain POD");
    static_assert(std::is_standard_layout_v<Event> && std::is_trivial_v<Event>, "Event must remain POD");

} // namespace sage2d
