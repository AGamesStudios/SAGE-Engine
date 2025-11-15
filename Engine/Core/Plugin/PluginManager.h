#pragma once

#include "PluginInterface.h"
#include <unordered_map>
#include <vector>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
    #define PLUGIN_HANDLE HMODULE
#else
    #include <dlfcn.h>
    #define PLUGIN_HANDLE void*
#endif

namespace SAGE {

    class PluginManager {
    public:
        static PluginManager& Get() {
            static PluginManager instance;
            return instance;
        }

        // Load plugin from DLL/SO file
        bool LoadPlugin(const std::string& path);
        
        // Unload plugin by name
        bool UnloadPlugin(const std::string& name);
        
        // Unload all plugins
        void UnloadAllPlugins();

        // Get plugin by name
        IPlugin* GetPlugin(const std::string& name);
        
        // Get all plugins of specific type
        std::vector<IPlugin*> GetPluginsByType(PluginType type);
        
        // Get all plugins
        const std::vector<IPlugin*>& GetAllPlugins() const { return m_PluginList; }

        // Plugin lifecycle
        void UpdatePlugins(float deltaTime);
        void RenderPlugins();

        // Enable/disable plugin
        void SetPluginEnabled(const std::string& name, bool enabled);
        
        // Check if plugin is loaded
        bool IsPluginLoaded(const std::string& name) const;

        // Event system for plugins
        using PluginEvent = std::function<void(IPlugin*)>;
        void OnPluginLoaded(PluginEvent callback) { m_OnPluginLoaded = callback; }
        void OnPluginUnloaded(PluginEvent callback) { m_OnPluginUnloaded = callback; }

    private:
        PluginManager() = default;
        ~PluginManager() { UnloadAllPlugins(); }

        PluginManager(const PluginManager&) = delete;
        PluginManager& operator=(const PluginManager&) = delete;

        struct PluginData {
            PLUGIN_HANDLE Handle;
            IPlugin* Instance;
            std::string Path;
        };

        std::unordered_map<std::string, PluginData> m_Plugins;
        std::vector<IPlugin*> m_PluginList;
        
        PluginEvent m_OnPluginLoaded;
        PluginEvent m_OnPluginUnloaded;
    };

} // namespace SAGE
