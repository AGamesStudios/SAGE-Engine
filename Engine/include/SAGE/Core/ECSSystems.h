#pragma once

#include "SAGE/Core/ECS.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Animation.h"
#include "SAGE/Input/Input.h"
#include "SAGE/Physics/PhysicsWorld.h"

#include <functional>
#include <unordered_map>

namespace SAGE::ECS {

// Обновление анимаций
class AnimationSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Рендер спрайтов
class SpriteRenderSystem : public ISystem {
public:
    using DrawCallback = std::function<void(Sprite&)>;
    void SetDrawCallback(DrawCallback cb) { m_DrawCallback = std::move(cb); }
    void Tick(Registry& reg, float deltaTime) override;
private:
    DrawCallback m_DrawCallback;
};

// Render Tilemaps
class TilemapRenderSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

class NativeScriptSystem : public ISystem {
public:
    NativeScriptSystem(Scene* scene) : m_Scene(scene) {}
    void Tick(Registry& reg, float deltaTime) override;
private:
    Scene* m_Scene;
};

// Движение по Velocity
class MovementSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Следование по пути
class PathFollowSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

class CollisionSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Проверка "на земле" для прыжков
class GroundCheckSystem : public ISystem {
public:
    explicit GroundCheckSystem(Physics::PhysicsWorld& physicsWorld) : m_PhysicsWorld(physicsWorld) {}
    void Tick(Registry& reg, float deltaTime) override;
private:
    Physics::PhysicsWorld& m_PhysicsWorld;
};

class PlatformBehaviorSystem : public ISystem {
public:
    explicit PlatformBehaviorSystem(Physics::PhysicsWorld& physicsWorld) : m_PhysicsWorld(physicsWorld) {}
    void Tick(Registry& reg, float deltaTime) override;
private:
    Physics::PhysicsWorld& m_PhysicsWorld;
};

// Простое управление игроком
class PlayerInputSystem : public ISystem {
public:
    struct InputState {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool jump = false;
    };

    using InputProvider = std::function<InputState()>;

    void Tick(Registry& reg, float deltaTime) override;
    float moveSpeed = 250.0f;
    void SetInputProvider(InputProvider provider) { m_Provider = std::move(provider); }
private:
    InputProvider m_Provider;
};

// Камера следует за сущностью с компонентом CameraFollowComponent
class CameraFollowSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Проигрывание звуков из AudioComponent
class AudioSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

class PhysicsSystem : public ISystem {
public:
    explicit PhysicsSystem(Physics::PhysicsWorld& world);
    void Tick(Registry& reg, float deltaTime) override;
    void FixedTick(Registry& reg, float fixedDeltaTime) override;
    
    // Debug draw
    void DrawDebug(Registry& reg);

private:
    Physics::PhysicsWorld& m_World;
    Registry* m_CurrentRegistry = nullptr;
    
    void InitBody(Entity e, RigidBodyComponent& rb, TransformComponent& trans, PhysicsColliderComponent* collider);
    void SyncTransformToBody(Entity e, RigidBodyComponent& rb, TransformComponent& trans);
    void SyncBodyToTransform(Entity e, RigidBodyComponent& rb, TransformComponent& trans);
    
    void OnContact(const Physics::ContactEvent& event);
};

class StatsSystem : public ISystem {
public:
    float regenHealthPerSec = 0.0f;
    float regenEnergyPerSec = 0.0f;
    void Tick(Registry& reg, float deltaTime) override;
};

// Обновление InputComponent состояниями ввода
class InputStateSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Обновление/рендер частиц через ParticleSystem
class ParticleSystemSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Удаление сущностей с нулевым здоровьем
class DeathSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Отрисовка простого HUD (HP/энергия, пауза)
class HudRenderSystem : public ISystem {
public:
    explicit HudRenderSystem(bool* pauseFlag) : m_PauseFlag(pauseFlag) {}
    void Tick(Registry& reg, float deltaTime) override;
private:
    bool* m_PauseFlag = nullptr;
};



class DamageSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

class CameraSystem : public ISystem {
public:
    void Tick(Registry& reg, float deltaTime) override;
};

// Система для обработки кликов и рейкастов
class RaycastSystem : public ISystem {
public:
    explicit RaycastSystem(Physics::PhysicsWorld& physicsWorld) : m_PhysicsWorld(physicsWorld) {}
    void Tick(Registry& reg, float deltaTime) override;
    
    // Helper to perform a raycast from screen coordinates
    // Returns the first entity hit
    Entity RaycastFromScreen(Registry& reg, const Vector2& screenPos, const Camera2D& camera);

    // Perform a physics raycast
    Physics::PhysicsWorld::RayCastHit Raycast(const Vector2& start, const Vector2& end);

private:
    Physics::PhysicsWorld& m_PhysicsWorld;
};
} // namespace SAGE::ECS
