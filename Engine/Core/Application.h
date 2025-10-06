#pragma once

#include "Core.h"
#include "EventBus.h"
#include "Scene.h"
#include "SceneStack.h"
#include "SceneManager.h"
#include "Window.h"

#include <type_traits>
#include <utility>

namespace SAGE {

    class Event;
    class Scene;

    class Application {
    public:
        Application(const std::string& name = "SAGE Application");
        virtual ~Application();
        
        void Run();
        void Close() { m_Running = false; }
        
        Window& GetWindow() { return *m_Window; }
        EventBus& GetEventBus() { return m_EventBus; }
        SceneStack& GetSceneStack() { return m_SceneStack; }
    SceneManager& GetSceneManager() { return m_SceneManager; }
        
        static Application& Get() { return *s_Instance; }
        static bool HasInstance() { return s_Instance != nullptr; }
        
    protected:
        virtual void OnInit() {}
        virtual void OnUpdate(float /*deltaTime*/) {}
        virtual void OnRender() {}
        virtual void OnShutdown() {}
        virtual void OnEvent(Event& /*event*/) {}
        
        bool m_Running = true; // Теперь protected для доступа из наследников
        
    public:
        void PushScene(Scope<Scene> scene);
        void PopScene(Scene* scene);
        void PopTopScene();

        template<typename SceneT, typename... Args>
        SceneT& EmplaceScene(Args&&... args) {
            static_assert(std::is_base_of_v<Scene, SceneT>, "SceneT must derive from Scene");
            auto scene = CreateScope<SceneT>(std::forward<Args>(args)...);
            SceneT& ref = *scene;
            PushScene(std::move(scene));
            return ref;
        }

    private:
        void HandleEvent(Event& event);

        Scope<Window> m_Window;
        SceneStack m_SceneStack;
    SceneManager m_SceneManager;
        EventBus m_EventBus;
        float m_LastFrameTime = 0.0f;
        bool m_Minimized = false;
        
        static Application* s_Instance;
    };

    // Определяется в клиентском приложении
    Application* CreateApplication();

}
