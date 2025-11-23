#include "SAGE/Plugin/IPlugin.h"
#include "SAGE/Log.h"

namespace SAGE {

class ExamplePlugin : public IPlugin {
public:
    const PluginInfo& GetInfo() const override {
        static PluginInfo info = {
            "ExamplePlugin",
            "SAGE Team",
            "A simple example plugin demonstrating the plugin system",
            {1, 0, 0}, // Plugin Version
            {1, 0, 0}  // Engine Version
        };
        return info;
    }

    bool OnLoad() override {
        SAGE_INFO("ExamplePlugin: Loaded successfully!");
        return true;
    }

    void OnUnload() override {
        SAGE_INFO("ExamplePlugin: Unloaded!");
    }

    void OnUpdate(double deltaTime) override {
        // Logic that runs every frame
        // SAGE_INFO("ExamplePlugin: Update {:.4f}", deltaTime);
    }
};

} // namespace SAGE

SAGE_DEFINE_PLUGIN(SAGE::ExamplePlugin)
