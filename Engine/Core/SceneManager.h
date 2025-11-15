#pragma once

#include "Core.h"
#include "SceneState.h"

#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
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
            Swap,
            Pop
        };

        using SceneFactory = std::function<Scope<Scene>()>;
        using TransitionCallback = std::function<void(const std::string&)>;

        SceneManager() = default;
        ~SceneManager() = default;

        void RegisterScene(const std::string& name, SceneFactory factory);
        void UnregisterScene(const std::string& name);
        [[nodiscard]] bool HasScene(const std::string& name) const;

        void QueuePush(const std::string& name,
                       SceneParameters params = SceneParameters{},
                       bool restoreState = true,
                       bool saveOutgoingState = false);
        void QueueReplace(const std::string& name,
                          SceneParameters params = SceneParameters{},
                          bool restoreState = true,
                          bool saveOutgoingState = true);
        void QueueSwap(const std::string& name,
                       SceneParameters params = SceneParameters{},
                       bool restoreState = true,
                       bool saveOutgoingState = true);
        void QueuePop(bool saveOutgoingState = false,
                      SceneParameters resumeParams = SceneParameters{},
                      bool restoreResumeState = false);

        void SaveState(Scene& scene);
        void ForgetState(const std::string& name);
        [[nodiscard]] bool HasSavedState(const std::string& name) const;
        [[nodiscard]] std::optional<SceneState> GetSavedState(const std::string& name) const;
        void ClearStates();

        void ProcessTransitions(SceneStack& stack);

        void SetTransitionCallback(TransitionCallback callback);

    private:
        struct PendingTransition {
            TransitionType Type = TransitionType::Push;
            std::string Target;
            SceneParameters Params;
            SceneParameters ResumeParams;
            bool RestoreState = false;
            bool SaveOutgoingState = false;
            bool RestoreResumeState = false;
        };

        Scope<Scene> CreateScene(const std::string& name);

        void ApplyPush(SceneStack& stack, PendingTransition& transition);
        void ApplyReplace(SceneStack& stack, PendingTransition& transition);
        void ApplySwap(SceneStack& stack, PendingTransition& transition);
        void ApplyPop(SceneStack& stack, PendingTransition& transition);

        bool RestoreSceneState(Scene& scene, bool restoreState);
        void MaybeSaveOutgoing(Scene* outgoing, bool saveOutgoingState);

        std::queue<PendingTransition> m_PendingTransitions;
        std::unordered_map<std::string, SceneFactory> m_Factories;
        std::unordered_map<std::string, SceneState> m_SavedStates;
        TransitionCallback m_OnTransition;
        /**
         * @brief Thread-safety mutex for scene factory (mutable for const methods)
         * 
         * Protects scene factory registration and access.
         * Allows concurrent scene creation while preventing factory modification races.
         */
        mutable std::shared_mutex m_FactoryMutex;
        mutable std::mutex m_StateMutex;
        mutable std::mutex m_TransitionMutex;
        mutable std::mutex m_CallbackMutex;
    };

} // namespace SAGE
