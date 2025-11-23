#include "SAGE/Core/Scene.h"
#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Core/Prefab.h"

namespace SAGE {

Scene::Scene(const std::string& name)
    : m_Name(name)
{
    // Register core systems
    m_Scheduler.AddSystem<ECS::NativeScriptSystem>(this);
    m_Scheduler.AddSystem<ECS::CameraSystem>();
}

ECS::Entity Scene::Instantiate(Prefab* prefab) {
    if (prefab) {
        return prefab->Instantiate(this);
    }
    return ECS::kInvalidEntity;
}

void Scene::OnUpdate(float deltaTime) {
    if (!m_IsPaused) {
        m_Scheduler.UpdateAll(m_Registry, deltaTime);
    }
}

void Scene::OnFixedUpdate(float fixedDeltaTime) {
    if (!m_IsPaused) {
        m_PhysicsWorld.Step(fixedDeltaTime);
    }
}

} // namespace SAGE
