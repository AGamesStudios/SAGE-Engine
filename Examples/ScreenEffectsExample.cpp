// ===================================
// Screen Effects - Примеры использования
// ===================================

#include "ECS/ECS.h"

using namespace SAGE;
using namespace SAGE::ECS;

// ===================================
// CAMERA SHAKE
// ===================================

void Example_CameraShake() {
    Registry registry;
    
    // Создать камеру с screen effects
    Entity camera = registry.CreateEntity();
    registry.AddComponent(camera, CameraComponent());
    auto& effects = registry.AddComponent<ScreenEffectsComponent>(camera);
    
    // === Пример 1: Слабая тряска (попадание) ===
    effects.Shake(0.2f, 5.0f);  // 0.2 сек, интенсивность 5
    
    // === Пример 2: Средняя тряска (взрыв) ===
    effects.Shake(0.5f, 15.0f, 25.0f);  // 0.5 сек, интенсивность 15, частота 25
    
    // === Пример 3: Сильная тряска (землетрясение) ===
    effects.Shake(2.0f, 30.0f, 15.0f);  // 2 сек, интенсивность 30, частота 15
    
    // В Update:
    // ScreenEffectsSystem system;
    // system.Update(registry, deltaTime);
    // system.ApplyToCamera(registry, camera);
}

// ===================================
// SCREEN FLASH
// ===================================

void Example_ScreenFlash() {
    Registry registry;
    
    Entity camera = registry.CreateEntity();
    auto& effects = registry.AddComponent<ScreenEffectsComponent>(camera);
    
    // === Пример 1: Белая вспышка (урон игрока) ===
    effects.Flash(0.15f, 1.0f, 1.0f, 1.0f, 0.7f);  // Белый, 70% прозрачность
    
    // === Пример 2: Красная вспышка (критический урон) ===
    effects.Flash(0.3f, 1.0f, 0.0f, 0.0f, 0.5f);  // Красный
    
    // === Пример 3: Жёлтая вспышка (лечение) ===
    effects.Flash(0.4f, 1.0f, 1.0f, 0.0f, 0.4f);  // Жёлтый
    
    // === Пример 4: Синяя вспышка (телепорт) ===
    effects.Flash(0.5f, 0.0f, 0.5f, 1.0f, 0.6f);  // Синий
}

// ===================================
// SCREEN TRANSITIONS
// ===================================

void Example_ScreenTransitions() {
    Registry registry;
    
    Entity camera = registry.CreateEntity();
    auto& effects = registry.AddComponent<ScreenEffectsComponent>(camera);
    
    // === Пример 1: Fade to black при смерти ===
    effects.FadeOut(1.0f, []() {
        // Callback: перезагрузить уровень
        // SceneManager::LoadScene("GameOver");
    });
    
    // === Пример 2: Fade in при старте уровня ===
    effects.FadeIn(1.5f, []() {
        // Callback: начать игру
        // GameManager::StartLevel();
    });
    
    // === Пример 3: Переход между комнатами ===
    auto TransitionToRoom = [&effects](const std::string& roomName) {
        // Fade out
        effects.FadeOut(0.5f, [&effects, roomName]() {
            // Загрузить комнату
            // RoomManager::LoadRoom(roomName);
            
            // Fade in
            effects.FadeIn(0.5f);
        });
    };
}

// ===================================
// КОМБИНАЦИЯ ЭФФЕКТОВ
// ===================================

void Example_CombinedEffects() {
    Registry registry;
    
    Entity camera = registry.CreateEntity();
    auto& effects = registry.AddComponent<ScreenEffectsComponent>(camera);
    
    // === Взрыв: Shake + Flash одновременно ===
    auto Explosion = [&effects]() {
        effects.Shake(0.8f, 20.0f, 20.0f);           // Сильная тряска
        effects.Flash(0.5f, 1.0f, 0.5f, 0.0f, 0.8f); // Оранжевая вспышка
    };
    
    // === Критический удар: Shake + Flash + Freeze ===
    auto CriticalHit = [&effects]() {
        effects.Shake(0.3f, 10.0f);                  // Средняя тряска
        effects.Flash(0.2f, 1.0f, 1.0f, 0.0f, 0.6f); // Жёлтая вспышка
        // TimeManager::FreezeFrame(0.1f);            // Freeze frame
    };
    
    // === Смерть босса: Flash -> Shake -> Transition ===
    auto BossDeath = [&effects]() {
        // 1. Белая вспышка
        effects.Flash(0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
        
        // 2. Через 0.5 сек - тряска
        // Timer::DelayedCall(0.5f, [&effects]() {
        //     effects.Shake(2.0f, 25.0f, 15.0f);
        // });
        
        // 3. Через 2.5 сек - переход
        // Timer::DelayedCall(2.5f, [&effects]() {
        //     effects.FadeOut(1.0f, []() {
        //         SceneManager::LoadScene("Victory");
        //     });
        // });
    };
}

// ===================================
// MOTION TRAIL (следы)
// ===================================

void Example_MotionTrail() {
    Registry registry;
    
    // Создать игрока с trail
    Entity player = registry.CreateEntity();
    registry.AddComponent(player, TransformComponent(100, 200));
    auto& trail = registry.AddComponent<TrailComponent>(player);
    
    // === Пример 1: Постоянный trail (бег) ===
    trail.SetupTrail(
        0.5f,   // pointLifetime - след живёт 0.5 сек
        0.05f,  // emissionRate - точка каждые 0.05 сек
        10.0f,  // startWidth - начальная ширина 10
        2.0f    // endWidth - конечная ширина 2
    );
    trail.EnableTrail(true);
    
    // === Пример 2: Trail только при быстром движении ===
    // В Update:
    // float speed = player.GetVelocity().Length();
    // if (speed > 200.0f) {
    //     trail.EnableTrail(true);
    // } else {
    //     trail.EnableTrail(false);
    // }
    
    // === Пример 3: Настройка цветов ===
    trail.trail.startColor = Color(0.0f, 0.5f, 1.0f, 1.0f);  // Синий
    trail.trail.endColor = Color(0.0f, 0.5f, 1.0f, 0.0f);    // Прозрачный синий
    trail.trail.startAlpha = 0.8f;
    trail.trail.endAlpha = 0.0f;
}

// ===================================
// DASH EFFECT
// ===================================

void Example_DashEffect() {
    Registry registry;
    
    Entity player = registry.CreateEntity();
    registry.AddComponent(player, TransformComponent(100, 200));
    auto& trail = registry.AddComponent<TrailComponent>(player);
    
    // Настроить dash эффект
    trail.SetupDash(
        0.3f,  // ghostLifetime - призрак живёт 0.3 сек
        0.05f, // interval - призрак каждые 0.05 сек
        10     // maxGhosts - максимум 10 призраков
    );
    
    trail.dashEffect.ghostColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
    
    // === Использование в dash механике ===
    auto Dash = [&trail](const Vector2& direction) {
        // 1. Начать dash эффект
        trail.StartDash();
        
        // 2. Применить силу dash
        // player.ApplyForce(direction * 1000.0f);
        
        // 3. Через 0.3 сек остановить эффект
        // Timer::DelayedCall(0.3f, [&trail]() {
        //     trail.StopDash();
        // });
    };
}

// ===================================
// ПОЛНЫЙ ПРИМЕР: DASH С ЭФФЕКТАМИ
// ===================================

void Example_FullDash() {
    Registry registry;
    
    // Игрок
    Entity player = registry.CreateEntity();
    registry.AddComponent(player, TransformComponent(100, 200));
    auto& trail = registry.AddComponent<TrailComponent>(player);
    
    // Камера
    Entity camera = registry.CreateEntity();
    auto& effects = registry.AddComponent<ScreenEffectsComponent>(camera);
    
    // Настройка dash
    trail.SetupDash(0.25f, 0.04f, 8);
    trail.dashEffect.ghostColor = Color(0.5f, 0.8f, 1.0f, 0.6f);  // Голубой
    
    // === Dash функция ===
    auto PerformDash = [&trail, &effects](const Vector2& direction) {
        // 1. Trail эффект
        trail.StartDash();
        
        // 2. Screen flash (синяя вспышка)
        effects.Flash(0.15f, 0.3f, 0.5f, 1.0f, 0.4f);
        
        // 3. Лёгкая тряска
        effects.Shake(0.2f, 3.0f, 30.0f);
        
        // 4. Применить движение
        // Vector2 dashForce = direction.Normalized() * 800.0f;
        // physics.ApplyImpulse(dashForce);
        
        // 5. Остановить dash через 0.25 сек
        // Timer::DelayedCall(0.25f, [&trail]() {
        //     trail.StopDash();
        // });
    };
}

// ===================================
// СИСТЕМЫ В ИГРОВОМ ЦИКЛЕ
// ===================================

void Example_Systems() {
    Registry registry;
    
    // Создать системы
    ScreenEffectsSystem effectsSystem;
    TrailUpdateSystem trailSystem;
    
    // В игровом цикле:
    // void Update(float deltaTime) {
    //     // 1. Обновить screen effects
    //     effectsSystem.Update(registry, deltaTime);
    //     
    //     // 2. Обновить trail
    //     trailSystem.Update(registry, deltaTime);
    //     
    //     // 3. Применить shake к камере
    //     Entity mainCamera = GetMainCamera();
    //     effectsSystem.ApplyToCamera(registry, mainCamera);
    // }
}
