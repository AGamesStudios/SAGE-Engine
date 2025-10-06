#pragma once

#include "Core.h"

#include <vector>
#include <string>

namespace SAGE {

    class Scene;
    class Event;

    class SceneStack {
    public:
        SceneStack() = default;
        ~SceneStack();

        SceneStack(const SceneStack&) = delete;
        SceneStack& operator=(const SceneStack&) = delete;
        SceneStack(SceneStack&&) noexcept = default;
        SceneStack& operator=(SceneStack&&) noexcept = default;

        void PushScene(Scope<Scene> scene);
        void PopScene(Scene* scene);
        void PopTopScene();
        void Clear();

        void OnUpdate(float deltaTime);
        void OnRender();
        void OnEvent(Event& event);

        bool Empty() const { return m_Scenes.empty(); }
        std::size_t Size() const { return m_Scenes.size(); }
        const std::vector<Scope<Scene>>& GetScenes() const { return m_Scenes; }
        Scene* GetTopScene();
        const Scene* GetTopScene() const;
        Scene* FindScene(const std::string& name);

        template<typename SceneT, typename... Args>
        SceneT& EmplaceScene(Args&&... args) {
            static_assert(std::is_base_of_v<Scene, SceneT>, "SceneT must derive from Scene");
            auto scene = CreateScope<SceneT>(std::forward<Args>(args)...);
            SceneT& ref = *scene;
            PushScene(std::move(scene));
            return ref;
        }

    private:
        std::vector<Scope<Scene>> m_Scenes;
    };

} // namespace SAGE
