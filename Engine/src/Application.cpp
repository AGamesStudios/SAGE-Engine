#include "SAGE/Application.h"

#include "SAGE/Log.h"
#include "SAGE/Logger.h"
#include "SAGE/Time.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Core/CommandLine.h"
#include "SAGE/Audio/Audio.h"
#include "SAGE/Core/SceneManager.h"

#include <memory>

namespace SAGE {

namespace {
    Application*& ActiveApplication() {
        static Application* instance = nullptr;
        return instance;
    }
}

Application::Application(const ApplicationConfig& config) {
    CommandLine::Initialize();
    InitialiseLogger(config);

    m_Window = Window::Create(config.window);
    Input::Init(m_Window->GetNativeHandle());
    Renderer::Init(config.renderer);
    Audio::Init();

    m_Window->SetResizeCallback([this](int width, int height) {
        HandleResize(width, height);
    });
    m_Window->SetFocusCallback([this](bool focused) {
        m_WindowActive = focused;
        OnFocusChanged(focused);
    });
    m_Window->SetCloseCallback([this]() {
        OnCloseRequested();
        Quit();
    });
    
    ActiveApplication() = this;
}

Application::~Application() {
    Audio::Shutdown();
    Renderer::Shutdown();
    Input::Shutdown();
    ActiveApplication() = nullptr;
    Logger::Shutdown();
}

void Application::Run() {
    Time::Reset();
    OnInit();

    int fbWidth = 0;
    int fbHeight = 0;
    m_Window->GetFramebufferSize(fbWidth, fbHeight);
    HandleResize(fbWidth, fbHeight);

    while (m_Running && !m_Window->ShouldClose()) {
        Time::Tick();
        
        // Update input state from previous frame (Pressed -> Held, JustReleased -> Released)
        Input::Update();
        
        // Poll new events for this frame
        m_Window->PollEvents();

        double deltaTime = Time::Delta();
        
        if (!m_WindowActive) {
            // If window is not active, we still run logic but skip rendering to save GPU
            // We also sleep a bit to reduce CPU usage since high FPS is not needed in background
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // ~50 FPS cap in background
        }

        m_Accumulator += deltaTime;
        double fixedStep = Time::FixedDelta();

        while (m_Accumulator >= fixedStep) {
            OnFixedUpdate(fixedStep);
            SceneManager::Get().FixedUpdate(static_cast<float>(fixedStep));
            // Also update plugins fixed update if we had one, but for now just standard update
            m_Accumulator -= fixedStep;
        }

        OnUpdate(deltaTime);
        SceneManager::Get().Update(static_cast<float>(deltaTime));
        SceneManager::Get().Render();
        
        // Update plugins
        PluginManager::Get().UpdatePlugins(deltaTime);

        m_Window->SwapBuffers();
    }

    OnShutdown();
    
    // Cleanup
    PluginManager::Get().UnloadAll();
    ResourceManager::Get().UnloadAll();
}

void Application::Quit() {
    m_Running = false;
    if (m_Window) {
        m_Window->RequestClose();
    }
}

void Application::LoadPlugin(const std::string& path) {
    PluginManager::Get().LoadPlugin(path);
}

void Application::UnloadPlugin(const std::string& name) {
    PluginManager::Get().UnloadPlugin(name);
}

void Application::InitialiseLogger(const ApplicationConfig& config) {
    if (!config.enableLogging) {
        Logger::SetLevel(LogLevel::Critical);
        return;
    }

    Logger::Init();
    Logger::SetLevel(LogLevel::Trace);
}

Application* GetActiveApplication() {
    return ActiveApplication();
}

void Application::HandleResize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    Renderer::SetViewport(0, 0, width, height);
    OnResize(width, height);
}

} // namespace SAGE
