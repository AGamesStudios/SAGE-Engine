#include "TestFramework.h"

#include "ECS/ECS.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "Physics/PhysicsWorld.h"

using namespace SAGE;
using namespace SAGE::ECS;
using namespace SAGE::Physics;

namespace {

struct PhysicsTestFixture {
    Registry registry;

    Entity CreateStaticBox(const Vector2& position, const Vector2& size) {
        Entity entity = registry.CreateEntity();
        registry.AddComponent(entity, TransformComponent(position.x, position.y));
        registry.AddComponent(entity, BoxColliderComponent(size));
        RigidBodyComponent body;
        body.SetStatic(true);
        registry.AddComponent(entity, body);
        return entity;
    }

    Entity CreateDynamicBox(const Vector2& position, const Vector2& size) {
        Entity entity = registry.CreateEntity();
        registry.AddComponent(entity, TransformComponent(position.x, position.y));
        registry.AddComponent(entity, BoxColliderComponent(size));
        RigidBodyComponent body;
        body.SetStatic(false);
        body.SetMass(1.0f);
        registry.AddComponent(entity, body);
        return entity;
    }
};

bool BoxesOverlap(const BoxColliderComponent& a, const TransformComponent& ta,
                  const BoxColliderComponent& b, const TransformComponent& tb) {
    const Vector2 minA = a.GetMin(ta);
    const Vector2 maxA = a.GetMax(ta);
    const Vector2 minB = b.GetMin(tb);
    const Vector2 maxB = b.GetMax(tb);

    const bool separated = maxA.x <= minB.x || maxB.x <= minA.x ||
                           maxA.y <= minB.y || maxB.y <= minA.y;
    return !separated;
}

} // namespace

TEST_CASE(PhysicsWorld_GeneratesResolvedContact) {
    PhysicsTestFixture fixture;
    PhysicsWorld world;
    world.SetGravity(Vector2::Zero());

    Entity ground = fixture.CreateStaticBox(Vector2(0.0f, 0.0f), Vector2(200.0f, 32.0f));
    Entity box = fixture.CreateDynamicBox(Vector2(80.0f, -24.0f), Vector2(32.0f, 32.0f));

    world.Step(fixture.registry, 0.016f);

    auto* groundTransform = fixture.registry.GetComponent<TransformComponent>(ground);
    auto* groundCollider = fixture.registry.GetComponent<BoxColliderComponent>(ground);
    auto* boxTransform = fixture.registry.GetComponent<TransformComponent>(box);
    auto* boxCollider = fixture.registry.GetComponent<BoxColliderComponent>(box);

    ASSERT_NOT_NULL(groundTransform);
    ASSERT_NOT_NULL(groundCollider);
    ASSERT_NOT_NULL(boxTransform);
    ASSERT_NOT_NULL(boxCollider);

    ASSERT_FALSE(BoxesOverlap(*groundCollider, *groundTransform, *boxCollider, *boxTransform),
                 "Expected box to be separated after resolution");

    const auto& contacts = world.GetContacts();
    ASSERT_FALSE(contacts.empty(), "Expected at least one contact");

    const Contact& contact = contacts.front();
    ASSERT_FALSE(contact.isTrigger);
    ASSERT_TRUE(contact.resolved);
    ASSERT_TRUE((contact.entityA == ground && contact.entityB == box) ||
                (contact.entityA == box && contact.entityB == ground));
}

TEST_CASE(PhysicsWorld_TriggersReportedWithoutResolution) {
    PhysicsTestFixture fixture;
    PhysicsWorld world;
    world.SetGravity(Vector2::Zero());

    Entity trigger = fixture.CreateStaticBox(Vector2(0.0f, 0.0f), Vector2(100.0f, 100.0f));
    auto* triggerCollider = fixture.registry.GetComponent<BoxColliderComponent>(trigger);
    ASSERT_NOT_NULL(triggerCollider);
    triggerCollider->isTrigger = true;

    [[maybe_unused]] Entity dynamic = fixture.CreateDynamicBox(Vector2(25.0f, 25.0f), Vector2(50.0f, 50.0f));

    world.Step(fixture.registry, 0.016f);

    const auto& contacts = world.GetContacts();
    ASSERT_EQ(static_cast<size_t>(1), contacts.size());
    const Contact& contact = contacts.front();
    ASSERT_TRUE(contact.isTrigger);
    ASSERT_FALSE(contact.resolved);
}

TEST_CASE(PhysicsSystem_DispatchesContactCallback) {
    PhysicsTestFixture fixture;
    PhysicsSystem system;
    system.GetWorld().SetGravity(Vector2::Zero());

    [[maybe_unused]] Entity a = fixture.CreateStaticBox(Vector2(0.0f, 0.0f), Vector2(128.0f, 32.0f));
    [[maybe_unused]] Entity b = fixture.CreateDynamicBox(Vector2(64.0f, -20.0f), Vector2(32.0f, 32.0f));

    int callbackCount = 0;
    system.SetContactCallback([&](const Contact& contact) {
        ++callbackCount;
        ASSERT_TRUE(contact.resolved || contact.isTrigger);
    });

    system.Update(fixture.registry, 0.016f);

    ASSERT_EQ(1, callbackCount);
}
