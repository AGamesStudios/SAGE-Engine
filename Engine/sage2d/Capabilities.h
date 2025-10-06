#pragma once

#include "Types.h"
#include "ResourceId.h"

#include <cstdint>
#include <string>

namespace sage2d {

    struct Object;

    struct Physics {
        float mass{ 1.0f };
        Vec2 velocity{};
        Vec2 acceleration{};
        bool kinematic{ false };
        float gravityScale{ 1.0f };
        float drag{ 0.0f };
    };

    struct Collider {
        float x{ 0.0f };
        float y{ 0.0f };
        float w{ 0.0f };
        float h{ 0.0f };
        bool trigger{ false };
        std::uint32_t layer{ 0 };
        std::uint32_t mask{ 0xFFFFFFFFu };
    };

    struct Sprite {
        ResId image{ kInvalidResId };
        ResId animation{ kInvalidResId };
        Vec2 size{ 32.0f, 32.0f };
        float layer{ 0.0f };
        float alpha{ 1.0f };
        bool flipX{ false };
        bool flipY{ false };
    };

    struct Controls {
        int left{ 'A' };
        int right{ 'D' };
        int up{ 'W' };
        int down{ 'S' };
        int action{ 'E' };
        int jump{ ' ' };
    };

    struct Script {
        using UpdateFn = void (*)(Object&, float);
        UpdateFn update{ nullptr };
        UpdateFn preUpdate{ nullptr };
        UpdateFn postUpdate{ nullptr };
        std::string binding; // Optional name for scripted binding (Lua, etc.)
    };

} // namespace sage2d
