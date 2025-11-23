#include "SAGE/Core/ECSGame.h"

namespace SAGE {

ECSGame::ECSGame(const ApplicationConfig& config)
    : Game(config) {
}

void ECSGame::OnInit() {
    // Создаём камеру по размеру окна
    int w = 1280, h = 720;
    GetWindow().GetFramebufferSize(w, h);
    m_Camera = std::make_unique<Camera2D>(static_cast<float>(w), static_cast<float>(h));
    m_Camera->SetPosition({w * 0.5f, h * 0.5f});

    Renderer::SetCamera(*m_Camera);

    // Create Camera Entity
    m_CameraEntity = m_World.CreateEntity();
    auto& camComp = m_World.Add<ECS::CameraComponent>(m_CameraEntity);
    camComp.camera = *m_Camera;
    camComp.active = true;

    // Регистрируем встроенные системы в порядке обновления и сохраняем ссылки
    m_InputStateSystem = &m_Scheduler.AddSystem<ECS::InputStateSystem>();
    m_PlayerInputSystem = &m_Scheduler.AddSystem<ECS::PlayerInputSystem>();
    m_MovementSystem = &m_Scheduler.AddSystem<ECS::MovementSystem>();
    m_CollisionSystem = &m_Scheduler.AddSystem<ECS::CollisionSystem>();
    m_GroundCheckSystem = &m_Scheduler.AddSystem<ECS::GroundCheckSystem>(m_PhysicsWorld);
    m_PlatformBehaviorSystem = &m_Scheduler.AddSystem<ECS::PlatformBehaviorSystem>(m_PhysicsWorld);
    m_RaycastSystem = &m_Scheduler.AddSystem<ECS::RaycastSystem>(m_PhysicsWorld);
    m_StatsSystem = &m_Scheduler.AddSystem<ECS::StatsSystem>();
    m_AnimationSystem = &m_Scheduler.AddSystem<ECS::AnimationSystem>();
    m_ParticleSystemSystem = &m_Scheduler.AddSystem<ECS::ParticleSystemSystem>();
    m_AudioSystem = &m_Scheduler.AddSystem<ECS::AudioSystem>();
    m_PhysicsSystem = &m_Scheduler.AddSystem<ECS::PhysicsSystem>(m_PhysicsWorld);
    m_DeathSystem = &m_Scheduler.AddSystem<ECS::DeathSystem>();
    m_CameraFollowSystem = &m_Scheduler.AddSystem<ECS::CameraFollowSystem>();
    m_SpriteRenderSystem = &m_Scheduler.AddSystem<ECS::SpriteRenderSystem>();
    m_HudSystem = &m_Scheduler.AddSystem<ECS::HudRenderSystem>(&m_Paused);

    // Register Physics Cleanup
    m_World.SetOnComponentRemoved<ECS::RigidBodyComponent>([this](ECS::Entity, ECS::RigidBodyComponent& rb) {
        if (rb.IsValid()) {
            m_PhysicsWorld.DestroyBody(rb.bodyHandle);
        }
    });

    OnECSCreate();
}

void ECSGame::ReloadScene() {
    m_World.Clear();
    
    // Re-create Camera Entity
    m_CameraEntity = m_World.CreateEntity();
    auto& camComp = m_World.Add<ECS::CameraComponent>(m_CameraEntity);
    if (m_Camera) {
        camComp.camera = *m_Camera;
    }
    camComp.active = true;

    OnECSCreate();
}

void ECSGame::OnUpdate(double deltaTime) {
    float dt = static_cast<float>(deltaTime);

    Renderer::BeginFrame();
    Renderer::Clear();

    if (m_Paused) {
        OnECSRender(); // можно отрисовать UI/плашку паузы
        Renderer::EndFrame();
        return;
    }

    // Обновление пользовательской логики до систем
    OnECSUpdate(dt);

    // Обновляем все системы
    m_Scheduler.UpdateAll(m_World, dt);

    // Sync Component -> Camera (for GetCamera() and Renderer::SetCamera in OnResize)
    if (m_World.IsAlive(m_CameraEntity)) {
        if (auto* cam = m_World.Get<ECS::CameraComponent>(m_CameraEntity)) {
            *m_Camera = cam->camera;
        }
    }

    OnGameRender();
}

void ECSGame::OnGameRender() {
    // Отрисовка встроенными системами уже была выполнена в UpdateAll (через SpriteRenderSystem)
    if (m_DebugPhysics && m_PhysicsSystem) {
        m_PhysicsSystem->DrawDebug(m_World);
    }
    OnECSRender();
    Renderer::EndFrame();
}

void ECSGame::OnResize(int width, int height) {
    if (m_Camera && width > 0 && height > 0) {
        m_Camera->SetViewportSize(static_cast<float>(width), static_cast<float>(height));
        Renderer::SetCamera(*m_Camera);

        // Sync Camera -> Component
        if (m_World.IsAlive(m_CameraEntity)) {
            if (auto* cam = m_World.Get<ECS::CameraComponent>(m_CameraEntity)) {
                cam->camera = *m_Camera;
            }
        }
    }
}

void ECSGame::OnShutdown() {
    // Очистка мира при завершении
    m_World.Clear();
}

void ECSGame::OnFocusChanged(bool focused) {
    m_Paused = !focused;
}

void ECSGame::SetCameraTarget(ECS::Entity entity, float smoothness) {
    m_CameraTarget = entity;
    m_CameraSmooth = smoothness;
    if (m_World.IsAlive(entity)) {
        auto& follow = m_World.Add<ECS::CameraFollowComponent>(entity);
        follow.smoothness = smoothness;
    }
}

void ECSGame::OnFixedUpdate(double fixedDeltaTime) {
    if (!m_Paused) {
        float dt = static_cast<float>(fixedDeltaTime);
        m_Scheduler.FixedUpdateAll(m_World, dt);
        m_PhysicsWorld.Step(dt);
    }
}

} // namespace SAGE
