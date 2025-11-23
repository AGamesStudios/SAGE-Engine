#include "SAGE/Physics/PhysicsWorld.h"

#include "SAGE/Log.h"

#include <algorithm>

namespace SAGE::Physics {

PhysicsWorld::PhysicsWorld(const PhysicsSettings& settings)
    : m_Settings(settings) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = ToB2Vec2(settings.gravity);
    m_WorldId = b2CreateWorld(&worldDef);
    m_ContactListener.worldId = m_WorldId;
}

PhysicsWorld::~PhysicsWorld() {
    DestroyWorld();
}

void PhysicsWorld::DestroyWorld() {
    if (b2World_IsValid(m_WorldId)) {
        b2DestroyWorld(m_WorldId);
        m_WorldId = b2_nullWorldId;
        m_ContactListener.worldId = b2_nullWorldId;
    }
}

void PhysicsWorld::Step(float deltaTime) {
    if (!b2World_IsValid(m_WorldId)) {
        return;
    }

    // Directly step the world without internal accumulator
    // The outer loop (Application/Game) handles the fixed timestep
    b2World_Step(m_WorldId, deltaTime, m_Settings.subSteps);
    m_ContactListener.DispatchEvents();
}

BodyHandle PhysicsWorld::CreateBody(const b2BodyDef& def) {
    if (!b2World_IsValid(m_WorldId)) {
        return {};
    }

    b2BodyId id = b2CreateBody(m_WorldId, &def);
    return ToBodyHandle(id);
}

void PhysicsWorld::DestroyBody(BodyHandle handle) {
    if (!handle.IsValid()) {
        return;
    }

    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2DestroyBody(id);
    }
}

void PhysicsWorld::ApplyForce(BodyHandle handle, const Vector2& force, const Vector2& point, bool wake) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_ApplyForce(id, ToB2Vec2(force), ToB2Vec2(point), wake);
    }
}

void PhysicsWorld::ApplyForceCenter(BodyHandle handle, const Vector2& force, bool wake) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_ApplyForceToCenter(id, ToB2Vec2(force), wake);
    }
}

void PhysicsWorld::ApplyLinearImpulse(BodyHandle handle, const Vector2& impulse, const Vector2& point, bool wake) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_ApplyLinearImpulse(id, ToB2Vec2(impulse), ToB2Vec2(point), wake);
    }
}

void PhysicsWorld::ApplyLinearImpulseCenter(BodyHandle handle, const Vector2& impulse, bool wake) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_ApplyLinearImpulseToCenter(id, ToB2Vec2(impulse), wake);
    }
}

void PhysicsWorld::SetLinearVelocity(BodyHandle handle, const Vector2& velocity) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_SetLinearVelocity(id, ToB2Vec2(velocity));
    }
}

Vector2 PhysicsWorld::GetLinearVelocity(BodyHandle handle) const {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        return ToVector2(b2Body_GetLinearVelocity(id));
    }
    return Vector2::Zero();
}

void PhysicsWorld::SetAngularVelocity(BodyHandle handle, float velocity) {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        b2Body_SetAngularVelocity(id, velocity);
    }
}

float PhysicsWorld::GetAngularVelocity(BodyHandle handle) const {
    b2BodyId id = ToB2BodyId(handle);
    if (b2Body_IsValid(id)) {
        return b2Body_GetAngularVelocity(id);
    }
    return 0.0f;
}

PhysicsWorld::RayCastHit PhysicsWorld::RayCast(const Vector2& start, const Vector2& end) {
    if (!b2World_IsValid(m_WorldId)) {
        return {};
    }

    b2Vec2 p1 = ToB2Vec2(start);
    b2Vec2 p2 = ToB2Vec2(end);
    
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2RayResult result = b2World_CastRayClosest(m_WorldId, p1, p2, filter);

    RayCastHit hit;
    if (result.hit) {
        hit.hit = true;
        hit.body = ToBodyHandle(b2Shape_GetBody(result.shapeId));
        hit.point = ToVector2(result.point);
        hit.normal = ToVector2(result.normal);
        hit.fraction = result.fraction;
    }

    return hit;
}

namespace {
    bool QueryCallback(b2ShapeId shapeId, void* context) {
        auto* bodies = static_cast<std::vector<BodyHandle>*>(context);
        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        bodies->push_back(ToBodyHandle(bodyId));
        return true; 
    }

    bool QueryFirstCallback(b2ShapeId shapeId, void* context) {
        auto* body = static_cast<BodyHandle*>(context);
        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        *body = ToBodyHandle(bodyId);
        return false; 
    }
}

std::vector<BodyHandle> PhysicsWorld::QueryPoint(const Vector2& point) {
    if (!b2World_IsValid(m_WorldId)) return {};

    std::vector<BodyHandle> bodies;
    b2QueryFilter filter = b2DefaultQueryFilter();
    
    b2AABB aabb;
    b2Vec2 p = ToB2Vec2(point);
    aabb.lowerBound = {p.x - 0.1f, p.y - 0.1f};
    aabb.upperBound = {p.x + 0.1f, p.y + 0.1f};
    
    b2World_OverlapAABB(m_WorldId, aabb, filter, QueryCallback, &bodies);
    return bodies;
}

BodyHandle PhysicsWorld::QueryPointFirst(const Vector2& point) {
    if (!b2World_IsValid(m_WorldId)) return {};

    BodyHandle body;
    b2QueryFilter filter = b2DefaultQueryFilter();
    
    b2AABB aabb;
    b2Vec2 p = ToB2Vec2(point);
    aabb.lowerBound = {p.x - 0.1f, p.y - 0.1f};
    aabb.upperBound = {p.x + 0.1f, p.y + 0.1f};

    b2World_OverlapAABB(m_WorldId, aabb, filter, QueryFirstCallback, &body);
    return body;
}

void PhysicsWorld::SetSettings(const PhysicsSettings& settings) {
    m_Settings = settings;
    ApplySettings();
}

void PhysicsWorld::ApplySettings() {
    if (!b2World_IsValid(m_WorldId)) {
        return;
    }
    b2World_SetGravity(m_WorldId, ToB2Vec2(m_Settings.gravity));
}

void PhysicsWorld::SetBeginContactCallback(ContactCallback cb) {
    m_ContactListener.begin = std::move(cb);
}

void PhysicsWorld::SetEndContactCallback(ContactCallback cb) {
    m_ContactListener.end = std::move(cb);
}

void PhysicsWorld::WorldContactListener::DispatchEvents() const {
    if (!b2World_IsValid(worldId)) {
        return;
    }

    b2ContactEvents events = b2World_GetContactEvents(worldId);

    if (begin) {
        for (int i = 0; i < events.beginCount; ++i) {
            b2ContactBeginTouchEvent evt = events.beginEvents[i];
            ContactEvent contact;
            contact.isBegin = true;
            contact.shapeA = evt.shapeIdA;
            contact.shapeB = evt.shapeIdB;

            b2BodyId bodyA = b2Shape_GetBody(evt.shapeIdA);
            b2BodyId bodyB = b2Shape_GetBody(evt.shapeIdB);
            contact.userDataA = reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA));
            contact.userDataB = reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB));

            begin(contact);
        }
    }

    if (end) {
        for (int i = 0; i < events.endCount; ++i) {
            b2ContactEndTouchEvent evt = events.endEvents[i];
            ContactEvent contact;
            contact.isBegin = false;
            contact.shapeA = evt.shapeIdA;
            contact.shapeB = evt.shapeIdB;

            b2BodyId bodyA = b2Shape_GetBody(evt.shapeIdA);
            b2BodyId bodyB = b2Shape_GetBody(evt.shapeIdB);
            contact.userDataA = reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA));
            contact.userDataB = reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB));

            end(contact);
        }
    }
}

} // namespace SAGE::Physics
