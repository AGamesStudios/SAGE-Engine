#pragma once

#include "SAGE/Math/Vector2.h"

#include <box2d/box2d.h>

#include <cstdint>

namespace SAGE::Physics {

struct PhysicsMaterial {
    float density = 1.0f;
    float friction = 0.3f;
    float restitution = 0.0f;
};

struct PhysicsSettings {
    Vector2 gravity{0.0f, 980.0f};
    float fixedTimeStep = 1.0f / 60.0f;
    int subSteps = 4;
};

struct BodyHandle {
    uint64_t value = 0;
    bool IsValid() const { return value != 0; }
};

struct ContactEvent {
    bool isBegin = true;
    b2ShapeId shapeA = b2_nullShapeId;
    b2ShapeId shapeB = b2_nullShapeId;
    uintptr_t userDataA = 0;
    uintptr_t userDataB = 0;
};

inline BodyHandle ToBodyHandle(b2BodyId id) {
    if (!b2Body_IsValid(id)) {
        return {};
    }

    BodyHandle handle{};
    handle.value = static_cast<uint64_t>(id.index1)
                 | (static_cast<uint64_t>(id.world0) << 32)
                 | (static_cast<uint64_t>(id.generation) << 48);

    return handle;
}

inline b2BodyId ToB2BodyId(BodyHandle handle) {
    b2BodyId id = b2_nullBodyId;
    if (!handle.IsValid()) {
        return id;
    }

    id.index1 = static_cast<int32_t>(handle.value & 0xFFFFFFFFull);
    id.world0 = static_cast<int16_t>((handle.value >> 32) & 0xFFFFull);
    id.generation = static_cast<int16_t>((handle.value >> 48) & 0xFFFFull);
    return id;
}

inline b2Vec2 ToB2Vec2(const Vector2& v) {
    return {v.x, v.y};
}

inline Vector2 ToVector2(const b2Vec2& v) {
    return {v.x, v.y};
}

} // namespace SAGE::Physics
