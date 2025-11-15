#pragma once

#include "Core.h"
#include "Scene.h"

#include <mutex>
#include <string>
#include <vector>

namespace SAGE {

    class Event;

    class SceneStack {
    public:
        SceneStack() = default;
        ~SceneStack();

        SceneStack(const SceneStack&) = delete;
        SceneStack& operator=(const SceneStack&) = delete;
        SceneStack(SceneStack&&) noexcept = default;
        SceneStack& operator=(SceneStack&&) noexcept = default;

        void PushScene(Scope<Scene> scene,
                       SceneParameters params = SceneParameters{},
                       bool stateRestored = false);
        void PopScene(Scene* scene,
                      SceneParameters resumeParams = SceneParameters{},
                      bool stateRestored = false);
        void PopTopScene(SceneParameters resumeParams = SceneParameters{},
                         bool stateRestored = false);
        void ReplaceTopScene(Scope<Scene> scene,
                              SceneParameters params = SceneParameters{},
                              bool stateRestored = false);
        void Clear();

        void OnUpdate(float deltaTime);
        void OnFixedUpdate(float fixedDeltaTime);
        void OnRender();
        void OnEvent(Event& event);

        bool Empty() const;
        std::size_t Size() const;
        Scene* GetTopScene();
        const Scene* GetTopScene() const;
        Scene* GetSceneBelowTop();
        const Scene* GetSceneBelowTop() const;
        Scene* FindScene(const std::string& name);
        Scene* FindSceneByID(SceneID id);

        template<typename SceneT, typename... Args>
        SceneT& EmplaceScene(Args&&... args) {
            static_assert(std::is_base_of_v<Scene, SceneT>, "SceneT must derive from Scene");
            auto scene = CreateScope<SceneT>(std::forward<Args>(args)...);
            SceneT& ref = *scene;
            PushScene(std::move(scene));
            return ref;
        }

    private:
        Scene* GetTopSceneUnsafe() const;
        struct SceneEntry {
            Scope<Scene> Instance;
            SceneParameters LastEnterParams;
            SceneParameters LastResumeParams;
            bool LastStateRestored = false;
            bool Active = true;
        };

        SceneEntry* GetTopEntryUnsafe();
        const SceneEntry* GetTopEntryUnsafe() const;

        mutable std::recursive_mutex m_Mutex;
        std::vector<SceneEntry> m_Scenes;
    };

} // namespace SAGE
