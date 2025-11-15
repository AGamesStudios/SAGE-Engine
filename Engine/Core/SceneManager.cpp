#include "SceneManager.h"

#include "Logger.h"
#include "Scene.h"
#include "SceneStack.h"

#include <utility>

namespace SAGE {


    void SceneManager::RegisterScene(const std::string& name, SceneFactory factory) {
        if (name.empty() || !factory) {
            SAGE_WARNING("Attempted to register invalid scene '{}'.", name);
            return;
        }

        std::unique_lock lock(m_FactoryMutex);
        m_Factories[name] = std::move(factory);
    }

    void SceneManager::UnregisterScene(const std::string& name) {
        {
            std::unique_lock lock(m_FactoryMutex);
            m_Factories.erase(name);
        }
        std::scoped_lock stateLock(m_StateMutex);
        m_SavedStates.erase(name);
    }

    bool SceneManager::HasScene(const std::string& name) const {
        std::shared_lock lock(m_FactoryMutex);
        return m_Factories.find(name) != m_Factories.end();
    }

    void SceneManager::QueuePush(const std::string& name,
                                 SceneParameters params,
                                 bool restoreState,
                                 bool saveOutgoingState) {
        PendingTransition transition;
        transition.Type = TransitionType::Push;
        transition.Target = name;
        transition.Params = std::move(params);
        transition.RestoreState = restoreState;
        transition.SaveOutgoingState = saveOutgoingState;

        std::scoped_lock lock(m_TransitionMutex);
        m_PendingTransitions.push(std::move(transition));
    }

    void SceneManager::QueueReplace(const std::string& name,
                                    SceneParameters params,
                                    bool restoreState,
                                    bool saveOutgoingState) {
        PendingTransition transition;
        transition.Type = TransitionType::Replace;
        transition.Target = name;
        transition.Params = std::move(params);
        transition.RestoreState = restoreState;
        transition.SaveOutgoingState = saveOutgoingState;

        std::scoped_lock lock(m_TransitionMutex);
        m_PendingTransitions.push(std::move(transition));
    }

    void SceneManager::QueueSwap(const std::string& name,
                                 SceneParameters params,
                                 bool restoreState,
                                 bool saveOutgoingState) {
        PendingTransition transition;
        transition.Type = TransitionType::Swap;
        transition.Target = name;
        transition.Params = std::move(params);
        transition.RestoreState = restoreState;
        transition.SaveOutgoingState = saveOutgoingState;

        std::scoped_lock lock(m_TransitionMutex);
        m_PendingTransitions.push(std::move(transition));
    }

    void SceneManager::QueuePop(bool saveOutgoingState,
                                SceneParameters resumeParams,
                                bool restoreResumeState) {
        PendingTransition transition;
        transition.Type = TransitionType::Pop;
        transition.SaveOutgoingState = saveOutgoingState;
        transition.ResumeParams = std::move(resumeParams);
        transition.RestoreResumeState = restoreResumeState;

        std::scoped_lock lock(m_TransitionMutex);
        m_PendingTransitions.push(std::move(transition));
    }

    void SceneManager::SaveState(Scene& scene) {
        if (!scene.IsPersistent()) {
            return;
        }

        SceneState state;
        scene.SaveState(state);

        std::scoped_lock lock(m_StateMutex);
        if (state.Empty()) {
            m_SavedStates.erase(scene.GetName());
            return;
        }
        m_SavedStates[scene.GetName()] = std::move(state);
    }

    void SceneManager::ForgetState(const std::string& name) {
        std::scoped_lock lock(m_StateMutex);
        m_SavedStates.erase(name);
    }

    bool SceneManager::HasSavedState(const std::string& name) const {
        std::scoped_lock lock(m_StateMutex);
        return m_SavedStates.find(name) != m_SavedStates.end();
    }

    std::optional<SceneState> SceneManager::GetSavedState(const std::string& name) const {
        std::scoped_lock lock(m_StateMutex);
        auto it = m_SavedStates.find(name);
        if (it == m_SavedStates.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    void SceneManager::ClearStates() {
        std::scoped_lock lock(m_StateMutex);
        m_SavedStates.clear();
    }

    void SceneManager::ProcessTransitions(SceneStack& stack) {
        std::queue<PendingTransition> local;
        {
            std::scoped_lock lock(m_TransitionMutex);
            std::swap(local, m_PendingTransitions);
        }

        while (!local.empty()) {
            PendingTransition transition = std::move(local.front());
            local.pop();

            switch (transition.Type) {
            case TransitionType::Push:
                ApplyPush(stack, transition);
                break;
            case TransitionType::Replace:
                ApplyReplace(stack, transition);
                break;
            case TransitionType::Swap:
                ApplySwap(stack, transition);
                break;
            case TransitionType::Pop:
                ApplyPop(stack, transition);
                break;
            }
        }
    }

    void SceneManager::SetTransitionCallback(TransitionCallback callback) {
        std::scoped_lock lock(m_CallbackMutex);
        m_OnTransition = std::move(callback);
    }

    Scope<Scene> SceneManager::CreateScene(const std::string& name) {
        SceneFactory factory;
        {
            std::shared_lock lock(m_FactoryMutex);
            auto it = m_Factories.find(name);
            if (it == m_Factories.end()) {
                SAGE_WARNING("Scene '{}' not registered.", name);
                return nullptr;
            }
            factory = it->second;
        }

        if (!factory) {
            SAGE_WARNING("Scene factory for '{}' is null.", name);
            return nullptr;
        }

        auto scene = factory();
        if (!scene) {
            SAGE_WARNING("Scene factory for '{}' returned null.", name);
            return nullptr;
        }

        scene->SetName(name);
        return scene;
    }

    void SceneManager::ApplyPush(SceneStack& stack, PendingTransition& transition) {
        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        auto scene = CreateScene(transition.Target);
        if (!scene) {
            return;
        }

        bool restored = RestoreSceneState(*scene, transition.RestoreState);
        stack.PushScene(std::move(scene), std::move(transition.Params), restored);

        TransitionCallback callback;
        {
            std::scoped_lock lock(m_CallbackMutex);
            callback = m_OnTransition;
        }
        if (callback && !transition.Target.empty()) {
            callback(transition.Target);
        }
    }

    void SceneManager::ApplyReplace(SceneStack& stack, PendingTransition& transition) {
        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        auto scene = CreateScene(transition.Target);
        if (!scene) {
            return;
        }

        bool restored = RestoreSceneState(*scene, transition.RestoreState);
        if (stack.Empty()) {
            stack.PushScene(std::move(scene), std::move(transition.Params), restored);
        } else {
            stack.ReplaceTopScene(std::move(scene), std::move(transition.Params), restored);
        }

        TransitionCallback callback;
        {
            std::scoped_lock lock(m_CallbackMutex);
            callback = m_OnTransition;
        }
        if (callback && !transition.Target.empty()) {
            callback(transition.Target);
        }
    }

    void SceneManager::ApplySwap(SceneStack& stack, PendingTransition& transition) {
        if (stack.Empty()) {
            ApplyPush(stack, transition);
            return;
        }

        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        auto scene = CreateScene(transition.Target);
        if (!scene) {
            return;
        }

        bool restored = RestoreSceneState(*scene, transition.RestoreState);
        stack.ReplaceTopScene(std::move(scene), std::move(transition.Params), restored);

        TransitionCallback callback;
        {
            std::scoped_lock lock(m_CallbackMutex);
            callback = m_OnTransition;
        }
        if (callback && !transition.Target.empty()) {
            callback(transition.Target);
        }
    }

    void SceneManager::ApplyPop(SceneStack& stack, PendingTransition& transition) {
        Scene* outgoing = stack.GetTopScene();
        MaybeSaveOutgoing(outgoing, transition.SaveOutgoingState);

        Scene* resumeTarget = stack.GetSceneBelowTop();
        bool resumeRestored = false;
        if (resumeTarget) {
            resumeRestored = RestoreSceneState(*resumeTarget, transition.RestoreResumeState);
        }

        if (!stack.Empty()) {
            stack.PopTopScene(std::move(transition.ResumeParams), resumeRestored);
        }

        TransitionCallback callback;
        {
            std::scoped_lock lock(m_CallbackMutex);
            callback = m_OnTransition;
        }
        if (callback) {
            Scene* top = stack.GetTopScene();
            callback(top ? top->GetName() : std::string{});
        }
    }

    bool SceneManager::RestoreSceneState(Scene& scene, bool restoreState) {
        if (!restoreState && !scene.IsPersistent()) {
            return false;
        }

        SceneState stateCopy;
        {
            std::scoped_lock lock(m_StateMutex);
            auto it = m_SavedStates.find(scene.GetName());
            if (it == m_SavedStates.end()) {
                return false;
            }
            stateCopy = it->second;
        }

        scene.LoadState(stateCopy);
        return true;
    }

    void SceneManager::MaybeSaveOutgoing(Scene* outgoing, bool saveOutgoingState) {
        if (!outgoing) {
            return;
        }
        if (!saveOutgoingState && !outgoing->IsPersistent()) {
            return;
        }
        SaveState(*outgoing);
    }

} // namespace SAGE
