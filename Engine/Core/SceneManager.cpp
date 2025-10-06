#include "SceneManager.h"

#include "Logger.h"
#include "Scene.h"
#include "SceneStack.h"

namespace SAGE {

    void SceneManager::RegisterScene(const std::string& name, SceneFactory factory) {
        if (name.empty() || !factory) {
              SAGE_WARNING("Attempted to register invalid scene '{}'.", name);
            return;
        }
        m_Factories[name] = std::move(factory);
    }

    void SceneManager::UnregisterScene(const std::string& name) {
        m_Factories.erase(name);
        m_SavedStates.erase(name);
    }

    bool SceneManager::HasScene(const std::string& name) const {
        return m_Factories.find(name) != m_Factories.end();
    }

    void SceneManager::QueuePush(const std::string& name, bool restoreState) {
        m_PendingTransitions.push({ TransitionType::Push, name, restoreState, false });
    }

    void SceneManager::QueueReplace(const std::string& name, bool restoreState, bool saveOutgoingState) {
        m_PendingTransitions.push({ TransitionType::Replace, name, restoreState, saveOutgoingState });
    }

    void SceneManager::QueuePop(bool saveOutgoingState) {
        m_PendingTransitions.push({ TransitionType::Pop, std::string{}, false, saveOutgoingState });
    }

    void SceneManager::SaveState(Scene& scene) {
        SceneState state;
        scene.SaveState(state);
        if (state.Empty()) {
            m_SavedStates.erase(scene.GetName());
            return;
        }
        m_SavedStates[scene.GetName()] = std::move(state);
    }

    void SceneManager::ForgetState(const std::string& name) {
        m_SavedStates.erase(name);
    }

    bool SceneManager::HasSavedState(const std::string& name) const {
        return m_SavedStates.find(name) != m_SavedStates.end();
    }

    const SceneState* SceneManager::GetSavedState(const std::string& name) const {
        auto it = m_SavedStates.find(name);
        if (it == m_SavedStates.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void SceneManager::ClearStates() {
        m_SavedStates.clear();
    }

    void SceneManager::ProcessTransitions(SceneStack& stack) {
        while (!m_PendingTransitions.empty()) {
            PendingTransition transition = m_PendingTransitions.front();
            m_PendingTransitions.pop();

            switch (transition.Type) {
            case TransitionType::Push:
                ApplyPush(stack, transition);
                break;
            case TransitionType::Replace:
                ApplyReplace(stack, transition);
                break;
            case TransitionType::Pop:
                ApplyPop(stack, transition);
                break;
            }
        }
    }

    void SceneManager::SetTransitionCallback(TransitionCallback callback) {
        m_OnTransition = std::move(callback);
    }

    Scope<Scene> SceneManager::CreateScene(const std::string& name) {
        auto it = m_Factories.find(name);
        if (it == m_Factories.end()) {
              SAGE_WARNING("Scene '{}' not registered.", name);
            return nullptr;
        }

        auto scene = it->second();
        if (!scene) {
              SAGE_WARNING("Scene factory for '{}' returned null.", name);
            return nullptr;
        }
        scene->SetName(name);
        return scene;
    }

    void SceneManager::ApplyPush(SceneStack& stack, const PendingTransition& transition) {
        auto scene = CreateScene(transition.Target);
        if (!scene) {
            return;
        }

        RestoreSceneState(*scene, transition.RestoreState);
        stack.PushScene(std::move(scene));

        if (m_OnTransition) {
            m_OnTransition(transition.Target);
        }
    }

    void SceneManager::ApplyReplace(SceneStack& stack, const PendingTransition& transition) {
        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        if (!stack.Empty()) {
            stack.PopTopScene();
        }

        auto scene = CreateScene(transition.Target);
        if (!scene) {
            return;
        }

        RestoreSceneState(*scene, transition.RestoreState);
        stack.PushScene(std::move(scene));

        if (m_OnTransition) {
            m_OnTransition(transition.Target);
        }
    }

    void SceneManager::ApplyPop(SceneStack& stack, const PendingTransition& transition) {
        (void)transition;
        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        if (!stack.Empty()) {
            stack.PopTopScene();
        }
    }

    void SceneManager::RestoreSceneState(Scene& scene, bool restoreState) {
        if (!restoreState) {
            return;
        }

        auto it = m_SavedStates.find(scene.GetName());
        if (it == m_SavedStates.end()) {
            return;
        }

        scene.LoadState(it->second);
    }

    void SceneManager::MaybeSaveOutgoing(Scene* outgoing, bool saveOutgoingState) {
        if (!outgoing || !saveOutgoingState) {
            return;
        }
        SaveState(*outgoing);
    }

} // namespace SAGE
