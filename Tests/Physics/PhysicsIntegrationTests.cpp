#include "TestFramework.h"
#include "Physics/Box2DBackend.h"
#include "Physics/IPhysicsBackend.h"
#include "ECS/ECS.h"

#include <chrono>

using namespace SAGE;
using namespace SAGE::Physics;
using namespace SAGE::ECS;
using namespace TestFramework;

/// @brief Тест инициализации Box2D backend
TEST(Physics_Box2DInitialization) {
        
    Box2DBackend backend;
    
    PhysicsSettings settings;
    settings.gravity = Vector2(0.0f, 980.0f);
    
    backend.Initialize(settings);
    
    auto gravity = backend.GetGravity();
    CHECK_NEAR(gravity.y, 980.0f, 0.1f);
    
    backend.Clear();
}

/// @brief Тест создания физического тела
TEST(Physics_CreateBody) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    backend.Initialize(settings);
    
    Registry registry;
    Entity entity = registry.CreateEntity();
    
    registry.AddComponent(entity, TransformComponent(100, 200));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Dynamic);
    physics.SetMass(1.0f);
    registry.AddComponent(entity, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 64.0f));
    registry.AddComponent(entity, collider);
    
    bool created = backend.CreateBody(entity, registry);
    CHECK(created);
    
    backend.Clear();
}

/// @brief Тест симуляции физики
TEST(Physics_Simulation) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    settings.gravity = Vector2(0.0f, 980.0f);
    backend.Initialize(settings);
    
    Registry registry;
    Entity entity = registry.CreateEntity();
    
    // Создаём падающий объект
    registry.AddComponent(entity, TransformComponent(0, 100));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Dynamic);
    physics.SetMass(1.0f);
    registry.AddComponent(entity, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
    registry.AddComponent(entity, collider);
    
    backend.CreateBody(entity, registry);
    
    // Симулируем несколько шагов
    float deltaTime = 0.016f; // 60 FPS
    for (int i = 0; i < 60; i++) { // 1 секунда
        backend.Step(registry, deltaTime);
        backend.SyncTransforms(registry);
    }
    
    // Объект должен упасть вниз под действием гравитации
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    ASSERT_NOT_NULL(transform);
    CHECK(transform->position.y > 100.0f); // Должен сместиться вниз
    
    backend.Clear();
}

/// @brief Тест статических тел
TEST(Physics_StaticBodies) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    backend.Initialize(settings);
    
    Registry registry;
    Entity groundEntity = registry.CreateEntity();
    
    registry.AddComponent(groundEntity, TransformComponent(0, 500));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Static);
    registry.AddComponent(groundEntity, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(1000.0f, 100.0f));
    registry.AddComponent(groundEntity, collider);
    
    backend.CreateBody(groundEntity, registry);
    
    auto* initialTransform = registry.GetComponent<TransformComponent>(groundEntity);
    ASSERT_NOT_NULL(initialTransform);
    float initialY = initialTransform->position.y;
    
    // Симулируем
    for (int i = 0; i < 60; i++) {
        backend.Step(registry, 0.016f);
        backend.SyncTransforms(registry);
    }
    
    // Статическое тело не должно двигаться
    auto* finalTransform = registry.GetComponent<TransformComponent>(groundEntity);
    ASSERT_NOT_NULL(finalTransform);
    CHECK_NEAR(finalTransform->position.y, initialY, 0.1f);
    
    backend.Clear();
}

/// @brief Тест Raycast
TEST(Physics_Raycast) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    backend.Initialize(settings);
    
    Registry registry;
    Entity obstacle = registry.CreateEntity();
    
    registry.AddComponent(obstacle, TransformComponent(100, 100));
    
    PhysicsComponent physics;
    physics.SetType(PhysicsBodyType::Static);
    registry.AddComponent(obstacle, physics);
    
    auto collider = ColliderComponent::CreateBox(Vector2(50.0f, 50.0f));
    registry.AddComponent(obstacle, collider);
    
    backend.CreateBody(obstacle, registry);
    
    // Raycast сквозь объект
    RaycastHit hit;
    Vector2 origin(50, 100);
    Vector2 direction(1, 0); // Вправо
    bool hitResult = backend.Raycast(origin, direction, 100.0f, hit);
    
    CHECK(hitResult);
    CHECK(hit.entity != NullEntity);
    
    backend.Clear();
}

/// @brief Тест AABB Query
TEST(Physics_AABBQuery) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    backend.Initialize(settings);
    
    Registry registry;
    
    // Создаём несколько объектов
    for (int i = 0; i < 5; i++) {
        Entity e = registry.CreateEntity();
        registry.AddComponent(e, TransformComponent(i * 50.0f, 100));
        
        PhysicsComponent physics;
        physics.SetType(PhysicsBodyType::Static);
        registry.AddComponent(e, physics);
        
        auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
        registry.AddComponent(e, collider);
        
        backend.CreateBody(e, registry);
    }
    
    // Запрос в определённой области
    std::vector<Entity> found;
    Vector2 min(0, 50);
    Vector2 max(150, 150);
    backend.QueryAABB(min, max, found);
    
    // Должны найти несколько объектов
    CHECK(found.size() >= 3);
    
    backend.Clear();
}

/// @brief Стресс-тест: много физических тел
TEST(Physics_StressTest_ManyBodies) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    backend.Initialize(settings);
    
    Registry registry;
    const int bodyCount = 100;
    
    // Создаём много тел
    for (int i = 0; i < bodyCount; i++) {
        Entity e = registry.CreateEntity();
        registry.AddComponent(e, TransformComponent(
            (i % 10) * 40.0f, 
            (i / 10) * 40.0f
        ));
        
        PhysicsComponent physics;
        physics.SetType(PhysicsBodyType::Dynamic);
        physics.SetMass(1.0f);
        registry.AddComponent(e, physics);
        
        auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
        registry.AddComponent(e, collider);
        
        backend.CreateBody(e, registry);
    }
    
    // Симулируем
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 60; i++) {
        backend.Step(registry, 0.016f);
        backend.SyncTransforms(registry);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Должно работать разумно быстро
    CHECK(duration.count() < 1000); // < 1 секунды для 100 тел
    
    backend.Clear();
}

/// @brief Тест изменения гравитации
TEST(Physics_GravityChange) {
        
    Box2DBackend backend;
    PhysicsSettings settings;
    settings.gravity = Vector2(0, 0); // Нет гравитации
    backend.Initialize(settings);
    
    auto gravity = backend.GetGravity();
    CHECK_NEAR(gravity.y, 0.0f, 0.001f);
    
    // Изменяем гравитацию
    backend.SetGravity(Vector2(0, 980));
    
    gravity = backend.GetGravity();
    CHECK_NEAR(gravity.y, 980.0f, 0.1f);
    
    backend.Clear();
}
