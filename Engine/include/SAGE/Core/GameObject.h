#pragma once

#include "SAGE/Core/Scene.h"
#include "SAGE/Core/ECS.h"

namespace SAGE {

class GameObject {
public:
    GameObject() = default;
    GameObject(ECS::Entity entity, Scene* scene)
        : m_Entity(entity), m_Scene(scene) {}

    bool IsValid() const {
        return m_Scene && m_Entity != ECS::kInvalidEntity && m_Scene->GetRegistry().IsAlive(m_Entity);
    }

    ECS::Entity GetEntity() const { return m_Entity; }

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        return m_Scene->GetRegistry().Add<T>(m_Entity, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent() {
        return *m_Scene->GetRegistry().Get<T>(m_Entity);
    }

    template<typename T>
    const T& GetComponent() const {
        return *m_Scene->GetRegistry().Get<T>(m_Entity);
    }

    template<typename T>
    bool HasComponent() const {
        return m_Scene->GetRegistry().Has<T>(m_Entity);
    }

    template<typename T>
    void RemoveComponent() {
        m_Scene->GetRegistry().Remove<T>(m_Entity);
    }

    void Destroy() {
        if (IsValid()) {
            m_Scene->DestroyEntity(m_Entity);
            m_Entity = ECS::kInvalidEntity;
        }
    }

    bool operator==(const GameObject& other) const {
        return m_Entity == other.m_Entity && m_Scene == other.m_Scene;
    }

    bool operator!=(const GameObject& other) const {
        return !(*this == other);
    }

private:
    ECS::Entity m_Entity = ECS::kInvalidEntity;
    Scene* m_Scene = nullptr;
};

} // namespace SAGE
