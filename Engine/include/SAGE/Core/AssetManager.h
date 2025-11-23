#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <future>
#include <filesystem>
#include <typeindex>

#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Log.h"

namespace SAGE {

// Asset metadata
struct AssetInfo {
    std::string path;
    std::string type;
    size_t sizeBytes = 0;
    bool loaded = false;
    float loadProgress = 0.0f;
};

// Asset loading callback
using LoadCallback = std::function<void(bool success, const std::string& error)>;

// Asset Manager - improved resource management
class AssetManager {
public:
    static AssetManager& Get();
    
    // Synchronous loading
    template<typename T>
    std::shared_ptr<T> Load(const std::string& path);
    
    // Asynchronous loading (queued)
    template<typename T>
    void LoadAsync(const std::string& path, LoadCallback callback = nullptr);
    
    // Unload asset
    void Unload(const std::string& path);
    
    // Unload all assets
    void UnloadAll();
    
    // Get asset info
    AssetInfo GetAssetInfo(const std::string& path) const;
    
    // Get all loaded assets
    std::vector<std::string> GetLoadedAssets() const;
    
    // Memory management
    size_t GetTotalMemoryUsage() const;
    void SetMemoryBudget(size_t bytes) { m_MemoryBudget = bytes; }
    size_t GetMemoryBudget() const { return m_MemoryBudget; }
    
    // Cache control
    void SetCacheEnabled(bool enabled) { m_CacheEnabled = enabled; }
    bool IsCacheEnabled() const { return m_CacheEnabled; }
    void ClearCache();

private:
    AssetManager() = default;
    
    struct AssetRecord {
        AssetInfo info;
        std::function<void()> unloadFunc;
        std::type_index type = typeid(void);
    };
    
    AssetRecord* GetRecord(const std::string& path);
    const AssetRecord* GetRecord(const std::string& path) const;
    
    size_t m_MemoryBudget = 512 * 1024 * 1024; // 512 MB default
    bool m_CacheEnabled = true;
    size_t m_TotalMemoryUsage = 0;
    
    std::unordered_map<std::string, AssetRecord> m_Assets;
    mutable std::mutex m_Mutex;
};

// --------------------------------------------
// Template Implementations
// --------------------------------------------

template<typename T>
std::shared_ptr<T> AssetManager::Load(const std::string& path) {
    if (path.empty()) {
        SAGE_WARN("AssetManager: пустой путь ресурса");
        return nullptr;
    }

    auto& self = Get();
    std::lock_guard<std::mutex> lock(self.m_Mutex);

    auto resource = ResourceManager::Get().Load<T>(path);

    AssetRecord& record = self.m_Assets[path];
    record.info.path = path;
    record.info.type = typeid(T).name();
    record.info.loaded = static_cast<bool>(resource);
    record.info.loadProgress = resource ? 1.0f : 0.0f;
    record.type = typeid(T);

    std::error_code ec;
    record.info.sizeBytes = std::filesystem::exists(path, ec)
        ? static_cast<size_t>(std::filesystem::file_size(path, ec))
        : 0;
    record.unloadFunc = [p = path]() { ResourceManager::Get().template Unload<T>(p); };

    // Track memory usage for reporting (best-effort, file size as proxy)
    self.m_TotalMemoryUsage = 0;
    for (const auto& [_, rec] : self.m_Assets) {
        if (rec.info.loaded) {
            self.m_TotalMemoryUsage += rec.info.sizeBytes;
        }
    }

    return resource;
}

template<typename T>
void AssetManager::LoadAsync(const std::string& path, LoadCallback callback) {
    std::async(std::launch::async, [path, callback]() {
        auto resource = AssetManager::Get().Load<T>(path);
        if (callback) {
            const bool ok = static_cast<bool>(resource);
            callback(ok, ok ? "" : "Failed to load asset");
        }
    });
}

} // namespace SAGE
