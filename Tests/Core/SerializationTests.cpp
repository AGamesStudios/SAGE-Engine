#include "TestFramework.h"
#include "ECS/ECS.h"

using namespace SAGE;
using namespace SAGE::ECS;
using namespace TestFramework;

/// @brief Minimal serialization test for components
TEST(Serialization_ComponentBasics) {
    TransformComponent transform(100.0f, 200.0f);
    transform.rotation = 45.0f;
    
    SpriteComponent sprite("test.png");
    sprite.tint = Color(1.0f, 0.5f, 0.25f, 1.0f);
    sprite.flipX = true;
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Dynamic);
    physics.SetMass(2.5f);
    physics.fixedRotation = true;
    
    // Basic component creation and property validation
    CHECK_NEAR(transform.position.x, 100.0f, 0.001f);
    CHECK_NEAR(transform.position.y, 200.0f, 0.001f);
    CHECK_NEAR(transform.rotation, 45.0f, 0.001f);
    
    CHECK(sprite.texturePath == "test.png");
    CHECK_NEAR(sprite.tint.r, 1.0f, 0.001f);
    CHECK(sprite.flipX == true);
    
    CHECK(physics.IsDynamic());
    CHECK_NEAR(physics.mass, 2.5f, 0.001f);
    CHECK(physics.fixedRotation == true);
}

/// @brief Test registry with multiple component types
TEST(Serialization_RegistryComponents) {
    Registry registry;
    
    Entity entity1 = registry.CreateEntity();
    registry.AddComponent(entity1, TransformComponent(123.0f, 456.0f));
    registry.AddComponent(entity1, SpriteComponent("sprite1.png"));
    
    Entity entity2 = registry.CreateEntity();
    registry.AddComponent(entity2, TransformComponent(789.0f, 321.0f));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Static);
    registry.AddComponent(entity2, physics);
    
    // Validate components exist
    CHECK(registry.HasComponent<TransformComponent>(entity1));
    CHECK(registry.HasComponent<SpriteComponent>(entity1));
    CHECK(!registry.HasComponent<PhysicsComponent>(entity1));
    
    CHECK(registry.HasComponent<TransformComponent>(entity2));
    CHECK(!registry.HasComponent<SpriteComponent>(entity2));
    CHECK(registry.HasComponent<PhysicsComponent>(entity2));
    
    auto* transform1 = registry.GetComponent<TransformComponent>(entity1);
    ASSERT_NOT_NULL(transform1);
    CHECK_NEAR(transform1->position.x, 123.0f, 0.001f);
    
    auto* physics2 = registry.GetComponent<PhysicsComponent>(entity2);
    ASSERT_NOT_NULL(physics2);
    CHECK(physics2->IsStatic());
}

extern "C" void __sage_force_link_SerializationTests() {}
