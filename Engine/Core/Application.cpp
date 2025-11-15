#include "Application.h"
#include "Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/WindowEvent.h"
#include "GameObject.h"
#include "Logger.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Rendering/StateManagement/RenderStateManager.h"
#include "Input/InputManager.h"
#include "Assets/AssetManager.h"
#include "../Resources/ResourceRegistry.h" // Adjusted relative path (source lives in Engine/Resources)
#include "UI/UISystem.h"
#include "UI/UIManager.h"
#include "Audio/AudioSystem.h"
#include <algorithm>
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
        ServiceLocator::SetGlobalInstance(&m_Services);
        
    // Initialize logging (writes to logs/engine.log)
    Logger::Init("logs");
    Logger::SetMinLevel(LogLevel::Trace);
    Logger::EnableRotation(5 * 1024 * 1024); // 5MB rotation
        
        // ========== Register Services ==========
        SAGE_INFO("Registering engine services...");
        m_Services.RegisterShaderManager(CreateScope<ShaderManager>());
        m_Services.RegisterRenderStateManager(CreateScope<StateManagement::RenderStateManager>());
        m_Services.RegisterAudioSystem(CreateScope<AudioSystem>());
        
        WindowProps props;
        props.Title = name;
        m_Window = CreateScope<Window>(props);
        m_Window->SetEventCallback([this](Event& event) { HandleEvent(event); });
        
        // Initialize Input Manager (replaces InputBridge setup)
        InputManager::Get().Initialize(m_Window->GetNativeWindow());
        
        Renderer::Init();
    // Initialize renderer viewport and camera with current framebuffer size
    Renderer::OnWindowResize(static_cast<int>(m_Window->GetFramebufferWidth()),
                 static_cast<int>(m_Window->GetFramebufferHeight()));
        
        
        // ========== Initialize Services ==========
        SAGE_INFO("Initializing ServiceLocator...");
        m_Services.Initialize();
        
        // Initialize UI Manager
        m_UIManager = &UI::UIManager::Get();
        m_UIManager->Init(InputManager::Get().GetBridge(), m_Window->GetNativeWindow());
        
        SAGE_INFO("SAGE Engine initialized");
    }

    Application::~Application() {
        OnShutdown();
        
        // Shutdown Input Manager
        InputManager::Get().Shutdown();
        
        // Shutdown UI Manager
        if (m_UIManager) {
            m_UIManager->Shutdown();
            m_UIManager = nullptr;
        }
        
        m_SceneStack.Clear();
        GameObject::DestroyAll();
        m_SceneManager.ClearStates();
        Renderer::Shutdown();
        
    // Shutdown services (in reverse order of initialization)
    m_Services.Shutdown();
        
        s_Instance = nullptr;
    }

    void Application::Run() {
        OnInit();
        
        // Validate that at least one scene is pushed before starting the main loop
        if (m_SceneStack.Empty()) {
            SAGE_ERROR("Application::Run() - No scene found! At least one scene must be pushed before calling Run().");
            SAGE_ERROR("Please call PushScene() or EmplaceScene() in your OnInit() implementation.");
            m_Running = false;
            return;
        }
        
        m_LastFrameTime = static_cast<float>(glfwGetTime());
        
        while (m_Running && !m_Window->ShouldClose()) {
            // Update input system (transitions Pressed->Held, polls gamepads, etc.)
            InputManager::Get().Update();

            m_Window->PollEvents();
            
            // Process async resource uploads on main thread (GPU operations)
            ResourceManager::Get().ProcessAsyncUploads();

            float time = static_cast<float>(glfwGetTime());
            float rawDelta = time - m_LastFrameTime;
            m_LastFrameTime = time;
            float deltaTime = std::clamp(rawDelta, 0.0f, m_MaxFixedStepTime);
            m_Window->SetDeltaTime(deltaTime);

            AppTickEvent tickEvent;
            m_EventBus.Publish(tickEvent);

            m_SceneManager.ProcessTransitions(m_SceneStack);
            
            // Check if all scenes were removed during transition processing
            if (m_SceneStack.Empty()) {
                SAGE_WARNING("Application::Run() - Scene stack is empty. Stopping application.");
                m_Running = false;
                break;
            }

            m_FixedAccumulator = std::clamp(m_FixedAccumulator + deltaTime, 0.0f, m_MaxFixedStepTime);
            int fixedSteps = 0;
            
            while (m_FixedAccumulator >= m_FixedTimeStep && fixedSteps < m_MaxFixedStepsPerFrame) {
                m_SceneStack.OnFixedUpdate(m_FixedTimeStep);
                m_FixedAccumulator -= m_FixedTimeStep;
                ++fixedSteps;
            }
            if (fixedSteps == m_MaxFixedStepsPerFrame) {
                m_FixedAccumulator = 0.0f;
            }
            if (m_Minimized) {
                m_Window->SwapBuffers();
                continue;
            }

            Renderer::BeginScene();
            Renderer::Update(deltaTime);

            AppUpdateEvent updateEvent(deltaTime);
            m_EventBus.Publish(updateEvent);

            m_SceneStack.OnUpdate(deltaTime);
            OnUpdate(deltaTime);
            GameObject::UpdateAll(deltaTime);
            
            // Update UI Manager (automatic)
            if (m_UIManager) {
                m_UIManager->Update(deltaTime);
            }
            
            m_SceneStack.OnRender();
            GameObject::RenderAll();
            OnRender();
            
            // Render UI Manager (automatic)
            if (m_UIManager) {
                m_UIManager->Render();
            }
            
            AppRenderEvent renderEvent;
            m_EventBus.Publish(renderEvent);
            
            Renderer::EndScene();
            m_Window->SwapBuffers();
        }

#if defined(_MSC_VER) && defined(_DEBUG)
        _CrtDumpMemoryLeaks();
#endif
    }

    void Application::PushScene(Scope<Scene> scene, SceneParameters params, bool stateRestored) {
        m_SceneStack.PushScene(std::move(scene), std::move(params), stateRestored);
    }

    void Application::PopScene(Scene* scene, SceneParameters resumeParams, bool stateRestored) {
        m_SceneStack.PopScene(scene, std::move(resumeParams), stateRestored);
    }

    void Application::PopTopScene(SceneParameters resumeParams, bool stateRestored) {
        m_SceneStack.PopTopScene(std::move(resumeParams), stateRestored);
    }

    void Application::HandleEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&) {
            SAGE_WARNING("[Application] WindowCloseEvent received, stopping run loop");
            m_Running = false;
            return true;
        });

        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
            const unsigned int width = e.GetWidth();
            const unsigned int height = e.GetHeight();
            m_Minimized = (width == 0 || height == 0);

            if (!m_Minimized) {
                Renderer::OnWindowResize(static_cast<int>(width), static_cast<int>(height));
            }
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
