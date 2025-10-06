#pragma once

#include "Core.h"
#include "SceneState.h"

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

namespace SAGE {

    class Scene;
    class SceneStack;

    class SceneManager {
    public:
        enum class TransitionType {
            Push,
            Replace,
            Pop
        };

        using SceneFactory = std::function<Scope<Scene>()>;
        using TransitionCallback = std::function<void(const std::string&)>;

        SceneManager() = default;
        ~SceneManager() = default;

        void RegisterScene(const std::string& name, SceneFactory factory);
        void UnregisterScene(const std::string& name);
        [[nodiscard]] bool HasScene(const std::string& name) const;

        void QueuePush(const std::string& name, bool restoreState = true);
        void QueueReplace(const std::string& name, bool restoreState = true, bool saveOutgoingState = true);
        void QueuePop(bool saveOutgoingState = false);

        void SaveState(Scene& scene);
        void ForgetState(const std::string& name);
        [[nodiscard]] bool HasSavedState(const std::string& name) const;
        [[nodiscard]] const SceneState* GetSavedState(const std::string& name) const;
        void ClearStates();

        void ProcessTransitions(SceneStack& stack);

        void SetTransitionCallback(TransitionCallback callback);

    private:
        struct PendingTransition {
            TransitionType Type = TransitionType::Push;
            std::string Target;
            bool RestoreState = false;
            bool SaveOutgoingState = false;
        };

        Scope<Scene> CreateScene(const std::string& name);

        void ApplyPush(SceneStack& stack, const PendingTransition& transition);
        void ApplyReplace(SceneStack& stack, const PendingTransition& transition);
        void ApplyPop(SceneStack& stack, const PendingTransition& transition);

        void RestoreSceneState(Scene& scene, bool restoreState);
        void MaybeSaveOutgoing(Scene* outgoing, bool saveOutgoingState);

        std::queue<PendingTransition> m_PendingTransitions;
        std::unordered_map<std::string, SceneFactory> m_Factories;
        std::unordered_map<std::string, SceneState> m_SavedStates;
        TransitionCallback m_OnTransition;
    };

} // namespace SAGE
