#include "PluginManager.h"
#include "Core/Log.h"
#include <filesystem>

namespace SAGE {

    bool PluginManager::LoadPlugin(const std::string& path) {
        // Check if file exists
        if (!std::filesystem::exists(path)) {
            SAGE_ERROR("Plugin file not found: {}", path);
            return false;
        }

        // Load library
        PLUGIN_HANDLE handle = nullptr;
        
#ifdef _WIN32
        handle = LoadLibraryA(path.c_str());
        if (!handle) {
            SAGE_ERROR("Failed to load plugin: {} (Error: {})", path, GetLastError());
            return false;
        }
#else
        handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            SAGE_ERROR("Failed to load plugin: {} (Error: {})", path, dlerror());
            return false;
        }
#endif

        // Get function pointers
        typedef IPlugin* (*CreatePluginFunc)();
        typedef void (*DestroyPluginFunc)(IPlugin*);
        typedef int (*GetAPIVersionFunc)();

#ifdef _WIN32
        auto createFunc = (CreatePluginFunc)GetProcAddress(handle, "CreatePlugin");
        auto destroyFunc = (DestroyPluginFunc)GetProcAddress(handle, "DestroyPlugin");
        auto versionFunc = (GetAPIVersionFunc)GetProcAddress(handle, "GetPluginAPIVersion");
#else
        auto createFunc = (CreatePluginFunc)dlsym(handle, "CreatePlugin");
        auto destroyFunc = (DestroyPluginFunc)dlsym(handle, "DestroyPlugin");
        auto versionFunc = (GetAPIVersionFunc)dlsym(handle, "GetPluginAPIVersion");
#endif

        if (!createFunc || !destroyFunc || !versionFunc) {
            SAGE_ERROR("Plugin missing required functions: {}", path);
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }

        // Check API version
        int apiVersion = versionFunc();
        if (apiVersion != SAGE_PLUGIN_API_VERSION) {
            SAGE_ERROR("Plugin API version mismatch: {} (expected {}, got {})", 
                path, SAGE_PLUGIN_API_VERSION, apiVersion);
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
            SAGE_ERROR("Failed to create plugin instance: {}", path);
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }

        // Initialize plugin
        if (!plugin->OnLoad()) {
            SAGE_ERROR("Plugin initialization failed: {}", path);
            destroyFunc(plugin);
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }

        // Store plugin data
        const PluginInfo& info = plugin->GetInfo();
        
        if (IsPluginLoaded(info.Name)) {
            SAGE_WARN("Plugin already loaded: {}", info.Name);
            destroyFunc(plugin);
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }

        PluginData data;
        data.Handle = handle;
        data.Instance = plugin;
        data.Path = path;
        
        m_Plugins[info.Name] = data;
        m_PluginList.push_back(plugin);

        SAGE_INFO("Plugin loaded: {} v{} by {}", info.Name, info.Version, info.Author);

        // Trigger callback
        if (m_OnPluginLoaded) {
            m_OnPluginLoaded(plugin);
        }

        return true;
    }

    bool PluginManager::UnloadPlugin(const std::string& name) {
        auto it = m_Plugins.find(name);
        if (it == m_Plugins.end()) {
            SAGE_WARN("Plugin not found: {}", name);
            return false;
        }

        PluginData& data = it->second;
        
        // Trigger callback before unload
        if (m_OnPluginUnloaded) {
            m_OnPluginUnloaded(data.Instance);
        }

        // Cleanup plugin
        data.Instance->OnUnload();

        // Get destroy function
        typedef void (*DestroyPluginFunc)(IPlugin*);
#ifdef _WIN32
        auto destroyFunc = (DestroyPluginFunc)GetProcAddress(data.Handle, "DestroyPlugin");
#else
        auto destroyFunc = (DestroyPluginFunc)dlsym(data.Handle, "DestroyPlugin");
#endif

        if (destroyFunc) {
            destroyFunc(data.Instance);
        }

        // Unload library
#ifdef _WIN32
        FreeLibrary(data.Handle);
#else
        dlclose(data.Handle);
#endif

        // Remove from list
        m_PluginList.erase(
            std::remove(m_PluginList.begin(), m_PluginList.end(), data.Instance),
            m_PluginList.end()
        );

        m_Plugins.erase(it);
        
        SAGE_INFO("Plugin unloaded: {}", name);
        return true;
    }

    void PluginManager::UnloadAllPlugins() {
        // Copy names to avoid iterator invalidation
        std::vector<std::string> names;
        names.reserve(m_Plugins.size());
        for (const auto& pair : m_Plugins) {
            names.push_back(pair.first);
        }

        for (const auto& name : names) {
            UnloadPlugin(name);
        }
    }

    IPlugin* PluginManager::GetPlugin(const std::string& name) {
        auto it = m_Plugins.find(name);
        return (it != m_Plugins.end()) ? it->second.Instance : nullptr;
    }

    std::vector<IPlugin*> PluginManager::GetPluginsByType(PluginType type) {
        std::vector<IPlugin*> result;
        for (auto* plugin : m_PluginList) {
            if (plugin->GetInfo().Type == type) {
                result.push_back(plugin);
            }
        }
        return result;
    }

    void PluginManager::UpdatePlugins(float deltaTime) {
        for (auto* plugin : m_PluginList) {
            if (plugin->IsEnabled()) {
                plugin->OnUpdate(deltaTime);
            }
        }
    }

    void PluginManager::RenderPlugins() {
        for (auto* plugin : m_PluginList) {
            if (plugin->IsEnabled()) {
                plugin->OnRender();
            }
        }
    }

    void PluginManager::SetPluginEnabled(const std::string& name, bool enabled) {
        IPlugin* plugin = GetPlugin(name);
        if (plugin) {
            plugin->SetEnabled(enabled);
            SAGE_INFO("Plugin {} {}", name, enabled ? "enabled" : "disabled");
        }
    }

    bool PluginManager::IsPluginLoaded(const std::string& name) const {
        return m_Plugins.find(name) != m_Plugins.end();
    }

} // namespace SAGE
