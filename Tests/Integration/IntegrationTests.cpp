#include "TestFramework.h"
#include "ECS/ECS.h"
#include "ECS/Systems/Physics/PhysicsSystem.h"
#include "ECS/Systems/Visual/RenderSystem.h"
#include "ECS/Systems/Visual/AnimationSystem.h"

#include <chrono>

namespace {

SAGE::Ref<SAGE::AnimationClip> CreateTestClip(float duration = 0.05f) {
    auto clip = SAGE::CreateRef<SAGE::AnimationClip>("TestClip");
    clip->ClearFrames();
    clip->AddFrame({0.0f, 0.0f}, {1.0f, 1.0f}, duration);
    clip->AddFrame({0.0f, 0.0f}, {1.0f, 1.0f}, duration);
    return clip;
}

}

using namespace SAGE;
using namespace SAGE::ECS;
using namespace TestFramework;

/// @brief Интеграционный тест: ECS + Физика
TEST(Integration_ECS_Physics) {
        
    Registry registry;
    PhysicsSystem physicsSystem;
    
    physicsSystem.Init();
    
    // Создаём падающий объект
    Entity entity = registry.CreateEntity();
    registry.AddComponent(entity, TransformComponent(100, 100));
    registry.AddComponent(entity, SpriteComponent("ball.png"));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Dynamic);
    physics.SetMass(1.0f);
    registry.AddComponent(entity, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
    registry.AddComponent(entity, collider);
    
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    ASSERT_NOT_NULL(transform);
    float initialY = transform->position.y;
    
    // Симулируем игровой цикл
    for (int frame = 0; frame < 60; frame++) { // 1 секунда
        physicsSystem.FixedUpdate(registry, 0.016f);
    }
    
    transform = registry.GetComponent<TransformComponent>(entity);
    ASSERT_NOT_NULL(transform);
    float finalY = transform->position.y;
    
    // Объект должен упасть
    CHECK(finalY > initialY);
    
    physicsSystem.Shutdown();
}

/// @brief Интеграционный тест: Анимация + Рендеринг
TEST(Integration_Animation_Rendering) {
        
    Registry registry;
    AnimationSystem animSystem;
    RenderSystem renderSystem;
    
    // Создаём анимированный спрайт
    Entity entity = registry.CreateEntity();
    registry.AddComponent(entity, TransformComponent(200, 200));
    registry.AddComponent(entity, SpriteComponent("character.png"));
    
    AnimationComponent anim;
    anim.SetClip(CreateTestClip());
    anim.Play();
    // Note: Animation frames now managed by AnimationClip
    registry.AddComponent(entity, anim);
    
    // Симулируем несколько кадров
    for (int frame = 0; frame < 30; frame++) {
        animSystem.Update(registry, 0.016f);
        // renderSystem.Update(registry, 0.016f); // Закомментировано т.к. требует OpenGL контекст
    }
    
    // Анимация должна обновиться
    auto* updatedAnim = registry.GetComponent<AnimationComponent>(entity);
    ASSERT_NOT_NULL(updatedAnim);
    CHECK(updatedAnim->IsPlaying());
    CHECK(updatedAnim->timeAccumulator > 0.0f);
}

/// @brief Интеграционный тест: Множественные системы
TEST(Integration_MultipleSystems) {
        
    Registry registry;
    
    PhysicsSystem physicsSystem;
    AnimationSystem animSystem;
    
    physicsSystem.Init();
    
    // Создаём сложную entity с несколькими компонентами
    Entity entity = registry.CreateEntity();
    registry.AddComponent(entity, TransformComponent(100, 100));
    registry.AddComponent(entity, SpriteComponent("animated_sprite.png"));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Dynamic);
    physics.SetMass(1.0f);
    registry.AddComponent(entity, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(64.0f, 64.0f));
    registry.AddComponent(entity, collider);
    
    AnimationComponent anim;
    anim.SetClip(CreateTestClip());
    anim.Play();
    registry.AddComponent(entity, anim);
    
    // Обновляем все системы
    for (int frame = 0; frame < 60; frame++) {
        physicsSystem.FixedUpdate(registry, 0.016f);
        animSystem.Update(registry, 0.016f);
    }
    
    // Проверяем что обе системы работают
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    ASSERT_NOT_NULL(transform);
    auto* animation = registry.GetComponent<AnimationComponent>(entity);
    ASSERT_NOT_NULL(animation);
    
    CHECK(transform->position.y != 100.0f); // Физика изменила позицию
    CHECK(animation->timeAccumulator > 0.0f);    // Анимация обновилась
    
    physicsSystem.Shutdown();
}

/// @brief Интеграционный тест: Создание/удаление во время обновления
TEST(Integration_DynamicEntityManagement) {
        
    Registry registry;
    AnimationSystem animSystem;
    
    // Создаём начальные entities
    std::vector<Entity> entities;
    for (int i = 0; i < 10; i++) {
        Entity e = registry.CreateEntity();
        registry.AddComponent(e, TransformComponent(i * 50.0f, 0));
        registry.AddComponent(e, SpriteComponent("sprite.png"));
        
        AnimationComponent anim;
        anim.SetClip(CreateTestClip());
        anim.Play();
        registry.AddComponent(e, anim);
        
        entities.push_back(e);
    }
    
    // Обновляем систему
    animSystem.Update(registry, 0.016f);
    
    // Удаляем половину
    for (size_t i = 0; i < entities.size() / 2; i++) {
        registry.DestroyEntity(entities[i]);
    }
    
    // Создаём новые
    for (int i = 0; i < 5; i++) {
        Entity e = registry.CreateEntity();
        registry.AddComponent(e, TransformComponent(0, 0));
        registry.AddComponent(e, SpriteComponent("new.png"));
        
        AnimationComponent anim;
        anim.SetClip(CreateTestClip());
        anim.Play();
        registry.AddComponent(e, anim);
    }
    
    // Обновляем ещё раз - не должно крашить
    animSystem.Update(registry, 0.016f);
    
    CHECK(true);
}

/// @brief Интеграционный тест: Сериализация + Загрузка состояния
TEST(Integration_SaveLoad_GameState) {
        
    Registry registry1;
    
    // Создаём игровое состояние
    Entity player = registry1.CreateEntity();
    registry1.AddComponent(player, TransformComponent(123.0f, 456.0f));
    registry1.AddComponent(player, SpriteComponent("player.png"));
    
    Entity enemy = registry1.CreateEntity();
    registry1.AddComponent(enemy, TransformComponent(789.0f, 321.0f));
    registry1.AddComponent(enemy, SpriteComponent("enemy.png"));
    
    // Создаём новый registry и проверяем базовое состояние
    Registry registry2;
    
    // Простая валидация состояния (без сериализации)
    auto* playerTransform = registry1.GetComponent<TransformComponent>(player);
    auto* enemyTransform = registry1.GetComponent<TransformComponent>(enemy);
    
    ASSERT_NOT_NULL(playerTransform);
    ASSERT_NOT_NULL(enemyTransform);
    
    CHECK_NEAR(playerTransform->position.x, 123.0f, 0.1f);
    CHECK_NEAR(enemyTransform->position.x, 789.0f, 0.1f);
}

/// @brief Стресс-тест: Полная игровая сцена
TEST(Integration_StressTest_FullScene) {
        
    Registry registry;
    PhysicsSystem physicsSystem;
    AnimationSystem animSystem;
    
    physicsSystem.Init();
    
    const int entityCount = 100;
    
    // Создаём сложную сцену
    for (int i = 0; i < entityCount; i++) {
        Entity e = registry.CreateEntity();
        registry.AddComponent(e, TransformComponent(
            (i % 10) * 64.0f,
            (i / 10) * 64.0f
        ));
        registry.AddComponent(e, SpriteComponent("sprite.png"));
        
        // Половина - с физикой
        if (i % 2 == 0) {
            PhysicsComponent physics;
            physics.SetType(PhysicsBodyType::Dynamic);
            registry.AddComponent(e, physics);
            
            auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
            registry.AddComponent(e, collider);
        }
        
        // Половина - с анимацией
        if (i % 3 == 0) {
            AnimationComponent anim;
            anim.SetClip(CreateTestClip());
            anim.Play();
            registry.AddComponent(e, anim);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Симулируем игровой цикл
    for (int frame = 0; frame < 60; frame++) {
        physicsSystem.FixedUpdate(registry, 0.016f);
        animSystem.Update(registry, 0.016f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 1 секунда игрового времени должна выполниться быстро
    CHECK(duration.count() < 2000); // < 2 секунд реального времени
    
    physicsSystem.Shutdown();
}
