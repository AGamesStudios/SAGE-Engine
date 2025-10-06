#include "SceneStack.h"

#include "Scene.h"
#include "Event.h"

#include <string>

namespace SAGE {

    SceneStack::~SceneStack() {
        Clear();
    }

    void SceneStack::PushScene(Scope<Scene> scene) {
        if (!scene) {
            return;
        }
        if (!m_Scenes.empty()) {
            if (auto* current = m_Scenes.back().get()) {
                current->OnPause();
            }
        }
        scene->OnAttach();
        m_Scenes.emplace_back(std::move(scene));
        if (auto* top = m_Scenes.back().get()) {
            top->OnResume();
        }
    }

    void SceneStack::PopScene(Scene* scene) {
        if (!scene) {
            return;
        }

        for (auto it = m_Scenes.begin(); it != m_Scenes.end(); ++it) {
            if (it->get() == scene) {
                (*it)->OnDetach();
                m_Scenes.erase(it);
                if (!m_Scenes.empty()) {
                    if (auto* top = m_Scenes.back().get()) {
                        top->OnResume();
                    }
                }
                break;
            }
        }
    }

    void SceneStack::PopTopScene() {
        if (m_Scenes.empty()) {
            return;
        }
        m_Scenes.back()->OnDetach();
        m_Scenes.pop_back();
        if (!m_Scenes.empty()) {
            if (auto* top = m_Scenes.back().get()) {
                top->OnResume();
            }
        }
    }

    void SceneStack::Clear() {
        for (auto& scene : m_Scenes) {
            if (scene) {
                scene->OnDetach();
            }
        }
        m_Scenes.clear();
    }

    Scene* SceneStack::GetTopScene() {
        if (m_Scenes.empty()) {
            return nullptr;
        }
        return m_Scenes.back().get();
    }

    const Scene* SceneStack::GetTopScene() const {
        if (m_Scenes.empty()) {
            return nullptr;
        }
        return m_Scenes.back().get();
    }

    Scene* SceneStack::FindScene(const std::string& name) {
        for (auto& scene : m_Scenes) {
            if (scene && scene->GetName() == name) {
                return scene.get();
            }
        }
        return nullptr;
    }

    void SceneStack::OnUpdate(float deltaTime) {
        for (auto& scene : m_Scenes) {
            if (scene) {
                scene->OnUpdate(deltaTime);
            }
        }
    }

    void SceneStack::OnRender() {
        for (auto& scene : m_Scenes) {
            if (scene) {
                scene->OnRender();
            }
        }
    }

    void SceneStack::OnEvent(Event& event) {
        for (auto it = m_Scenes.rbegin(); it != m_Scenes.rend(); ++it) {
            auto& scene = *it;
            if (!scene) {
                continue;
            }
            scene->OnEvent(event);
            if (event.Handled) {
                break;
            }
        }
    }

} // namespace SAGE
