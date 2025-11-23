#pragma once

#include "SAGE/Plugin/IPlugin.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace SAGE {

class PluginManager {
public:
    static PluginManager& Get();

    // Plugin loading/unloading
    bool LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& name);
    void UnloadAll();

    // Plugin access
    IPlugin* GetPlugin(const std::string& name);
    std::vector<std::string> GetLoadedPlugins() const;

    // Plugin lifecycle
    void UpdatePlugins(double deltaTime);

    // Safety checks
    bool IsPluginLoaded(const std::string& name) const;
    bool IsPluginCompatible(const PluginVersion& version) const;

private:
    PluginManager() = default;
    ~PluginManager();

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    struct PluginData {
        std::unique_ptr<IPlugin> plugin;
        void* libraryHandle = nullptr;
        DestroyPluginFunc destroyFunc = nullptr;
        std::string path;
    };

    std::unordered_map<std::string, PluginData> m_Plugins;
    PluginVersion m_EngineVersion{0, 1, 0};
};

} // namespace SAGE
