#include "../../Engine/Core/Plugin/PluginInterface.h"
#include "../../Engine/Core/Logger.h"
#include <chrono>
#include <iostream>

using namespace SAGE;

#define LOG_INFO(msg) std::cout << "[FPSCounter] " << msg << std::endl
#define LOG_TRACE(msg) // Disabled for performance

class FPSCounterPlugin : public IPlugin {
private:
    float m_FrameTime = 0.0f;
    int m_FrameCount = 0;
    float m_FPS = 0.0f;
    float m_AccumulatedTime = 0.0f;
    
    bool m_ShowFPS = true;
    bool m_ShowFrameTime = true;
    
public:
    bool OnLoad() override {
        LOG_INFO("FPS Counter Plugin loaded");
        return true;
    }
    
    void OnUnload() override {
        LOG_INFO("FPS Counter Plugin unloaded");
    }
    
    void OnUpdate(float deltaTime) override {
        m_FrameTime = deltaTime;
        m_FrameCount++;
        m_AccumulatedTime += deltaTime;
        
        // Update FPS every second
        if (m_AccumulatedTime >= 1.0f) {
            m_FPS = m_FrameCount / m_AccumulatedTime;
            m_FrameCount = 0;
            m_AccumulatedTime = 0.0f;
        }
    }
    
    void OnRender() override {
        if (!IsEnabled()) return;
        
        // Simple console output (would use ImGui or text renderer in real impl)
        if (m_ShowFPS) {
            LOG_TRACE("FPS: " << m_FPS);
        }
        
        if (m_ShowFrameTime) {
            LOG_TRACE("Frame Time: " << (m_FrameTime * 1000.0f) << "ms");
        }
    }
    
    const PluginInfo& GetInfo() const override {
        static PluginInfo info = {
            "FPSCounter",
            "Displays FPS and frame time overlay",
            "SAGE Team",
            "1.0.0",
            SAGE_PLUGIN_API_VERSION,
            PluginType::Tool
        };
        return info;
    }
    
    // Custom API
    float GetFPS() const { return m_FPS; }
    float GetFrameTime() const { return m_FrameTime; }
    
    void SetShowFPS(bool show) { m_ShowFPS = show; }
    void SetShowFrameTime(bool show) { m_ShowFrameTime = show; }
};

// Export plugin
SAGE_PLUGIN_CLASS(FPSCounterPlugin)
