#include "SAGE.h"
#include "Core/Plugin/PluginManager.h"
#include "Graphics/API/RenderSystemConfig.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <string>

using namespace SAGE;
using namespace SAGE::Graphics;

void LOG_INFO(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

int main() {
    Logger::Init();
    LOG_INFO("Plugin Demo Starting...");
    
    WindowProps props;
    props.Title = "SAGE Plugin System Demo";
    props.Width = 1280;
    props.Height = 720;
    Window window(props);
    
    RenderSystemConfig cfg;
    cfg.backendType = BackendType::OpenGL;
    Renderer::Init(cfg);
    
    // Get plugin manager
    auto& pm = PluginManager::Get();
    
    // Setup plugin events
    pm.OnPluginLoaded([](IPlugin* plugin) {
        std::cout << "[INFO] ✓ Plugin loaded: " << plugin->GetInfo().Name 
                  << " v" << plugin->GetInfo().Version << std::endl;
    });
    
    pm.OnPluginUnloaded([](IPlugin* plugin) {
        std::cout << "[INFO] ✗ Plugin unloaded: " << plugin->GetInfo().Name << std::endl;
    });
    
    // Load FPS counter plugin
    std::string pluginPath = "plugins/FPSCounterPlugin.dll";
    if (pm.LoadPlugin(pluginPath)) {
        LOG_INFO("FPS Counter plugin ready!");
    } else {
        LOG_INFO("Failed to load FPS Counter plugin");
    }
    
    LOG_INFO("\n=== PLUGIN DEMO ===");
    LOG_INFO("ESC: Exit | 1: Toggle FPS plugin | 2: Reload plugin");
    std::cout << "[INFO] Loaded plugins: " << pm.GetAllPlugins().size() << "\n" << std::endl;
    
    float lastTime = glfwGetTime();
    bool key1 = false, key2 = false;
    
    while (!window.ShouldClose()) {
        window.PollEvents();
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
        
        // Toggle plugin
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_1) == GLFW_PRESS && !key1) {
            auto* plugin = pm.GetPlugin("FPSCounter");
            if (plugin) {
                bool enabled = !plugin->IsEnabled();
                pm.SetPluginEnabled("FPSCounter", enabled);
                LOG_INFO(enabled ? "FPS Counter: ON" : "FPS Counter: OFF");
            }
            key1 = true;
        }
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_1) == GLFW_RELEASE) {
            key1 = false;
        }
        
        // Reload plugin
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_2) == GLFW_PRESS && !key2) {
            LOG_INFO("Reloading plugin...");
            pm.UnloadPlugin("FPSCounter");
            if (pm.LoadPlugin(pluginPath)) {
                LOG_INFO("Plugin reloaded successfully!");
            }
            key2 = true;
        }
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_2) == GLFW_RELEASE) {
            key2 = false;
        }
        
        // Update all plugins
        pm.UpdatePlugins(deltaTime);
        
        // Render
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render all plugins
        pm.RenderPlugins();
        
        window.SwapBuffers();
    }
    
    // Cleanup
    LOG_INFO("Unloading all plugins...");
    pm.UnloadAllPlugins();
    
    Renderer::Shutdown();
    LOG_INFO("Demo finished");
    return 0;
}
