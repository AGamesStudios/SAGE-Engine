#pragma once

#include "SAGE/Core/Game.h"
#include "SAGE/Core/ECS.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Physics/PhysicsWorld.h"

namespace SAGE {

// Базовая обёртка над Game с интегрированным ECS Registry/SystemScheduler
class ECSGame : public Game {
public:
    explicit ECSGame(const ApplicationConfig& config = {});
    ~ECSGame() override = default;

    ECS::Registry& GetRegistry() { return m_World; }
    ECS::SystemScheduler& GetScheduler() { return m_Scheduler; }
    Camera2D& GetCamera() { return *m_Camera; }
    Physics::PhysicsWorld& GetPhysicsWorld() { return m_PhysicsWorld; }
    ECS::RaycastSystem* GetRaycastSystem() { return m_RaycastSystem; }

    void ReloadScene();

protected:
    void OnInit() override;
    void OnUpdate(double deltaTime) override;
    void OnFixedUpdate(double fixedDeltaTime) override;
    void OnGameRender() override;
    void OnResize(int width, int height) override;
    void OnShutdown() override;
    void OnFocusChanged(bool focused) override;

    // Хук для пользовательской логики
    virtual void OnECSCreate() {}
    virtual void OnECSUpdate(float /*dt*/) {}
    virtual void OnECSRender() {}

    // Установка сущности, за которой следует камера
    void SetCameraTarget(ECS::Entity entity, float smoothness = 5.0f);

    void SetDebugPhysics(bool enabled) { m_DebugPhysics = enabled; }

private:
    ECS::Registry m_World;
    ECS::SystemScheduler m_Scheduler;
    Physics::PhysicsWorld m_PhysicsWorld; // Add PhysicsWorld instance
    std::unique_ptr<Camera2D> m_Camera;
    ECS::Entity m_CameraTarget = ECS::kInvalidEntity;
    ECS::Entity m_CameraEntity = ECS::kInvalidEntity;
    float m_CameraSmooth = 5.0f;
    bool m_Paused = false;
    bool m_DebugPhysics = false;

    // Встроенные системы
    ECS::AnimationSystem* m_AnimationSystem = nullptr;
    ECS::SpriteRenderSystem* m_SpriteRenderSystem = nullptr;
    ECS::MovementSystem* m_MovementSystem = nullptr;
    ECS::GroundCheckSystem* m_GroundCheckSystem = nullptr;
    ECS::PlatformBehaviorSystem* m_PlatformBehaviorSystem = nullptr;
    ECS::RaycastSystem* m_RaycastSystem = nullptr;
    ECS::PlayerInputSystem* m_PlayerInputSystem = nullptr;
    ECS::AudioSystem* m_AudioSystem = nullptr;
    ECS::CameraFollowSystem* m_CameraFollowSystem = nullptr;
    ECS::StatsSystem* m_StatsSystem = nullptr;
    ECS::InputStateSystem* m_InputStateSystem = nullptr;
    ECS::ParticleSystemSystem* m_ParticleSystemSystem = nullptr;
    ECS::CollisionSystem* m_CollisionSystem = nullptr;
    ECS::HudRenderSystem* m_HudSystem = nullptr;
    ECS::PhysicsSystem* m_PhysicsSystem = nullptr;
    ECS::DeathSystem* m_DeathSystem = nullptr;
};

} // namespace SAGE
