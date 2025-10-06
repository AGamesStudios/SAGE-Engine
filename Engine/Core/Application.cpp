#include "Application.h"
#include "Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/WindowEvent.h"
#include "GameObject.h"
#include "Logger.h"
#include "../Graphics/Renderer.h"
#include "../Input/Input.h"
#include "../Resources/ResourceRegistry.h"
#include "../UI/UISystem.h"
#include "../Audio/AudioSystem.h"
#include <GLFW/glfw3.h>
#if defined(_MSC_VER)
#include <crtdbg.h>
#endif

namespace SAGE {

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name) {
        if (s_Instance) {
            SAGE_ERROR("Application already exists!");
            return;
        }
        s_Instance = this;
        
        Logger::Init();
        ResourceRegistry::Initialize();
        
        WindowProps props;
        props.Title = name;
        m_Window = CreateScope<Window>(props);
        m_Window->SetEventCallback([this](Event& event) { HandleEvent(event); });
        
        Renderer::Init();
        Input::Init();
        UI::UISystem::Init();
        AudioSystem::Init();
        
        SAGE_INFO("SAGE Engine initialized");
    }

    Application::~Application() {
        OnShutdown();
        m_SceneStack.Clear();
        UI::UISystem::Shutdown();
        GameObject::DestroyAll();
        m_SceneManager.ClearStates();
        ResourceRegistry::Shutdown();
        AudioSystem::Shutdown();
        Renderer::Shutdown();
        s_Instance = nullptr;
    }

    void Application::Run() {
        OnInit();
        m_LastFrameTime = static_cast<float>(glfwGetTime());
        
        while (m_Running && !m_Window->ShouldClose()) {
            m_Window->PollEvents();
            Input::Update();

            float time = (float)glfwGetTime();
            float deltaTime = time - m_LastFrameTime;
            m_LastFrameTime = time;
            m_Window->SetDeltaTime(deltaTime);

            AppTickEvent tickEvent;
            m_EventBus.Publish(tickEvent);

            m_SceneManager.ProcessTransitions(m_SceneStack);

            if (m_Minimized) {
                m_Window->SwapBuffers();
                continue;
            }

            Renderer::Update(deltaTime);
            UI::UISystem::BeginFrame(deltaTime);

            AppUpdateEvent updateEvent(deltaTime);
            m_EventBus.Publish(updateEvent);

            m_SceneStack.OnUpdate(deltaTime);
            OnUpdate(deltaTime);
            GameObject::UpdateAll(deltaTime);
            Renderer::Clear();
            m_SceneStack.OnRender();
            GameObject::RenderAll();
            OnRender();
            AppRenderEvent renderEvent;
            m_EventBus.Publish(renderEvent);
            UI::UISystem::Render();
            m_Window->SwapBuffers();
        }

#if defined(_MSC_VER) && defined(_DEBUG)
        _CrtDumpMemoryLeaks();
#endif
    }

    void Application::PushScene(Scope<Scene> scene) {
        m_SceneStack.PushScene(std::move(scene));
    }

    void Application::PopScene(Scene* scene) {
        m_SceneStack.PopScene(scene);
    }

    void Application::PopTopScene() {
        m_SceneStack.PopTopScene();
    }

    void Application::HandleEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&) {
            m_Running = false;
            return true;
        });

        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
            m_Minimized = (e.GetWidth() == 0 || e.GetHeight() == 0);
            return false;
        });

        if (!event.Handled) {
            OnEvent(event);
        }

        if (!event.Handled) {
            m_SceneStack.OnEvent(event);
        }

        if (!event.Handled) {
            m_EventBus.Publish(event);
        }
    }

}
