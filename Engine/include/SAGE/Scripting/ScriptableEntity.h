#pragma once

#include "SAGE/Core/GameObject.h"

namespace SAGE {

namespace ECS {
    class NativeScriptSystem;
}

class ScriptableEntity {
public:
    virtual ~ScriptableEntity() = default;

    template<typename T>
    T& GetComponent() {
        return m_GameObject.GetComponent<T>();
    }

    template<typename T>
    bool HasComponent() const {
        return m_GameObject.HasComponent<T>();
    }

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        return m_GameObject.AddComponent<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent() {
        m_GameObject.RemoveComponent<T>();
    }

    virtual void OnCreate() {}
    virtual void OnUpdate(float dt) { (void)dt; }
    virtual void OnDestroy() {}

protected:
    GameObject m_GameObject;
    friend class ECS::NativeScriptSystem;
};

} // namespace SAGE
