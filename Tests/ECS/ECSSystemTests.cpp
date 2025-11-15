#include "TestFramework.h"
#include "ECS/ECS.h"
#include "ECS/Systems/Physics/PhysicsSystem.h"
#include "ECS/Systems/Visual/RenderSystem.h"
#include "ECS/Systems/Visual/AnimationSystem.h"
#include "Graphics/Core/Animation/AnimationClip.h"
#include "Memory/Ref.h"

#include <memory>

using namespace SAGE;
using namespace SAGE::ECS;
using namespace TestFramework;

TEST(ECS_SystemDefaults) {
    auto renderSystem = std::make_unique<RenderSystem>();
    CHECK(renderSystem != nullptr);
    CHECK(renderSystem->IsActive());
    CHECK_EQ(renderSystem->GetPriority(), 1000);

    AnimationSystem animationSystem;
    CHECK(animationSystem.IsActive());
    CHECK_EQ(animationSystem.GetPriority(), 50);

    PhysicsSystem physicsSystem;
    CHECK(physicsSystem.IsActive());
    CHECK_EQ(physicsSystem.GetPriority(), 20);
}

TEST(ECS_SystemPriorityOrder) {
    PhysicsSystem physicsSystem;
    AnimationSystem animationSystem;
    RenderSystem renderSystem;

    CHECK(physicsSystem.GetPriority() < animationSystem.GetPriority());
    CHECK(animationSystem.GetPriority() < renderSystem.GetPriority());
}

TEST(ECS_AnimationSystemAdvancesClip) {
    Registry registry;
    AnimationSystem animationSystem;

    Entity entity = registry.CreateEntity();

    auto clip = CreateRef<AnimationClip>("Walk");
    clip->AddFrame(SAGE::AnimationFrame(Float2(0.0f, 0.0f), Float2(0.5f, 0.5f), 0.1f));
    clip->AddFrame(SAGE::AnimationFrame(Float2(0.5f, 0.0f), Float2(1.0f, 0.5f), 0.1f));
    clip->SetPlayMode(AnimationPlayMode::Loop);

    AnimationComponent animationComponent;
    animationComponent.SetClip(clip);
    animationComponent.Play();
    registry.AddComponent(entity, animationComponent);

    SpriteComponent sprite("sprite.png");
    registry.AddComponent(entity, sprite);

    animationSystem.Update(registry, 0.2f);

    auto* updatedAnimation = registry.GetComponent<AnimationComponent>(entity);
    CHECK(updatedAnimation != nullptr);
    CHECK(updatedAnimation->IsPlaying());

    const SAGE::AnimationFrame* frameData = updatedAnimation->GetCurrentFrameData();
    CHECK(frameData != nullptr);

    auto* updatedSprite = registry.GetComponent<SpriteComponent>(entity);
    CHECK(updatedSprite != nullptr);
    CHECK_NEAR(updatedSprite->uvMin.x, frameData->uvMin.x, 1e-4f);
    CHECK_NEAR(updatedSprite->uvMax.y, frameData->uvMax.y, 1e-4f);
}

TEST(ECS_SystemActiveFlag) {
    AnimationSystem system;
    CHECK(system.IsActive());

    system.SetActive(false);
    CHECK(!system.IsActive());

    system.SetActive(true);
    CHECK(system.IsActive());
}

TEST(ECS_PhysicsSystemCreatesBody) {
    Registry registry;
    PhysicsSystem physicsSystem;
    physicsSystem.Init();

    Entity entity = registry.CreateEntity();
    registry.AddComponent(entity, TransformComponent{});

    PhysicsComponent physicsComponent;
    physicsComponent.SetType(PhysicsBodyType::Dynamic);
    physicsComponent.SetMass(1.0f);
    registry.AddComponent(entity, physicsComponent);

    auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
    registry.AddComponent(entity, collider);

    physicsSystem.FixedUpdate(registry, 1.0f / 60.0f);

    auto* storedPhysics = registry.GetComponent<PhysicsComponent>(entity);
    CHECK(storedPhysics != nullptr);
    CHECK(storedPhysics->bodyCreated);

    physicsSystem.Shutdown();
}
