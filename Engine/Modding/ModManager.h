#pragma once

#include "ModInfo.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

namespace SAGE::Modding {

/**
 * @brief Mod loading event callbacks
 */
using ModEventCallback = std::function<void(const ModInfo&)>;

/**
 * @brief Mod loading error callback
 */
using ModErrorCallback = std::function<void(const std::string& modId, const std::string& error)>;

/**
 * @brief Central mod management system
 * 
 * Features:
 * - Load/unload mods dynamically
 * - Dependency resolution
 * - Asset override system
 * - Hot-reload support
 * - Priority-based loading
 */
class ModManager {
public:
    static ModManager& Instance();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /**
     * @brief Initialize mod system
     * @param modsDirectory Base directory for mods (default: "mods/")
     */
    void Initialize(const std::string& modsDirectory = "mods/");
    
    /**
     * @brief Shutdown mod system and unload all mods
     */
    void Shutdown();
    
    /**
     * @brief Check if mod system is initialized
     */
    bool IsInitialized() const { return m_Initialized; }
    
    // ========================================================================
    // Mod Discovery and Loading
    // ========================================================================
    
    /**
     * @brief Scan mods directory for available mods
     * @return Number of mods discovered
     */
    int DiscoverMods();
    
    /**
     * @brief Load a single mod by ID
     * @param modId Mod identifier
     * @return true if loaded successfully
     */
    bool LoadMod(const std::string& modId);
    
    /**
     * @brief Load mod from specific path
     * @param modPath Path to mod directory
     * @return true if loaded successfully
     */
    bool LoadModFromPath(const std::string& modPath);
    
    /**
     * @brief Unload a mod
     * @param modId Mod identifier
     * @return true if unloaded successfully
     */
    bool UnloadMod(const std::string& modId);
    
    /**
     * @brief Reload a mod (unload + load)
     * @param modId Mod identifier
     * @return true if reloaded successfully
     */
    bool ReloadMod(const std::string& modId);
    
    /**
     * @brief Load all enabled mods with dependency resolution
     * @return Number of mods loaded
     */
    int LoadAllMods();
    
    /**
     * @brief Unload all mods
     */
    void UnloadAllMods();
    
    // ========================================================================
    // Mod Information
    // ========================================================================
    
    /**
     * @brief Get mod information
     * @param modId Mod identifier
     * @return Pointer to ModInfo or nullptr if not found
     */
    const ModInfo* GetModInfo(const std::string& modId) const;
    
    /**
     * @brief Get all loaded mod IDs
     */
    std::vector<std::string> GetLoadedMods() const;
    
    /**
     * @brief Get all available (discovered) mod IDs
     */
    std::vector<std::string> GetAvailableMods() const;
    
    /**
     * @brief Check if mod is loaded
     */
    bool IsModLoaded(const std::string& modId) const;
    
    /**
     * @brief Check if mod is enabled
     */
    bool IsModEnabled(const std::string& modId) const;
    
    /**
     * @brief Get load order (sorted by dependencies and priority)
     */
    std::vector<std::string> GetLoadOrder() const;
    
    // ========================================================================
    // Mod Control
    // ========================================================================
    
    /**
     * @brief Enable or disable a mod
     * @param modId Mod identifier
     * @param enable true to enable, false to disable
     */
    void SetModEnabled(const std::string& modId, bool enable);
    
    /**
     * @brief Set mod priority (higher = loads later)
     * @param modId Mod identifier
     * @param priority Priority value
     */
    void SetModPriority(const std::string& modId, int priority);
    
    // ========================================================================
    // Asset Override System
    // ========================================================================
    
    /**
     * @brief Resolve asset path (check for mod overrides)
     * @param originalPath Original asset path
     * @return Actual path to use (mod override or original)
     */
    std::string ResolveAssetPath(const std::string& originalPath) const;
    
    /**
     * @brief Register asset override
     * @param modId Mod that provides override
     * @param originalPath Original asset path
     * @param modAssetPath Path to mod's replacement asset
     */
    void RegisterAssetOverride(const std::string& modId,
                              const std::string& originalPath,
                              const std::string& modAssetPath);
    
    /**
     * @brief Get all overrides for an asset
     * @param originalPath Original asset path
     * @return List of mod IDs that override this asset
     */
    std::vector<std::string> GetAssetOverrides(const std::string& originalPath) const;
    
    /**
     * @brief Check if asset has any overrides
     */
    bool HasAssetOverride(const std::string& originalPath) const;
    
    // ========================================================================
    // Hot-Reload System
    // ========================================================================
    
    /**
     * @brief Enable/disable hot-reload
     */
    void EnableHotReload(bool enable);
    
    /**
     * @brief Check if hot-reload is enabled
     */
    bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }
    
    /**
     * @brief Check for file changes (call in update loop)
     */
    void CheckForChanges();
    
    /**
     * @brief Set hot-reload check interval (seconds)
     */
    void SetHotReloadInterval(float seconds);
    
    // ========================================================================
    // Event Callbacks
    // ========================================================================
    
    /**
     * @brief Register callback for mod loaded event
     */
    void OnModLoaded(ModEventCallback callback);
    
    /**
     * @brief Register callback for mod unloaded event
     */
    void OnModUnloaded(ModEventCallback callback);
    
    /**
     * @brief Register callback for mod error event
     */
    void OnModError(ModErrorCallback callback);
    
    // ========================================================================
    // Utility
    // ========================================================================
    
    /**
     * @brief Get mods directory
     */
    std::string GetModsDirectory() const { return m_ModsDirectory; }
    
    /**
     * @brief Get statistics
     */
    struct Statistics {
        int totalAvailable = 0;
        int totalLoaded = 0;
        int totalEnabled = 0;
        int totalAssetOverrides = 0;
    };
    
    Statistics GetStatistics() const;
    
    /**
     * @brief Validate mod dependencies
     * @return true if all dependencies are satisfied
     */
    bool ValidateDependencies(const std::string& modId, std::vector<std::string>& missingDeps) const;
    
private:
    ModManager() = default;
    ~ModManager();
    ModManager(const ModManager&) = delete;
    ModManager& operator=(const ModManager&) = delete;
    
    // Dependency resolution (topological sort)
    std::vector<std::string> ResolveDependencies(const std::vector<std::string>& modIds) const;
    bool HasCircularDependency(const std::string& modId, std::unordered_set<std::string>& visited) const;
    
    // Loading helpers
    bool LoadModInternal(const std::string& modId, std::unique_lock<std::mutex>& lock);
    void UnloadModInternal(const std::string& modId, std::unique_lock<std::mutex>& lock);
    void ProcessAssetOverrides(const ModInfo& info);
    void RegisterAssetOverrideLocked(const std::string& modId,
                                     const std::string& originalPath,
                                     const std::string& modAssetPath);
    
    // Callbacks
    void TriggerModLoaded(const ModInfo& info, std::unique_lock<std::mutex>& lock);
    void TriggerModUnloaded(const ModInfo& info, std::unique_lock<std::mutex>& lock);
    void TriggerModError(const std::string& modId, const std::string& error, std::unique_lock<std::mutex>& lock);
    
    // Data
    bool m_Initialized = false;
    std::string m_ModsDirectory;
    
    // Mod storage
    std::unordered_map<std::string, std::shared_ptr<ModInfo>> m_AvailableMods;  // All discovered mods
    std::unordered_set<std::string> m_LoadedMods;                               // Currently loaded
    std::vector<std::string> m_LoadOrder;                                       // Order mods were loaded
    
    // Asset overrides: original_path -> [(modId, mod_path, priority)]
    struct AssetOverride {
        std::string modId;
        std::string modPath;
        int priority;
    };
    std::unordered_map<std::string, std::vector<AssetOverride>> m_AssetOverrides;
    
    // Hot-reload
    bool m_HotReloadEnabled = false;
    float m_HotReloadInterval = 1.0f;
    float m_TimeSinceLastCheck = 0.0f;
    std::unordered_map<std::string, time_t> m_FileModTimes;
    bool m_HotReloadWarningIssued = false;
    
    // Callbacks
    std::vector<ModEventCallback> m_OnModLoaded;
    std::vector<ModEventCallback> m_OnModUnloaded;
    std::vector<ModErrorCallback> m_OnModError;
    
    // Thread safety
    mutable std::mutex m_Mutex;
};

} // namespace SAGE::Modding
