#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Core/ECS.h"
#include "SAGE/Physics/PhysicsWorld.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace SAGE {

// Forward declarations
class Event;

// Scene transition context
struct TransitionContext {
    std::string fromScene;
    std::unordered_map<std::string, std::string> parameters;
};

// Base Scene class for game scenes
class Scene {
public:
    explicit Scene(const std::string& name);
    virtual ~Scene() = default;

    // Scene lifecycle
    virtual void OnEnter(const TransitionContext& context) = 0;
    virtual void OnExit() = 0;
    virtual void OnUpdate(float deltaTime); // Made virtual with default impl
    virtual void OnFixedUpdate(float fixedDeltaTime);
    virtual void OnRender() = 0;
    virtual void OnEvent([[maybe_unused]] Event& event) {}

    // Scene identification
    const std::string& GetName() const { return m_Name; }
    
    // Pause/Resume support
    virtual void OnPause() {}
    virtual void OnResume() {}

    // ECS Access
    ECS::Registry& GetRegistry() { return m_Registry; }
    ECS::SystemScheduler& GetScheduler() { return m_Scheduler; }

    // Entity Creation
    ECS::Entity CreateEntity() { return m_Registry.CreateEntity(); }
    void DestroyEntity(ECS::Entity entity) { m_Registry.DestroyEntity(entity); }

    // Instantiate Prefab
    ECS::Entity Instantiate(class Prefab* prefab);

    // Physics Access
    Physics::PhysicsWorld& GetPhysicsWorld() { return m_PhysicsWorld; }
    
    Physics::PhysicsWorld::RayCastHit RayCast(const Vector2& start, const Vector2& end) {
        return m_PhysicsWorld.RayCast(start, end);
    }

protected:
    std::string m_Name;
    bool m_IsPaused = false;

    ECS::Registry m_Registry;
    ECS::SystemScheduler m_Scheduler;
    Physics::PhysicsWorld m_PhysicsWorld;
};

} // namespace SAGE
