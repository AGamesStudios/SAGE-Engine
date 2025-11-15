#pragma once

#include "IPhysicsBackend.h"
#include <box2d/box2d.h>
#include <unordered_map>

namespace SAGE {
namespace ECS {
    class Registry;
    struct PhysicsComponent;
}
}

namespace SAGE::Physics {

/// \brief Box2D v3.x backend implementation
class Box2DBackend : public IPhysicsBackend {
public:
    Box2DBackend();
    ~Box2DBackend() override;

    // IPhysicsBackend interface
    void Initialize(const PhysicsSettings& settings) override;
    void Step(ECS::Registry& registry, float deltaTime) override;
    void SyncTransforms(ECS::Registry& registry) override;
    bool CreateBody(ECS::Entity entity, ECS::Registry& registry) override;
    void DestroyBody(ECS::Entity entity) override;
    bool Raycast(const Vector2& origin, const Vector2& direction, 
                float maxDistance, RaycastHit& hit) override;
    void QueryAABB(const Vector2& min, const Vector2& max, 
                   std::vector<ECS::Entity>& entities) override;
    void SetGravity(const Vector2& gravity) override;
    Vector2 GetGravity() const override;
    void Clear() override;
    void SetContactCallback(ContactCallback callback) override;
    void SetDebugDraw(bool enabled) override;
    const std::vector<Contact>& GetContacts() const override;
    const char* GetName() const override { return "Box2D v3.x"; }
    
    /// \brief Draw debug visualization (call after rendering)
    void DrawDebug();

private:
    // Helper: Contact callback for Box2D
    static bool PreSolveCallback(b2ShapeId shapeIdA, b2ShapeId shapeIdB, 
                                 b2Vec2 point, b2Vec2 normal, void* context);
    
    // Helper: AABB query callback
    struct QueryContext {
        std::vector<ECS::Entity>* entities;
        std::unordered_map<uint32_t, ECS::Entity>* bodyMap;
    };
    static bool QueryAABBCallback(b2ShapeId shapeId, void* context);
    
    // Helper: Convert meters to pixels (Box2D works in meters, we work in pixels)
    static constexpr float PIXELS_PER_METER = 100.0f;
    static float ToMeters(float pixels) { return pixels / PIXELS_PER_METER; }
    static float ToPixels(float meters) { return meters * PIXELS_PER_METER; }
    static b2Vec2 ToB2Vec2(const Vector2& v) { return {ToMeters(v.x), ToMeters(v.y)}; }
    static Vector2 FromB2Vec2(const b2Vec2& v) { return {ToPixels(v.x), ToPixels(v.y)}; }

    // World
    b2WorldId m_worldId;
    PhysicsSettings m_settings;
    
    // Entity -> Body mapping
    std::unordered_map<ECS::Entity, b2BodyId> m_entityToBody;
    std::unordered_map<uint32_t, ECS::Entity> m_bodyIndexToEntity; // b2BodyId.index1 -> Entity
    
    // Contacts
    std::vector<Contact> m_contacts;
    ContactCallback m_contactCallback;
    
    // Debug Draw
    bool m_debugDraw = false;
    b2DebugDraw m_b2DebugDraw;
    
    // Debug draw callbacks
    static void DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context);
    static void DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, 
                                 float radius, b2HexColor color, void* context);
    static void DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context);
    static void DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context);
    static void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context);
    static void DrawTransform(b2Transform transform, void* context);
    static void DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context);
    static void DrawString(b2Vec2 p, const char* s, b2HexColor color, void* context);
};

} // namespace SAGE::Physics
