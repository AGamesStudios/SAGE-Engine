#pragma once

#include <string>
#include <memory>

namespace SAGE {

// Plugin version structure
struct PluginVersion {
    int major = 1;
    int minor = 0;
    int patch = 0;

    std::string ToString() const {
        return std::to_string(major) + "." + 
               std::to_string(minor) + "." + 
               std::to_string(patch);
    }

    bool IsCompatible(const PluginVersion& other) const {
        return major == other.major;
    }
};

// Plugin metadata
struct PluginInfo {
    std::string name;
    std::string author;
    std::string description;
    PluginVersion version;
    PluginVersion engineVersion;
};

// Base plugin interface
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Plugin lifecycle
    virtual bool OnLoad() = 0;
    virtual void OnUnload() = 0;
    virtual void OnUpdate(double deltaTime) = 0;

    // Plugin info
    virtual const PluginInfo& GetInfo() const = 0;

    // Safe resource access
    virtual bool IsEnabled() const { return m_Enabled; }
    virtual void SetEnabled(bool enabled) { m_Enabled = enabled; }

protected:
    bool m_Enabled = true;
};

// Plugin creation function type
using CreatePluginFunc = IPlugin* (*)();
using DestroyPluginFunc = void (*)(IPlugin*);

} // namespace SAGE

// Macro for plugin export
#ifdef _WIN32
    #define SAGE_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define SAGE_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Helper macro to define plugin
#define SAGE_DEFINE_PLUGIN(PluginClass) \
    extern "C" { \
        SAGE_PLUGIN_EXPORT SAGE::IPlugin* CreatePlugin() { \
            return new PluginClass(); \
        } \
        SAGE_PLUGIN_EXPORT void DestroyPlugin(SAGE::IPlugin* plugin) { \
            delete plugin; \
        } \
    }
