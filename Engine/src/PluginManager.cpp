#include "SAGE/Plugin/PluginManager.h"
#include "SAGE/Log.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

namespace SAGE {

PluginManager& PluginManager::Get() {
    static PluginManager instance;
    return instance;
}

PluginManager::~PluginManager() {
    UnloadAll();
}

bool PluginManager::LoadPlugin(const std::string& path) {
    SAGE_INFO("Loading plugin: {}", path);

#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        SAGE_ERROR("Failed to load plugin library: {}", path);
        return false;
    }

    auto createFunc = reinterpret_cast<CreatePluginFunc>(
        GetProcAddress(handle, "CreatePlugin"));
    auto destroyFunc = reinterpret_cast<DestroyPluginFunc>(
        GetProcAddress(handle, "DestroyPlugin"));
#else
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        SAGE_ERROR("Failed to load plugin library: {}", path);
        return false;
    }

    auto createFunc = reinterpret_cast<CreatePluginFunc>(
        dlsym(handle, "CreatePlugin"));
    auto destroyFunc = reinterpret_cast<DestroyPluginFunc>(
        dlsym(handle, "DestroyPlugin"));
#endif

    if (!createFunc || !destroyFunc) {
        SAGE_ERROR("Plugin missing required functions: {}", path);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // Create plugin instance
    IPlugin* plugin = createFunc();
    if (!plugin) {
        SAGE_ERROR("Failed to create plugin instance (createFunc returned nullptr): {}", path);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // Validate plugin info
    const PluginInfo& info = plugin->GetInfo();
    if (info.name.empty()) {
        SAGE_ERROR("Plugin has empty name: {}", path);
        destroyFunc(plugin);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }
    
    // Check version compatibility
    if (!IsPluginCompatible(info.engineVersion)) {
        SAGE_ERROR("Plugin {} requires engine version {}, current is {}",
                   info.name, info.engineVersion.ToString(), 
                   m_EngineVersion.ToString());
        destroyFunc(plugin);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // Load plugin
    if (!plugin->OnLoad()) {
        SAGE_ERROR("Plugin OnLoad failed: {}", info.name);
        destroyFunc(plugin);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // Store plugin
    PluginData data;
    data.plugin.reset(plugin);
    data.libraryHandle = handle;
    data.destroyFunc = destroyFunc;
    data.path = path;

    m_Plugins[info.name] = std::move(data);

    SAGE_INFO("Plugin loaded: {} v{} by {}", 
              info.name, info.version.ToString(), info.author);
    return true;
}

void PluginManager::UnloadPlugin(const std::string& name) {
    auto it = m_Plugins.find(name);
    if (it == m_Plugins.end()) {
        SAGE_WARN("Plugin not found: {}", name);
        return;
    }

    SAGE_INFO("Unloading plugin: {}", name);

    PluginData& data = it->second;
    
    // Call OnUnload
    if (data.plugin) {
        data.plugin->OnUnload();
    }

    // Destroy plugin
    if (data.destroyFunc && data.plugin) {
        data.destroyFunc(data.plugin.release());
    }

    // Unload library
#ifdef _WIN32
    if (data.libraryHandle) {
        FreeLibrary(static_cast<HMODULE>(data.libraryHandle));
    }
#else
    if (data.libraryHandle) {
        dlclose(data.libraryHandle);
    }
#endif

    m_Plugins.erase(it);
}

void PluginManager::UnloadAll() {
    auto pluginNames = GetLoadedPlugins();
    for (const auto& name : pluginNames) {
        UnloadPlugin(name);
    }
}

IPlugin* PluginManager::GetPlugin(const std::string& name) {
    auto it = m_Plugins.find(name);
    if (it == m_Plugins.end()) {
        return nullptr;
    }
    return it->second.plugin.get();
}

std::vector<std::string> PluginManager::GetLoadedPlugins() const {
    std::vector<std::string> names;
    names.reserve(m_Plugins.size());
    for (const auto& [name, data] : m_Plugins) {
        names.push_back(name);
    }
    return names;
}

void PluginManager::UpdatePlugins(double deltaTime) {
    for (auto& [name, data] : m_Plugins) {
        if (data.plugin && data.plugin->IsEnabled()) {
            try {
                data.plugin->OnUpdate(deltaTime);
            } catch (const std::exception& e) {
                SAGE_ERROR("Plugin {} update failed: {}", name, e.what());
                data.plugin->SetEnabled(false);
            }
        }
    }
}

bool PluginManager::IsPluginLoaded(const std::string& name) const {
    return m_Plugins.find(name) != m_Plugins.end();
}

bool PluginManager::IsPluginCompatible(const PluginVersion& version) const {
    return version.IsCompatible(m_EngineVersion);
}

} // namespace SAGE
