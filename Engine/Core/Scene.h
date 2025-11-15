#pragma once

#include "SceneState.h"
#include "ECS/ECSContext.h"

#include <string>
#include <utility>
#include <atomic>

namespace SAGE {

    class Event;
    class Scene;

    using SceneID = size_t;

    class Scene {
    public:
        explicit Scene(std::string name = "Scene")
            : m_Name(std::move(name)), m_SceneID(GenerateSceneID()) {}
        virtual ~Scene() {
            Clear();
        }

        const std::string& GetName() const { return m_Name; }
        void SetName(std::string name) { m_Name = std::move(name); }
        SceneID GetID() const { return m_SceneID; }

        struct TransitionContext {
            const SceneParameters& Parameters;
            Scene* PreviousScene = nullptr;
            bool StateRestored = false;
        };

        virtual void OnPause() {}
        virtual void OnResume(const TransitionContext& /*context*/) {}

        virtual void OnEnter(const TransitionContext& /*context*/) {}
        virtual void OnExit() {}
        virtual void Clear() {
            m_ECS.Shutdown();
        }
    virtual void OnUpdate(float /*deltaTime*/) {}
        virtual void OnFixedUpdate(float /*fixedDeltaTime*/) {}
        virtual void OnRender() {}
        virtual void OnEvent(Event& /*event*/) {}

        virtual bool IsPersistent() const { return false; }
        virtual void SaveState(SceneState& /*outState*/) const {}
        virtual void LoadState(const SceneState& /*state*/) {}

        // ===== ECS интеграция =====
    public:
        ECS::ECSContext& GetECS() { return m_ECS; }
        const ECS::ECSContext& GetECS() const { return m_ECS; }
        
        ECS::Registry& GetRegistry() { return m_ECS.GetRegistry(); }
        const ECS::Registry& GetRegistry() const { return m_ECS.GetRegistry(); }

    protected:
        std::string m_Name;
        SceneID m_SceneID;
        ECS::ECSContext m_ECS; // Контекст ECS для этой сцены

    private:
        static SceneID GenerateSceneID() {
            static std::atomic<SceneID> s_NextID{1};
            return s_NextID.fetch_add(1);
        }
    };

} // namespace SAGE
