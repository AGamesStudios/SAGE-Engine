#include "catch2.hpp"
#include "SAGE/Plugin/PluginManager.h"
#include <filesystem>

using namespace SAGE;

TEST_CASE("PluginManager lifecycle", "[plugin][core]") {
    PluginManager& pm = PluginManager::Get();
    
    SECTION("Initial state") {
        // After construction, should have no plugins loaded
        auto loadedPlugins = pm.GetLoadedPlugins();
        // Note: Depending on global state, this might have plugins from previous tests
        // In a real scenario, we'd need test isolation
    }
    
    SECTION("Load nonexistent plugin") {
        // Should handle gracefully without crashing
        bool loaded = pm.LoadPlugin("nonexistent_plugin_xyz123.dll");
        REQUIRE_FALSE(loaded);
    }
    
    SECTION("Check if plugin is loaded") {
        // Nonexistent plugin should not be loaded
        bool isLoaded = pm.IsPluginLoaded("nonexistent_plugin_xyz123");
        REQUIRE_FALSE(isLoaded);
    }
    
    SECTION("Get nonexistent plugin") {
        // Should return nullptr for nonexistent plugins
        IPlugin* plugin = pm.GetPlugin("nonexistent_plugin_xyz123");
        REQUIRE(plugin == nullptr);
    }
    
    SECTION("Update plugins when none loaded") {
        // Should not crash when updating with no plugins
        pm.UpdatePlugins(0.016);  // Simulate 60 FPS frame
    }
    
    // Note: Testing actual plugin loading requires valid plugin DLLs,
    // which would need to be built and placed in the correct directory.
    // These basic tests verify the API doesn't crash with invalid inputs.
}

TEST_CASE("PluginManager version compatibility", "[plugin][core]") {
    PluginManager& pm = PluginManager::Get();
    
    SECTION("Check engine version compatibility") {
        // Current engine version
        PluginVersion engineVersion{0, 1, 0};
        REQUIRE(pm.IsPluginCompatible(engineVersion));
        
        // Compatible versions (same major version)
        PluginVersion compatible{0, 2, 0};
        REQUIRE(pm.IsPluginCompatible(compatible));
        
        // Incompatible version (different major version)
        PluginVersion incompatible{1, 0, 0};
        REQUIRE_FALSE(pm.IsPluginCompatible(incompatible));
    }
}
