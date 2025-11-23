#include "SAGE/Core/SceneManager.h"
#include "SAGE/Log.h"

namespace SAGE {

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name) {
    if (name.empty()) {
        SAGE_ERROR("SceneManager: Cannot create scene with empty name");
        return nullptr;
    }
    
    auto it = m_SceneFactories.find(name);
    if (it == m_SceneFactories.end()) {
        SAGE_ERROR("SceneManager: Scene '{}' not registered", name);
        return nullptr;
    }
    
    auto scene = it->second();
    if (!scene) {
        SAGE_ERROR("SceneManager: Factory for scene '{}' returned null", name);
    }
    return scene;
}

void SceneManager::SwitchToScene(const std::string& name, const TransitionContext& context) {
    m_PendingChange.type = SwitchType::Switch;
    m_PendingChange.name = name;
    m_PendingChange.context = context;
}

void SceneManager::PushScene(const std::string& name, const TransitionContext& context) {
    m_PendingChange.type = SwitchType::Push;
    m_PendingChange.name = name;
    m_PendingChange.context = context;
}

void SceneManager::PopScene() {
    m_PendingChange.type = SwitchType::Pop;
}

void SceneManager::ReloadScene() {
    m_PendingChange.type = SwitchType::Reload;
}

void SceneManager::Update(float deltaTime) {
    ApplyPendingChange();

    if (m_CurrentScene) {
        m_CurrentScene->OnUpdate(deltaTime);
    }
}

void SceneManager::ApplyPendingChange() {
    if (m_PendingChange.type == SwitchType::None) {
        return;
    }

    switch (m_PendingChange.type) {
        case SwitchType::Switch: {
            auto newScene = CreateScene(m_PendingChange.name);
            if (!newScene) break;

            if (m_CurrentScene) {
                SAGE_INFO("SceneManager: Exiting scene '{}'", m_CurrentScene->GetName());
                m_CurrentScene->OnExit();
            }

            while (!m_SceneStack.empty()) {
                m_SceneStack.pop();
            }

            m_CurrentScene = newScene;
            SAGE_INFO("SceneManager: Entering scene '{}'", m_CurrentScene->GetName());
            m_CurrentScene->OnEnter(m_PendingChange.context);
            break;
        }
        case SwitchType::Push: {
            auto newScene = CreateScene(m_PendingChange.name);
            if (!newScene) break;

            if (m_CurrentScene) {
                SAGE_INFO("SceneManager: Pausing scene '{}'", m_CurrentScene->GetName());
                m_CurrentScene->OnPause();
                m_SceneStack.push(m_CurrentScene);
            }

            m_CurrentScene = newScene;
            SAGE_INFO("SceneManager: Pushing scene '{}'", m_CurrentScene->GetName());
            m_CurrentScene->OnEnter(m_PendingChange.context);
            break;
        }
        case SwitchType::Pop: {
            if (m_SceneStack.empty()) {
                SAGE_WARNING("SceneManager: Cannot pop scene - stack is empty");
                break;
            }

            if (m_CurrentScene) {
                SAGE_INFO("SceneManager: Exiting scene '{}'", m_CurrentScene->GetName());
                m_CurrentScene->OnExit();
            }

            m_CurrentScene = m_SceneStack.top();
            m_SceneStack.pop();

            SAGE_INFO("SceneManager: Resuming scene '{}'", m_CurrentScene->GetName());
            m_CurrentScene->OnResume();
            break;
        }
        case SwitchType::Reload: {
            if (!m_CurrentScene) break;
            
            std::string sceneName = m_CurrentScene->GetName();
            TransitionContext ctx;
            ctx.fromScene = sceneName;

            // Clear stack
            while (!m_SceneStack.empty()) {
                m_SceneStack.pop();
            }
            
            // Exit and destroy current
            m_CurrentScene->OnExit();
            m_CurrentScene.reset();

            // Recreate
            auto newScene = CreateScene(sceneName);
            if (newScene) {
                m_CurrentScene = newScene;
                SAGE_INFO("SceneManager: Reloading scene '{}'", m_CurrentScene->GetName());
                m_CurrentScene->OnEnter(ctx);
            }
            break;
        }
        case SwitchType::None:
            break;
    }

    m_PendingChange.type = SwitchType::None;
}

void SceneManager::FixedUpdate(float fixedDeltaTime) {
    if (m_CurrentScene) {
        m_CurrentScene->OnFixedUpdate(fixedDeltaTime);
    }
}

void SceneManager::Render() {
    if (m_CurrentScene) {
        m_CurrentScene->OnRender();
    }
}

void SceneManager::HandleEvent(Event& event) {
    if (m_CurrentScene) {
        m_CurrentScene->OnEvent(event);
    }
}

} // namespace SAGE
