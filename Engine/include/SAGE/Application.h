#pragma once

#include "SAGE/ApplicationConfig.h"
#include "SAGE/Window.h"
#include "SAGE/Input/Input.h"
#include "SAGE/Plugin/PluginManager.h"
#include "SAGE/Core/ResourceManager.h"

#include <memory>

namespace SAGE {

class Application {
public:
    explicit Application(const ApplicationConfig& config = {});
    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Run();
    void Quit();

    // Plugin management
    void LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& name);

    Window& GetWindow() { return *m_Window; }
    const Window& GetWindow() const { return *m_Window; }

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(double /*deltaTime*/) {}
    virtual void OnFixedUpdate(double /*fixedDeltaTime*/) {}
    virtual void OnShutdown() {}
    virtual void OnResize(int /*width*/, int /*height*/) {}
    virtual void OnFocusChanged(bool /*focused*/) {}
    virtual void OnCloseRequested() {}

private:
    void InitialiseLogger(const ApplicationConfig& config);
    void HandleResize(int width, int height);

    std::unique_ptr<Window> m_Window;
    bool m_Running = true;
    bool m_WindowActive = true;
    
    // Game Loop settings
    double m_FixedTimeStep = 1.0 / 60.0; // 60 Hz physics/logic
    double m_Accumulator = 0.0;
};

Application* GetActiveApplication();

} // namespace SAGE
