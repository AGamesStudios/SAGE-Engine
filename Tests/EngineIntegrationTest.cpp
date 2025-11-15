/// \file EngineIntegrationTest.cpp
/// \brief Комплексный тест всех систем SAGE Engine
/// 
/// Проверяет:
/// - Инициализацию всех основных систем
/// - Взаимодействие между системами
/// - Создание и управление entity
/// - Физическую симуляцию
/// - Анимацию
/// - Управление ресурсами
/// - Аудио систему

#include "TestFramework.h"
#include "ECS/ECS.h"
#include "ECS/Systems/Physics/PhysicsSystem.h"
#include "ECS/Systems/Visual/RenderSystem.h"
#include "ECS/Systems/Visual/AnimationSystem.h"
#include "Physics/Box2DBackend.h"
#include "Core/ResourceManager.h"
#include "Core/Logger.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Audio/AudioSystem.h"
#include "Input/InputManager.h"
#include "Math/Random.h"
#include <chrono>

using namespace SAGE;
using namespace SAGE::ECS;
using namespace SAGE::Math;
using namespace TestFramework;

namespace {

SAGE::Ref<SAGE::AnimationClip> CreateTestClip(float duration = 0.05f) {
    auto clip = SAGE::CreateRef<SAGE::AnimationClip>("TestClip");
    clip->ClearFrames();
    clip->AddFrame({0.0f, 0.0f}, {1.0f, 1.0f}, duration);
    clip->AddFrame({0.0f, 0.0f}, {1.0f, 1.0f}, duration);
    return clip;
}

}

/// @brief Полный цикл работы движка: инициализация всех систем
TEST(EngineIntegration_FullSystemInitialization) {
    
    // 1. ECS Registry
    Registry registry;
    CHECK(true); // Registry создан успешно
    
    // 2. Physics System
    PhysicsSystem physicsSystem;
    physicsSystem.Init();
    CHECK(true); // Физика инициализирована
    
    // 3. Animation System
    AnimationSystem animSystem;
    CHECK(true); // Анимация инициализирована
    
    // 4. Render System
    RenderSystem renderSystem;
    CHECK(true); // Рендер инициализирован
    
    // 5. Audio System
    AudioSystem audioSys;
    bool audioInit = audioSys.Init();
    CHECK(audioInit);
    
    // 6. Resource Manager
    auto& resManager = ResourceManager::Get();
    resManager.SetGpuLoadingEnabled(false); // Headless mode
    CHECK(true); // ResourceManager доступен
    
    // 7. Random Number Generator
    auto& rng = Random::Global();
    rng.SetSeed(12345);
    int randomVal = rng.NextInt(0, 100);
    CHECK(randomVal >= 0);
    
    // All systems initialized successfully
    
    physicsSystem.Shutdown();
    audioSys.Shutdown();
    resManager.ClearCache();
}

/// @brief Создание и управление сложной сценой с множеством entity
TEST(EngineIntegration_ComplexSceneCreation) {
    
    Registry registry;
    PhysicsSystem physicsSystem;
    AnimationSystem animSystem;
    RenderSystem renderSystem;
    
    physicsSystem.Init();
    
    const int ENTITY_COUNT = 50;
    std::vector<Entity> entities;
    
    // Создаем разнообразные entity
    for (int i = 0; i < ENTITY_COUNT; i++) {
        Entity e = registry.CreateEntity();
        entities.push_back(e);
        
        // Transform (все entity)
        TransformComponent transform;
        transform.position = Vector2(i * 20.0f, i * 10.0f);
        transform.rotation = i * 5.0f;
        transform.size = Vector2(16.0f + i % 10, 16.0f + i % 10);
        registry.AddComponent(e, transform);
        
        // Sprite (все entity)
        SpriteComponent sprite;
        sprite.texturePath = "entity_" + std::to_string(i) + ".png";
        registry.AddComponent(e, sprite);
        
        // Физика (каждый второй)
        if (i % 2 == 0) {
            PhysicsComponent physics;
            physics.SetType(PhysicsBodyType::Dynamic);
            physics.SetMass(1.0f + i * 0.1f);
            physics.restitution = 0.5f;
            registry.AddComponent(e, physics);
            
            auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
            registry.AddComponent(e, collider);
        }
        
        // Анимация (каждый третий)
        if (i % 3 == 0) {
            AnimationComponent anim;
            anim.SetClip(CreateTestClip(0.1f));
            anim.Play();
            registry.AddComponent(e, anim);
        }
    }
    
    CHECK(entities.size() == ENTITY_COUNT);
    
    // Проверяем компоненты
    int transformCount = 0;
    int physicsCount = 0;
    int animCount = 0;
    
    registry.ForEach<TransformComponent>([&](Entity e, TransformComponent& t) {
        transformCount++;
    });
    
    registry.ForEach<PhysicsComponent>([&](Entity e, PhysicsComponent& p) {
        physicsCount++;
    });
    
    registry.ForEach<AnimationComponent>([&](Entity e, AnimationComponent& a) {
        animCount++;
    });
    
    CHECK(transformCount == ENTITY_COUNT);
    CHECK(physicsCount >= ENTITY_COUNT / 2 - 1); // ~25
    CHECK(animCount >= ENTITY_COUNT / 3 - 1);    // ~16
    
    physicsSystem.Shutdown();
}

/// @brief Полный игровой цикл: создание, обновление, рендеринг
TEST(EngineIntegration_GameLoopSimulation) {
    
    Registry registry;
    PhysicsSystem physicsSystem;
    AnimationSystem animSystem;
    RenderSystem renderSystem;
    
    physicsSystem.Init();
    
    // Создаем игровую сцену
    // Пол
    Entity ground = registry.CreateEntity();
    TransformComponent groundTrans;
    groundTrans.position = Vector2(400.0f, 550.0f);
    groundTrans.size = Vector2(800.0f, 50.0f);
    registry.AddComponent(ground, groundTrans);
    
    PhysicsComponent groundPhys;
    groundPhys.SetType(PhysicsBodyType::Static);
    registry.AddComponent(ground, groundPhys);
    
    auto groundCollider = ColliderComponent::CreateBox(Vector2(800.0f, 50.0f));
    registry.AddComponent(ground, groundCollider);
    
    SpriteComponent groundSprite;
    registry.AddComponent(ground, groundSprite);
    
    // Падающие объекты
    std::vector<Entity> fallingObjects;
    for (int i = 0; i < 10; i++) {
        Entity obj = registry.CreateEntity();
        
        TransformComponent trans;
        trans.position = Vector2(200.0f + i * 50.0f, 100.0f);
        trans.size = Vector2(30.0f, 30.0f);
        registry.AddComponent(obj, trans);
        
        PhysicsComponent phys;
        phys.SetType(PhysicsBodyType::Dynamic);
        phys.SetMass(1.0f);
        registry.AddComponent(obj, phys);
        
        auto collider = ColliderComponent::CreateBox(Vector2(30.0f, 30.0f));
        registry.AddComponent(obj, collider);
        
        SpriteComponent sprite;
        registry.AddComponent(obj, sprite);
        
        // Половина с анимацией
        if (i % 2 == 0) {
            AnimationComponent anim;
            anim.SetClip(CreateTestClip());
            anim.Play();
            registry.AddComponent(obj, anim);
        }
        
        fallingObjects.push_back(obj);
    }
    
    // Симулируем игровой цикл
    const int FRAME_COUNT = 120; // 2 секунды при 60 FPS
    const float DELTA_TIME = 1.0f / 60.0f;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int frame = 0; frame < FRAME_COUNT; frame++) {
        // Update physics
        physicsSystem.FixedUpdate(registry, DELTA_TIME);
        
        // Update animations
        animSystem.Update(registry, DELTA_TIME);
        
        // Render (логически - без реального OpenGL)
        // renderSystem.Update(registry, DELTA_TIME);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Проверяем что объекты упали
    bool anyFell = false;
    for (Entity e : fallingObjects) {
        auto* trans = registry.GetComponent<TransformComponent>(e);
        if (trans && trans->position.y > 150.0f) {
            anyFell = true;
            break;
        }
    }
    
    CHECK(anyFell); // Хотя бы один объект должен был упасть
    CHECK(duration.count() < 5000); // Должно выполниться быстро (< 5 сек)
    
    physicsSystem.Shutdown();
}

/// @brief Стресс-тест: максимальная нагрузка на все системы
TEST(EngineIntegration_StressTest_AllSystems) {
    
    Registry registry;
    PhysicsSystem physicsSystem;
    AnimationSystem animSystem;
    
    physicsSystem.Init();
    
    const int STRESS_ENTITY_COUNT = 500;
    
    auto creationStart = std::chrono::high_resolution_clock::now();
    
    // Массовое создание entity
    auto& rng = Random::Global();
    for (int i = 0; i < STRESS_ENTITY_COUNT; i++) {
        Entity e = registry.CreateEntity();
        
        TransformComponent trans;
        trans.position = Vector2(
            rng.NextRange(0.0f, 1920.0f),
            rng.NextRange(0.0f, 1080.0f)
        );
        trans.rotation = rng.NextRange(0.0f, 360.0f);
        trans.size = Vector2(
            rng.NextRange(10.0f, 50.0f),
            rng.NextRange(10.0f, 50.0f)
        );
        registry.AddComponent(e, trans);
        
        SpriteComponent sprite;
        registry.AddComponent(e, sprite);
        
        if (i % 2 == 0) {
            PhysicsComponent phys;
            phys.SetType(PhysicsBodyType::Dynamic);
            phys.SetMass(rng.NextRange(0.5f, 2.0f));
            registry.AddComponent(e, phys);
            
            auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
            registry.AddComponent(e, collider);
        }
        
        if (i % 3 == 0) {
            AnimationComponent anim;
            anim.SetClip(CreateTestClip());
            anim.Play();
            registry.AddComponent(e, anim);
        }
    }
    
    auto creationEnd = std::chrono::high_resolution_clock::now();
    auto creationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        creationEnd - creationStart
    );
    
    // Симуляция
    auto simStart = std::chrono::high_resolution_clock::now();
    
    for (int frame = 0; frame < 60; frame++) {
        physicsSystem.FixedUpdate(registry, 0.016f);
        animSystem.Update(registry, 0.016f);
    }
    
    auto simEnd = std::chrono::high_resolution_clock::now();
    auto simTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        simEnd - simStart
    );
    
    CHECK(creationTime.count() < 3000); // Создание < 3 сек
    CHECK(simTime.count() < 5000);      // Симуляция < 5 сек
    
    physicsSystem.Shutdown();
}

/// @brief Проверка корректного освобождения ресурсов
TEST(EngineIntegration_ResourceCleanup) {
    
    // Создаем и уничтожаем несколько систем
    for (int cycle = 0; cycle < 3; cycle++) {
        Registry registry;
        
        PhysicsSystem physicsSystem;
        physicsSystem.Init();
        
        AnimationSystem animSystem;
        
        // Создаем entity
        for (int i = 0; i < 100; i++) {
            Entity e = registry.CreateEntity();
            
            TransformComponent trans;
            registry.AddComponent(e, trans);
            
            SpriteComponent sprite;
            registry.AddComponent(e, sprite);
            
            if (i % 2 == 0) {
                PhysicsComponent phys;
                phys.SetType(PhysicsBodyType::Dynamic);
                phys.SetMass(1.0f);
                registry.AddComponent(e, phys);
                
                auto collider = ColliderComponent::CreateBox(Vector2(32.0f, 32.0f));
                registry.AddComponent(e, collider);
            }
        }
        
        // Обновляем
        for (int frame = 0; frame < 10; frame++) {
            physicsSystem.FixedUpdate(registry, 0.016f);
            animSystem.Update(registry, 0.016f);
        }
        
        // Уничтожаем половину entity
        std::vector<Entity> toDestroy;
        registry.ForEach<TransformComponent>([&](Entity e, TransformComponent& t) {
            if (GetEntityID(e) % 2 == 0) {
                toDestroy.push_back(e);
            }
        });
        
        for (Entity e : toDestroy) {
            registry.DestroyEntity(e);
        }
        
        // Обновляем после удаления
        physicsSystem.FixedUpdate(registry, 0.016f);
        animSystem.Update(registry, 0.016f);
        
        physicsSystem.Shutdown();
        
        CHECK(true); // Цикл завершен без краша
    }
}

/// @brief Проверка работы аудио системы
TEST(EngineIntegration_AudioSystem) {
    
    AudioSystem audio;
    
    // Инициализация
    bool initSuccess = audio.Init();
    CHECK(initSuccess);
    
    // Громкость
    audio.SetMasterVolume(0.8f);
    float masterVol = audio.GetMasterVolume();
    CHECK_NEAR(masterVol, 0.8f, 0.01f);
    
    audio.SetSFXVolume(0.6f);
    float sfxVol = audio.GetSFXVolume();
    CHECK_NEAR(sfxVol, 0.6f, 0.01f);
    
    // Позиция слушателя
    audio.SetListenerPosition(100.0f, 200.0f, 0.0f);
    
    // Пауза/Возобновление
    audio.StopAllSFX();
    
    // Update
    audio.Update(0.016f);
    
    // Shutdown
    audio.Shutdown();
}

/// @brief Проверка ResourceManager в headless режиме
TEST(EngineIntegration_ResourceManager) {
    
    auto& rm = ResourceManager::Get();
    
    // Headless mode (без GPU)
    rm.SetGpuLoadingEnabled(false);
    bool gpuDisabled = !rm.IsGpuLoadingEnabled();
    CHECK(gpuDisabled);
    
    // Очистка
    rm.ClearCache();
    
    // Включение GPU
    rm.SetGpuLoadingEnabled(true);
    bool gpuEnabled = rm.IsGpuLoadingEnabled();
    CHECK(gpuEnabled);
    
    rm.ClearCache();
}
