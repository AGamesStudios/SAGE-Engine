#include "ModManager.h"
#include <Core/Logger.h>
#include <filesystem>
#include <algorithm>
#include <queue>

namespace fs = std::filesystem;

namespace SAGE::Modding {

ModManager& ModManager::Instance() {
    static ModManager instance;
    return instance;
}

ModManager::~ModManager() {
    if (m_Initialized) {
        Shutdown();
    }
}

// ============================================================================
// Initialization
// ============================================================================

void ModManager::Initialize(const std::string& modsDirectory) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_Initialized) {
        SAGE_WARN("ModManager: Already initialized");
        return;
    }
    
    m_ModsDirectory = modsDirectory;
    
    // Создать папку модов если не существует
    if (!fs::exists(modsDirectory)) {
        fs::create_directories(modsDirectory);
        SAGE_INFO("ModManager: Created mods directory: {}", modsDirectory);
    }
    
    m_Initialized = true;
    SAGE_INFO("ModManager: Initialized with directory: {}", modsDirectory);
}

void ModManager::Shutdown() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    
    if (!m_Initialized) return;
    
    SAGE_INFO("ModManager: Shutting down...");
    
    // Unload all mods
    auto loadedCopy = m_LoadedMods; // Copy because UnloadModInternal modifies the set
    for (const auto& modId : loadedCopy) {
        UnloadModInternal(modId, lock);
    }
    
    m_AvailableMods.clear();
    m_LoadedMods.clear();
    m_LoadOrder.clear();
    m_AssetOverrides.clear();
    m_FileModTimes.clear();
    
    m_Initialized = false;
    SAGE_INFO("ModManager: Shutdown complete");
}

// ============================================================================
// Mod Discovery and Loading
// ============================================================================

int ModManager::DiscoverMods() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (!m_Initialized) {
        SAGE_ERROR("ModManager: Not initialized");
        return 0;
    }
    
    SAGE_INFO("ModManager: Discovering mods in {}", m_ModsDirectory);
    
    int discovered = 0;
    
    try {
        for (const auto& entry : fs::directory_iterator(m_ModsDirectory)) {
            if (!entry.is_directory()) continue;
            
            std::string modPath = entry.path().string();
            std::string modJsonPath = modPath + "/mod.json";
            
            if (!fs::exists(modJsonPath)) {
                SAGE_WARN("ModManager: Skipping {} - no mod.json found", modPath);
                continue;
            }
            
            auto modInfo = ModInfo::FromJSON(modJsonPath);
            if (!modInfo.has_value()) {
                SAGE_ERROR("ModManager: Failed to parse mod.json in {}", modPath);
                continue;
            }
            
            modInfo->path = modPath;
            
            // Validate
            if (!modInfo->IsValid()) {
                auto errors = modInfo->GetValidationErrors();
                SAGE_ERROR("ModManager: Invalid mod '{}': {}", modInfo->id, errors[0]);
                continue;
            }
            
            m_AvailableMods[modInfo->id] = std::make_shared<ModInfo>(std::move(*modInfo));
            discovered++;
            
            SAGE_INFO("ModManager: Discovered mod '{}' v{}", 
                     m_AvailableMods[modInfo->id]->name,
                     m_AvailableMods[modInfo->id]->version.ToString());
        }
        
    } catch (const fs::filesystem_error& e) {
        SAGE_ERROR("ModManager: Filesystem error during discovery: {}", e.what());
        return discovered;
    }
    
    SAGE_INFO("ModManager: Discovered {} mods", discovered);
    return discovered;
}

bool ModManager::LoadMod(const std::string& modId) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    return LoadModInternal(modId, lock);
}

bool ModManager::LoadModInternal(const std::string& modId, std::unique_lock<std::mutex>& lock) {
    if (!m_Initialized) {
        SAGE_ERROR("ModManager: Not initialized");
        return false;
    }
    
    // Already loaded?
    if (m_LoadedMods.count(modId) > 0) {
        SAGE_WARN("ModManager: Mod '{}' already loaded", modId);
        return true;
    }
    
    // Find mod
    auto it = m_AvailableMods.find(modId);
    if (it == m_AvailableMods.end()) {
        TriggerModError(modId, "Mod not found in available mods", lock);
        return false;
    }
    
    auto modInfoPtr = it->second;
    auto& modInfo = *modInfoPtr;
    
    // Check if enabled
    if (!modInfo.enabled) {
        SAGE_WARN("ModManager: Mod '{}' is disabled", modId);
        return false;
    }
    
    // Validate dependencies
    std::vector<std::string> missingDeps;
    if (!ValidateDependencies(modId, missingDeps)) {
        std::string error = "Missing dependencies: ";
        for (size_t i = 0; i < missingDeps.size(); ++i) {
            error += missingDeps[i];
            if (i < missingDeps.size() - 1) error += ", ";
        }
        TriggerModError(modId, error, lock);
        return false;
    }
    
    // Load dependencies first
    for (const auto& dep : modInfo.dependencies) {
        bool isOptional = !dep.required;
        auto depIt = m_AvailableMods.find(dep.modId);
        if (isOptional) {
            if (depIt == m_AvailableMods.end() || !depIt->second->enabled) {
                continue;
            }
        } else if (depIt == m_AvailableMods.end()) {
            TriggerModError(modId, "Dependency not found: " + dep.modId, lock);
            return false;
        }

        if (depIt != m_AvailableMods.end() && !dep.IsSatisfiedBy(depIt->second->version)) {
            if (isOptional) {
                SAGE_WARN("ModManager: Optional dependency '{}' version mismatch for '{}'", dep.modId, modId);
                continue;
            }
            TriggerModError(modId, "Dependency version mismatch: " + dep.modId, lock);
            return false;
        }

        if (m_LoadedMods.count(dep.modId) == 0) {
            SAGE_INFO("ModManager: Loading dependency '{}' for '{}'", dep.modId, modId);
            if (!LoadModInternal(dep.modId, lock)) {
                TriggerModError(modId, "Failed to load dependency: " + dep.modId, lock);
                return false;
            }
        }
    }
    
    // Process asset overrides
    ProcessAssetOverrides(modInfo);
    
    // Mark as loaded
    m_LoadedMods.insert(modId);
    m_LoadOrder.push_back(modId);
    
    SAGE_INFO("ModManager: Loaded mod '{}' v{}", modInfo.name, modInfo.version.ToString());
    
    TriggerModLoaded(modInfo, lock);

    return m_LoadedMods.count(modId) > 0;
}

bool ModManager::LoadModFromPath(const std::string& modPath) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    
    std::string modJsonPath = modPath + "/mod.json";
    
    if (!fs::exists(modJsonPath)) {
        SAGE_ERROR("ModManager: No mod.json found in {}", modPath);
        return false;
    }
    
    auto modInfo = ModInfo::FromJSON(modJsonPath);
    if (!modInfo.has_value()) {
        SAGE_ERROR("ModManager: Failed to parse mod.json in {}", modPath);
        return false;
    }
    
    modInfo->path = modPath;
    
    if (!modInfo->IsValid()) {
        auto errors = modInfo->GetValidationErrors();
        SAGE_ERROR("ModManager: Invalid mod '{}': {}", modInfo->id, errors[0]);
        return false;
    }
    
    std::string modId = modInfo->id;
    m_AvailableMods[modId] = std::make_shared<ModInfo>(std::move(*modInfo));
    
    return LoadModInternal(modId, lock);
}

bool ModManager::UnloadMod(const std::string& modId) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    UnloadModInternal(modId, lock);
    return m_LoadedMods.count(modId) == 0;
}

void ModManager::UnloadModInternal(const std::string& modId, std::unique_lock<std::mutex>& lock) {
    if (m_LoadedMods.count(modId) == 0) {
        return;
    }
    
    auto it = m_AvailableMods.find(modId);
    if (it == m_AvailableMods.end()) {
        return;
    }
    
    auto modInfoPtr = it->second;
    auto& modInfo = *modInfoPtr;
    
    // Remove asset overrides
    for (auto& [originalPath, overrides] : m_AssetOverrides) {
        overrides.erase(
            std::remove_if(overrides.begin(), overrides.end(),
                [&](const AssetOverride& o) { return o.modId == modId; }),
            overrides.end()
        );
    }
    
    // Remove from loaded set
    m_LoadedMods.erase(modId);
    
    // Remove from load order
    m_LoadOrder.erase(
        std::remove(m_LoadOrder.begin(), m_LoadOrder.end(), modId),
        m_LoadOrder.end()
    );
    
    SAGE_INFO("ModManager: Unloaded mod '{}'", modInfo.name);

    TriggerModUnloaded(modInfo, lock);
}

bool ModManager::ReloadMod(const std::string& modId) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    
    UnloadModInternal(modId, lock);
    
    // Reload mod.json
    auto it = m_AvailableMods.find(modId);
    if (it != m_AvailableMods.end()) {
        std::string modJsonPath = it->second->path + "/mod.json";
        auto newInfo = ModInfo::FromJSON(modJsonPath);
        if (newInfo.has_value()) {
            newInfo->path = it->second->path;
            m_AvailableMods[modId] = std::make_shared<ModInfo>(std::move(*newInfo));
        }
    }
    
    return LoadModInternal(modId, lock);
}

int ModManager::LoadAllMods() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    
    if (!m_Initialized) {
        SAGE_ERROR("ModManager: Not initialized");
        return 0;
    }
    
    // Collect enabled mods
    std::vector<std::string> toLoad;
    for (const auto& [modId, modInfo] : m_AvailableMods) {
        if (modInfo->enabled && m_LoadedMods.count(modId) == 0) {
            toLoad.push_back(modId);
        }
    }
    
    // Resolve dependencies
    auto loadOrder = ResolveDependencies(toLoad);
    
    int loaded = 0;
    for (const auto& modId : loadOrder) {
        if (LoadModInternal(modId, lock)) {
            loaded++;
        }
    }
    
    SAGE_INFO("ModManager: Loaded {} / {} mods", loaded, toLoad.size());
    
    return loaded;
}

void ModManager::UnloadAllMods() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    
    auto loadedCopy = m_LoadedMods;
    for (const auto& modId : loadedCopy) {
        UnloadModInternal(modId, lock);
    }
}

// ============================================================================
// Mod Information
// ============================================================================

const ModInfo* ModManager::GetModInfo(const std::string& modId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AvailableMods.find(modId);
    if (it != m_AvailableMods.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> ModManager::GetLoadedMods() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_LoadOrder;
}

std::vector<std::string> ModManager::GetAvailableMods() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<std::string> mods;
    mods.reserve(m_AvailableMods.size());
    
    for (const auto& [modId, _] : m_AvailableMods) {
        mods.push_back(modId);
    }
    
    return mods;
}

bool ModManager::IsModLoaded(const std::string& modId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_LoadedMods.count(modId) > 0;
}

bool ModManager::IsModEnabled(const std::string& modId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AvailableMods.find(modId);
    if (it != m_AvailableMods.end()) {
        return it->second->enabled;
    }
    return false;
}

std::vector<std::string> ModManager::GetLoadOrder() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_LoadOrder;
}

// ============================================================================
// Mod Control
// ============================================================================

void ModManager::SetModEnabled(const std::string& modId, bool enable) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AvailableMods.find(modId);
    if (it != m_AvailableMods.end()) {
        it->second->enabled = enable;
        
        // Save to mod.json
        std::string modJsonPath = it->second->path + "/mod.json";
        it->second->ToJSON(modJsonPath);
    }
}

void ModManager::SetModPriority(const std::string& modId, int priority) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AvailableMods.find(modId);
    if (it != m_AvailableMods.end()) {
        it->second->priority = priority;
        
        // Save to mod.json
        std::string modJsonPath = it->second->path + "/mod.json";
        it->second->ToJSON(modJsonPath);
    }
}

// ============================================================================
// Asset Override System
// ============================================================================

std::string ModManager::ResolveAssetPath(const std::string& originalPath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AssetOverrides.find(originalPath);
    if (it != m_AssetOverrides.end() && !it->second.empty()) {
        // Return highest priority override
        return it->second.back().modPath;
    }
    
    return originalPath;
}

void ModManager::RegisterAssetOverride(const std::string& modId,
                                      const std::string& originalPath,
                                      const std::string& modAssetPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    RegisterAssetOverrideLocked(modId, originalPath, modAssetPath);
}

void ModManager::RegisterAssetOverrideLocked(const std::string& modId,
                                            const std::string& originalPath,
                                            const std::string& modAssetPath) {
    auto it = m_AvailableMods.find(modId);
    if (it == m_AvailableMods.end()) {
        return;
    }

    int priority = it->second->priority;

    AssetOverride override;
    override.modId = modId;
    override.modPath = modAssetPath;
    override.priority = priority;

    auto& overrides = m_AssetOverrides[originalPath];
    overrides.push_back(override);

    // Sort by priority
    std::sort(overrides.begin(), overrides.end(),
        [](const AssetOverride& a, const AssetOverride& b) {
            return a.priority < b.priority;
        });

    SAGE_INFO("ModManager: Registered asset override: {} -> {} (mod: {})",
             originalPath, modAssetPath, modId);
}

std::vector<std::string> ModManager::GetAssetOverrides(const std::string& originalPath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<std::string> result;
    
    auto it = m_AssetOverrides.find(originalPath);
    if (it != m_AssetOverrides.end()) {
        result.reserve(it->second.size());
        for (const auto& override : it->second) {
            result.push_back(override.modId);
        }
    }
    
    return result;
}

bool ModManager::HasAssetOverride(const std::string& originalPath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_AssetOverrides.find(originalPath);
    return it != m_AssetOverrides.end() && !it->second.empty();
}

void ModManager::ProcessAssetOverrides(const ModInfo& info) {
    for (const auto& [originalPath, modPath] : info.assetOverrides) {
        std::string fullModPath = info.path + "/" + modPath;
        RegisterAssetOverrideLocked(info.id, originalPath, fullModPath);
    }
}

// ============================================================================
// Hot-Reload System
// ============================================================================

void ModManager::EnableHotReload(bool enable) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (enable) {
        m_HotReloadEnabled = false;
        if (!m_HotReloadWarningIssued) {
            SAGE_WARN("ModManager: Hot-reload requested but not yet implemented; feature remains disabled");
            m_HotReloadWarningIssued = true;
        }
        return;
    }

    m_HotReloadEnabled = false;
    m_HotReloadWarningIssued = false;
    SAGE_INFO("ModManager: Hot-reload disabled");
}

void ModManager::CheckForChanges() {
    // TODO: Implement file watching and auto-reload
    // For now, this is a placeholder
}

void ModManager::SetHotReloadInterval(float seconds) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_HotReloadInterval = seconds;
}

// ============================================================================
// Event Callbacks
// ============================================================================

void ModManager::OnModLoaded(ModEventCallback callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_OnModLoaded.push_back(std::move(callback));
}

void ModManager::OnModUnloaded(ModEventCallback callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_OnModUnloaded.push_back(std::move(callback));
}

void ModManager::OnModError(ModErrorCallback callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_OnModError.push_back(std::move(callback));
}

void ModManager::TriggerModLoaded(const ModInfo& info, std::unique_lock<std::mutex>& lock) {
    auto callbacks = m_OnModLoaded;
    ModInfo copy = info;
    lock.unlock();
    for (auto& callback : callbacks) {
        if (callback) {
            callback(copy);
        }
    }
    lock.lock();
}

void ModManager::TriggerModUnloaded(const ModInfo& info, std::unique_lock<std::mutex>& lock) {
    auto callbacks = m_OnModUnloaded;
    ModInfo copy = info;
    lock.unlock();
    for (auto& callback : callbacks) {
        if (callback) {
            callback(copy);
        }
    }
    lock.lock();
}

void ModManager::TriggerModError(const std::string& modId, const std::string& error, std::unique_lock<std::mutex>& lock) {
    SAGE_ERROR("ModManager: Mod '{}' error: {}", modId, error);
    auto callbacks = m_OnModError;
    lock.unlock();
    for (auto& callback : callbacks) {
        if (callback) {
            callback(modId, error);
        }
    }
    lock.lock();
}

// ============================================================================
// Utility
// ============================================================================

ModManager::Statistics ModManager::GetStatistics() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    Statistics stats;
    stats.totalAvailable = static_cast<int>(m_AvailableMods.size());
    stats.totalLoaded = static_cast<int>(m_LoadedMods.size());
    
    for (const auto& [modId, modInfo] : m_AvailableMods) {
        if (modInfo->enabled) stats.totalEnabled++;
    }
    
    for (const auto& [path, overrides] : m_AssetOverrides) {
        stats.totalAssetOverrides += static_cast<int>(overrides.size());
    }
    
    return stats;
}

bool ModManager::ValidateDependencies(const std::string& modId, std::vector<std::string>& missingDeps) const {
    auto it = m_AvailableMods.find(modId);
    if (it == m_AvailableMods.end()) {
        missingDeps.push_back(modId);
        return false;
    }
    
    const auto& modInfo = *it->second;
    
    for (const auto& dep : modInfo.dependencies) {
        if (!dep.required) continue;
        
        auto depIt = m_AvailableMods.find(dep.modId);
        if (depIt == m_AvailableMods.end()) {
            missingDeps.push_back(dep.modId);
            continue;
        }
        
        // Check version
        if (!dep.IsSatisfiedBy(depIt->second->version)) {
            missingDeps.push_back(dep.modId + " (version mismatch)");
        }
    }
    
    return missingDeps.empty();
}

std::vector<std::string> ModManager::ResolveDependencies(const std::vector<std::string>& modIds) const {
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> inProgress;
    
    std::function<bool(const std::string&)> visit = [&](const std::string& modId) -> bool {
        if (visited.count(modId) > 0) return true;
        if (inProgress.count(modId) > 0) {
            SAGE_ERROR("ModManager: Circular dependency detected involving '{}'", modId);
            return false;
        }
        
        auto it = m_AvailableMods.find(modId);
        if (it == m_AvailableMods.end()) {
            return false;
        }
        
        inProgress.insert(modId);
        
        const auto& modInfo = *it->second;
        for (const auto& dep : modInfo.dependencies) {
            bool optional = !dep.required;
            auto depIt = m_AvailableMods.find(dep.modId);

            if (optional) {
                if (depIt == m_AvailableMods.end() || !depIt->second->enabled) {
                    continue;
                }
            } else if (depIt == m_AvailableMods.end()) {
                SAGE_ERROR("ModManager: Missing required dependency '{}' while resolving '{}'", dep.modId, modId);
                continue;
            }

            if (!visit(dep.modId)) {
                return false;
            }
        }
        
        inProgress.erase(modId);
        visited.insert(modId);
        result.push_back(modId);
        
        return true;
    };
    
    for (const auto& modId : modIds) {
        visit(modId);
    }
    
    auto dependsOn = [&](const std::string& source, const std::string& target) {
        if (source == target) {
            return false;
        }

        std::unordered_set<std::string> seen;
        std::function<bool(const std::string&)> dfs = [&](const std::string& current) -> bool {
            if (current == target) {
                return true;
            }
            if (!seen.insert(current).second) {
                return false;
            }

            auto itCurrent = m_AvailableMods.find(current);
            if (itCurrent == m_AvailableMods.end()) {
                return false;
            }

            const auto& info = *itCurrent->second;
            for (const auto& dep : info.dependencies) {
                bool optional = !dep.required;
                auto depIt = m_AvailableMods.find(dep.modId);
                if (optional) {
                    if (depIt == m_AvailableMods.end() || !depIt->second->enabled) {
                        continue;
                    }
                } else if (depIt == m_AvailableMods.end()) {
                    continue;
                }

                if (dfs(dep.modId)) {
                    return true;
                }
            }

            return false;
        };

        return dfs(source);
    };

    std::stable_sort(result.begin(), result.end(),
        [&](const std::string& a, const std::string& b) {
            if (dependsOn(a, b)) {
                return false;
            }
            if (dependsOn(b, a)) {
                return true;
            }

            auto itA = m_AvailableMods.find(a);
            auto itB = m_AvailableMods.find(b);
            int priorityA = (itA != m_AvailableMods.end()) ? itA->second->priority : 0;
            int priorityB = (itB != m_AvailableMods.end()) ? itB->second->priority : 0;

            if (priorityA == priorityB) {
                return a < b;
            }
            return priorityA < priorityB;
        });
    
    return result;
}

bool ModManager::HasCircularDependency(const std::string& modId, std::unordered_set<std::string>& visited) const {
    if (visited.count(modId) > 0) {
        return true; // Circular dependency detected
    }
    
    auto it = m_AvailableMods.find(modId);
    if (it == m_AvailableMods.end()) {
        return false;
    }
    
    visited.insert(modId);
    
    const auto& modInfo = *it->second;
    for (const auto& dep : modInfo.dependencies) {
        if (HasCircularDependency(dep.modId, visited)) {
            return true;
        }
    }
    
    visited.erase(modId);
    return false;
}

} // namespace SAGE::Modding
