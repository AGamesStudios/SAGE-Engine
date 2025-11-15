#pragma once

#include <string>
#include <memory>

// Plugin export/import macro
#ifdef _WIN32
    #ifdef SAGE_PLUGIN_EXPORTS
        #define SAGE_PLUGIN_API __declspec(dllexport)
    #else
        #define SAGE_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #define SAGE_PLUGIN_API
#endif

namespace SAGE {

    // Plugin lifecycle version
    #define SAGE_PLUGIN_API_VERSION 1

    // Plugin types
    enum class PluginType {
        Renderer,
        Audio,
        Physics,
        Scripting,
        Tool,
        Custom
    };

    // Plugin information
    struct PluginInfo {
        std::string Name;
        std::string Description;
        std::string Author;
        std::string Version;
        int APIVersion = SAGE_PLUGIN_API_VERSION;
        PluginType Type = PluginType::Custom;
    };

    // Base plugin interface
    class IPlugin {
    public:
        virtual ~IPlugin() = default;

        // Plugin lifecycle
        virtual bool OnLoad() = 0;
        virtual void OnUnload() = 0;
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}

        // Plugin info
        virtual const PluginInfo& GetInfo() const = 0;

        // Enable/disable plugin at runtime
        virtual void SetEnabled(bool enabled) { m_Enabled = enabled; }
        virtual bool IsEnabled() const { return m_Enabled; }

    protected:
        bool m_Enabled = true;
    };

    // Macro for easy plugin creation
    #define SAGE_PLUGIN_CLASS(ClassName) \
        extern "C" { \
            SAGE_PLUGIN_API SAGE::IPlugin* CreatePlugin() { \
                return new ClassName(); \
            } \
            SAGE_PLUGIN_API void DestroyPlugin(SAGE::IPlugin* plugin) { \
                delete plugin; \
            } \
            SAGE_PLUGIN_API int GetPluginAPIVersion() { \
                return SAGE_PLUGIN_API_VERSION; \
            } \
        }

} // namespace SAGE
