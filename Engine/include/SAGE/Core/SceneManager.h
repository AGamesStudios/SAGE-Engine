#pragma once

#include "SAGE/Core/Scene.h"
#include <memory>
#include <unordered_map>
#include <stack>
#include <functional>

namespace SAGE {

// Scene Manager - handles scene switching and lifecycle
class SceneManager {
public:
    static SceneManager& Get();

    // Scene registration
    template<typename T, typename... Args>
    void RegisterScene(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");
        m_SceneFactories[name] = [args...]() -> std::shared_ptr<Scene> {
            return std::make_shared<T>(std::forward<Args>(args)...);
        };
    }

    // Scene transitions
    void SwitchToScene(const std::string& name, const TransitionContext& context = {});
    void PushScene(const std::string& name, const TransitionContext& context = {});
    void PopScene();
    void ReloadScene();
    
    // Update current scene
    void Update(float deltaTime);
    void FixedUpdate(float fixedDeltaTime);
    void Render();
    void HandleEvent(Event& event);

    // Query
    Scene* GetCurrentScene() const { return m_CurrentScene.get(); }
    
    template<typename T>
    T* GetCurrentScene() const {
        return dynamic_cast<T*>(m_CurrentScene.get());
    }

    bool HasScene(const std::string& name) const { return m_SceneFactories.count(name) > 0; }
    size_t GetSceneStackSize() const { return m_SceneStack.size(); }

private:
    SceneManager() = default;
    ~SceneManager() = default;
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::shared_ptr<Scene> CreateScene(const std::string& name);
    void ApplyPendingChange();

    using SceneFactory = std::function<std::shared_ptr<Scene>()>;
    std::unordered_map<std::string, SceneFactory> m_SceneFactories;
    
    std::shared_ptr<Scene> m_CurrentScene;
    std::stack<std::shared_ptr<Scene>> m_SceneStack;  // For pause menus, etc.

    enum class SwitchType { None, Switch, Push, Pop, Reload };
    struct PendingChange {
        SwitchType type = SwitchType::None;
        std::string name;
        TransitionContext context;
    };
    PendingChange m_PendingChange;
};

} // namespace SAGE
