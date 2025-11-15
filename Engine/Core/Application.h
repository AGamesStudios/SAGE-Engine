#pragma once

#include "Core.h"
#include "EventBus.h"
#include "Scene.h"
#include "SceneStack.h"
#include "SceneManager.h"
#include "ServiceLocator.h"
#include "Window.h"

#include <type_traits>
#include <utility>

namespace SAGE {

    class Event;
    class Scene;
    
    namespace UI {
        class UIManager;
    }

    class Application {
    public:
        Application(const std::string& name = "SAGE Application");
        virtual ~Application();
        
        void Run();
        void Close() { m_Running = false; }
        bool IsRunning() const { return m_Running; }
        
        Window& GetWindow() { return *m_Window; }
        EventBus& GetEventBus() { return m_EventBus; }
        SceneStack& GetSceneStack() { return m_SceneStack; }
        SceneManager& GetSceneManager() { return m_SceneManager; }
        ServiceLocator& GetServices() { return m_Services; }
        
        // Input system accessors (now via InputManager singleton)
        // Deprecated: Use Input:: namespace or InputManager::Get() directly
        
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
        void PushScene(Scope<Scene> scene, SceneParameters params = SceneParameters{}, bool stateRestored = false);
        void PopScene(Scene* scene, SceneParameters resumeParams = SceneParameters{}, bool stateRestored = false);
        void PopTopScene(SceneParameters resumeParams = SceneParameters{}, bool stateRestored = false);

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

        ServiceLocator m_Services;  // Service container (owns all services)
        Scope<Window> m_Window;
        SceneStack m_SceneStack;
        SceneManager m_SceneManager;
        EventBus m_EventBus;
        float m_LastFrameTime = 0.0f;
        float m_FixedAccumulator = 0.0f;
        float m_FixedTimeStep = 1.0f / 60.0f;
        float m_MaxFixedStepTime = 0.25f;
        int m_MaxFixedStepsPerFrame = 5;
        bool m_Minimized = false;
        
        // UI Manager (automatically managed)
        UI::UIManager* m_UIManager = nullptr;
        
        static Application* s_Instance;
    };

    // Определяется в клиентском приложении
    Application* CreateApplication();

}
