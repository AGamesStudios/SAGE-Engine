#pragma once

#include "SAGE/Physics/PhysicsCommon.h"

#include <functional>

namespace SAGE::Physics {

class PhysicsWorld {
public:
    using ContactCallback = std::function<void(const ContactEvent&)>;

    explicit PhysicsWorld(const PhysicsSettings& settings = {});
    ~PhysicsWorld();

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    PhysicsWorld(PhysicsWorld&&) = delete;
    PhysicsWorld& operator=(PhysicsWorld&&) = delete;

    void Step(float deltaTime);

    BodyHandle CreateBody(const b2BodyDef& def);
    void DestroyBody(BodyHandle handle);

    void ApplyForce(BodyHandle handle, const Vector2& force, const Vector2& point, bool wake);
    void ApplyForceCenter(BodyHandle handle, const Vector2& force, bool wake);
    void ApplyLinearImpulse(BodyHandle handle, const Vector2& impulse, const Vector2& point, bool wake);
    void ApplyLinearImpulseCenter(BodyHandle handle, const Vector2& impulse, bool wake);
    void SetLinearVelocity(BodyHandle handle, const Vector2& velocity);
    Vector2 GetLinearVelocity(BodyHandle handle) const;
    void SetAngularVelocity(BodyHandle handle, float velocity);
    float GetAngularVelocity(BodyHandle handle) const;

    struct RayCastHit {
        BodyHandle body;
        Vector2 point;
        Vector2 normal;
        float fraction;
        bool hit = false;
    };

    RayCastHit RayCast(const Vector2& start, const Vector2& end);
    
    // Query
    std::vector<BodyHandle> QueryPoint(const Vector2& point);
    BodyHandle QueryPointFirst(const Vector2& point);

    void SetSettings(const PhysicsSettings& settings);
    const PhysicsSettings& GetSettings() const { return m_Settings; }

    b2WorldId GetNativeWorld() const { return m_WorldId; }

    void SetBeginContactCallback(ContactCallback cb);
    void SetEndContactCallback(ContactCallback cb);

private:
    struct WorldContactListener {
        b2WorldId worldId = b2_nullWorldId;
        ContactCallback begin;
        ContactCallback end;

        void DispatchEvents() const;
    };

    PhysicsSettings m_Settings{};
    b2WorldId m_WorldId = b2_nullWorldId;
    WorldContactListener m_ContactListener{};
    float m_Accumulator = 0.0f;

    void DestroyWorld();
    void ApplySettings();
};

} // namespace SAGE::Physics
