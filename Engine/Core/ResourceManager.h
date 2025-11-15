#pragma once

#include <Core/IResource.h>
#include <Core/Logger.h>
#include <Core/FileSystem.h>
#include <Memory/Ref.h>
#include <Graphics/Core/Resources/Texture.h> // ensure Texture is complete for stub creation

#include <cstddef>
#include <future>
#include <list>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace SAGE {

using ResourceID = size_t;

/// @brief Priority for async resource loading
enum class LoadPriority {
    Low = 0,
    Normal = 1,
    High = 2
};

/// @brief Async load task (type-erased)
struct LoadTask {
    std::string path;
    LoadPriority priority = LoadPriority::Normal;
    std::function<void()> decodeFunc;     // Runs on worker thread (CPU decode)
    std::function<void()> uploadFunc;      // Runs on main thread (GPU upload)
    
    bool operator<(const LoadTask& other) const {
        // Higher priority = lower value in priority_queue (reversed)
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};


/// @brief Менеджер ресурсов с VRAM tracking и LRU cache
class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }
    
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    /// @brief Загрузить ресурс (с кэшированием)
    template<typename T>
    Ref<T> Load(const std::string& path) {
        static_assert(std::is_base_of_v<IResource, T>, "T must derive from IResource");
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        // Normalize & validate path
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) {
            SAGE_ERROR("ResourceManager: Invalid or unsafe path '{}'", path);
            return GetStub<T>();
        }
        // Use relative (from base) as key if base set
        std::string relativeKey = canonical;
        if (!m_BaseAssetsDir.empty()) {
            const auto baseAbs = FileSystem::NormalizePath(m_BaseAssetsDir);
            if (!baseAbs.empty() && relativeKey.rfind(baseAbs, 0) == 0) {
                relativeKey = relativeKey.substr(baseAbs.size());
                if (!relativeKey.empty() && (relativeKey[0] == '/' || relativeKey[0] == '\\')) {
                    relativeKey.erase(0,1);
                }
            }
        }
        ResourceID id = Hash(relativeKey);
        
        // Проверка кэша
        auto it = m_ResourceCache.find(id);
        if (it != m_ResourceCache.end()) {
            UpdateLRU(id);
            ++m_Hits;
            SAGE_INFO("ResourceManager: Cache hit for '{}'", relativeKey);
            return std::static_pointer_cast<T>(it->second);
        }
        ++m_Misses;
        
        // Проверка бюджета
        size_t estimatedSize = EstimateResourceSize<T>(canonical);
        if (m_CurrentGPUUsage + estimatedSize > m_MaxGPUMemory) {
            SAGE_WARNING("ResourceManager: GPU memory budget exceeded, evicting LRU resources");
            EvictLRU(estimatedSize);
        }
        
        // Загрузка ресурса
        SAGE_INFO("ResourceManager: Loading resource '{}'", relativeKey);
        Ref<T> resource = LoadResource<T>(canonical);
        
        if (!resource) {
            SAGE_ERROR("ResourceManager: Failed to load resource '{}'", relativeKey);
            return GetStub<T>();
        }
        
        // Добавление в кэш
        size_t actualSize = resource->GetGPUMemorySize();
        m_ResourceCache[id] = resource;
        m_LRUList.push_front(id);
        m_LRUMap[id] = m_LRUList.begin();
        m_CurrentGPUUsage += actualSize;
        // Post-load overshoot eviction if real size pushes over budget
        if (m_CurrentGPUUsage > m_MaxGPUMemory) {
            SAGE_WARNING("ResourceManager: Post-load budget exceeded ({}MB > {}MB), evicting...",
                         m_CurrentGPUUsage / (1024.0f * 1024.0f),
                         m_MaxGPUMemory / (1024.0f * 1024.0f));
            EvictLRU(0);
        }
        
        SAGE_TRACE("ResourceManager: Loaded '{}', GPU usage: {:.2f}MB / {:.2f}MB", 
                  relativeKey, 
                  m_CurrentGPUUsage / (1024.0f * 1024.0f),
                  m_MaxGPUMemory / (1024.0f * 1024.0f));
        
        return resource;
    }
    
    /// @brief Асинхронная загрузка ресурса с приоритетом
    /// @return Promise-подобный объект для ожидания результата
    template<typename T>
    std::future<Ref<T>> LoadAsync(const std::string& path, LoadPriority priority = LoadPriority::Normal) {
        static_assert(std::is_base_of_v<IResource, T>, "T must derive from IResource");
        
        auto promise = std::make_shared<std::promise<Ref<T>>>();
        std::future<Ref<T>> future = promise->get_future();
        
        LoadTask task;
        task.path = path;
        task.priority = priority;
        
        // Decode function runs on worker thread (CPU-intensive work)
        task.decodeFunc = [this, path, promise]() {
            try {
                // Perform CPU decode (file I/O, decompression, etc.)
                // This is resource-specific; for now, just prepare for upload
                SAGE_TRACE("ResourceManager: Async decode started for '{}'", path);
                // Real implementation would decode here without GPU calls
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };
        
        // Upload function runs on main thread (GPU work)
        task.uploadFunc = [this, path, promise]() {
            try {
                // Perform GPU upload on main thread
                Ref<T> resource = Load<T>(path);
                promise->set_value(resource);
                SAGE_TRACE("ResourceManager: Async upload completed for '{}'", path);
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };
        
        // Enqueue task
        {
            std::lock_guard<std::mutex> lock(m_AsyncMutex);
            m_LoadQueue.push(task);
        }
        m_AsyncCV.notify_one();
        
        return future;
    }
    
    /// @brief Process pending GPU uploads on main thread (call once per frame)
    void ProcessAsyncUploads() {
        std::lock_guard<std::mutex> lock(m_AsyncMutex);
        while (!m_UploadQueue.empty()) {
            auto& uploadFunc = m_UploadQueue.front();
            uploadFunc();  // Execute GPU upload
            m_UploadQueue.pop();
        }
    }

    
    /// @brief Выгрузить ресурс из кэша
    void Unload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) {
            return;
        }
        std::string relativeKey = canonical;
        if (!m_BaseAssetsDir.empty()) {
            const auto baseAbs = FileSystem::NormalizePath(m_BaseAssetsDir);
            if (!baseAbs.empty() && relativeKey.rfind(baseAbs, 0) == 0) {
                relativeKey = relativeKey.substr(baseAbs.size());
                if (!relativeKey.empty() && (relativeKey[0] == '/' || relativeKey[0] == '\\')) {
                    relativeKey.erase(0,1);
                }
            }
        }
        ResourceID id = Hash(relativeKey);
        
        auto it = m_ResourceCache.find(id);
        if (it == m_ResourceCache.end()) {
            return;
        }
        
        size_t size = it->second->GetGPUMemorySize();
        it->second->Unload();
        // Keep resource in cache but mark as unloaded (GPU memory freed)
        m_CurrentGPUUsage -= size;
        
        SAGE_TRACE("ResourceManager: Unloaded '{}', GPU usage: {:.2f}MB", 
                  relativeKey, m_CurrentGPUUsage / (1024.0f * 1024.0f));
    }
    
    /// @brief Очистить весь кэш
    void ClearCache() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        for (auto& [id, resource] : m_ResourceCache) {
            resource->Unload();
        }
        
        m_ResourceCache.clear();
        m_LRUList.clear();
        m_LRUMap.clear();
        m_CurrentGPUUsage = 0;
        
        SAGE_INFO("ResourceManager: Cache cleared");
    }
    
    /// @brief Перезагрузить ресурс (hot-reload)
    void Reload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) {
            return;
        }
        std::string relativeKey = canonical;
        if (!m_BaseAssetsDir.empty()) {
            const auto baseAbs = FileSystem::NormalizePath(m_BaseAssetsDir);
            if (!baseAbs.empty() && relativeKey.rfind(baseAbs, 0) == 0) {
                relativeKey = relativeKey.substr(baseAbs.size());
                if (!relativeKey.empty() && (relativeKey[0] == '/' || relativeKey[0] == '\\')) {
                    relativeKey.erase(0,1);
                }
            }
        }
        ResourceID id = Hash(relativeKey);
        auto it = m_ResourceCache.find(id);
        if (it != m_ResourceCache.end()) {
            const size_t oldSize = it->second->GetGPUMemorySize();
            SAGE_INFO("ResourceManager: Reloading '{}'", relativeKey);
            it->second->Reload();
            const size_t newSize = it->second->GetGPUMemorySize();
            if (newSize > oldSize) {
                m_CurrentGPUUsage += (newSize - oldSize);
                if (m_CurrentGPUUsage > m_MaxGPUMemory) {
                    SAGE_WARNING("ResourceManager: Budget exceeded after reload, evicting");
                    EvictLRU(0);
                }
            } else if (oldSize > newSize) {
                m_CurrentGPUUsage -= (oldSize - newSize);
            }
        }
    }

    /// @brief Разрешить или запретить загрузку GPU-ресурсов (используется тестами)
    void SetGpuLoadingEnabled(bool enabled) {
        m_EnableGpuResources = enabled;
        SAGE_INFO("ResourceManager: GPU resource loading {}", enabled ? "enabled" : "disabled");
    }

    /// @brief Проверить разрешена ли загрузка GPU-ресурсов
    [[nodiscard]] bool IsGpuLoadingEnabled() const {
        return m_EnableGpuResources;
    }
    
    /// @brief Установить максимальный бюджет GPU памяти
    void SetMaxGPUMemory(size_t bytes) {
        m_MaxGPUMemory = bytes;
        SAGE_INFO("ResourceManager: Max GPU memory set to {:.2f}MB", 
                  bytes / (1024.0f * 1024.0f));
    }
    
    /// @brief Получить текущее использование GPU памяти
    size_t GetCurrentGPUUsage() const { return m_CurrentGPUUsage; }
    
    /// @brief Получить максимальный бюджет GPU памяти
    size_t GetMaxGPUMemory() const { return m_MaxGPUMemory; }
    
    /// @brief Получить количество кэшированных ресурсов
    size_t GetCachedResourceCount() const { return m_ResourceCache.size(); }
    
    /// @brief Вывести статистику в лог
    void LogStats() const {
        SAGE_INFO("=== ResourceManager Statistics ===");
        SAGE_INFO("  Cached Resources: {}", m_ResourceCache.size());
        SAGE_INFO("  GPU Memory: {:.2f}MB / {:.2f}MB ({:.1f}%)",
                  m_CurrentGPUUsage / (1024.0f * 1024.0f),
                  m_MaxGPUMemory / (1024.0f * 1024.0f),
                  (m_CurrentGPUUsage * 100.0f) / m_MaxGPUMemory);
        SAGE_INFO("  Hits: {}  Misses: {}  HitRate: {:.1f}%", m_Hits, m_Misses, m_Hits + m_Misses == 0 ? 0.0 : (double)m_Hits * 100.0 / (double)(m_Hits + m_Misses));
        SAGE_INFO("===================================");
    }

    // Query API
    bool IsCached(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) return false;
        ResourceID id = Hash(canonical);
        return m_ResourceCache.find(id) != m_ResourceCache.end();
    }

    // Pin resource (prevent eviction)
    void Pin(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) return;
        m_Pinned.insert(Hash(canonical));
    }
    void Unpin(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::string canonical = FileSystem::NormalizePath(path, m_BaseAssetsDir);
        if (canonical.empty()) return;
        m_Pinned.erase(Hash(canonical));
    }

    // Set base assets directory for normalization (optional)
    void SetBaseAssetsDir(const std::string& dir) { m_BaseAssetsDir = dir; }

    // Eviction callback
    void SetEvictionCallback(std::function<void(const std::string&)> cb) { m_OnEvict = std::move(cb); }

private:
    ResourceManager() 
        : m_MaxGPUMemory(2ULL * 1024 * 1024 * 1024)  // 2GB по умолчанию
        , m_CurrentGPUUsage(0)
        , m_WorkerRunning(true)
    {
        // Start worker thread for async loading
        m_WorkerThread = std::thread(&ResourceManager::WorkerThreadLoop, this);
    }
    
    ~ResourceManager() {
        // Stop worker thread
        {
            std::lock_guard<std::mutex> lock(m_AsyncMutex);
            m_WorkerRunning = false;
        }
        m_AsyncCV.notify_all();
        if (m_WorkerThread.joinable()) {
            m_WorkerThread.join();
        }
        
        ClearCache();
    }
    
    /// @brief Worker thread loop for async resource loading
    void WorkerThreadLoop() {
        while (true) {
            LoadTask task;
            {
                std::unique_lock<std::mutex> lock(m_AsyncMutex);
                m_AsyncCV.wait(lock, [this] { 
                    return !m_LoadQueue.empty() || !m_WorkerRunning; 
                });
                
                if (!m_WorkerRunning && m_LoadQueue.empty()) {
                    break;  // Exit thread
                }
                
                if (m_LoadQueue.empty()) {
                    continue;
                }
                
                task = m_LoadQueue.top();
                m_LoadQueue.pop();
            }
            
            // Execute decode on worker thread (CPU work)
            if (task.decodeFunc) {
                task.decodeFunc();
            }
            
            // Queue upload for main thread (GPU work)
            if (task.uploadFunc) {
                std::lock_guard<std::mutex> lock(m_AsyncMutex);
                m_UploadQueue.push(task.uploadFunc);
            }
        }
    }
    
    /// @brief Хэш пути файла в ResourceID
    ResourceID Hash(const std::string& path) const {
        return std::hash<std::string>{}(path);
    }
    
    /// @brief Обновить позицию в LRU списке
    void UpdateLRU(ResourceID id) {
        auto it = m_LRUMap.find(id);
        if (it != m_LRUMap.end()) {
            m_LRUList.erase(it->second);
            m_LRUList.push_front(id);
            m_LRUMap[id] = m_LRUList.begin();
        }
    }
    
    /// @brief Выгрузить LRU ресурсы для освобождения памяти
    void EvictLRU(size_t requiredSize) {
        // Ensure sufficient memory is freed so that (currentUsage + requiredSize) <= maxBudget.
        // freedSize is tracked only for logging purposes.
        size_t freedSize = 0;
        while ((m_CurrentGPUUsage + requiredSize > m_MaxGPUMemory) && !m_LRUList.empty()) {
            ResourceID oldestId = m_LRUList.back();
            if (m_Pinned.count(oldestId)) { // skip pinned
                // Move pinned to front to avoid repeated checks
                m_LRUList.pop_back();
                m_LRUList.push_front(oldestId);
                m_LRUMap[oldestId] = m_LRUList.begin();
                continue;
            }
            auto cacheIt = m_ResourceCache.find(oldestId);
            if (cacheIt != m_ResourceCache.end()) {
                const size_t resSize = cacheIt->second->GetGPUMemorySize();
                const std::string path = cacheIt->second->GetPath();
                SAGE_INFO("ResourceManager: Evicting LRU resource '{}'", path);
                cacheIt->second->Unload();
                m_ResourceCache.erase(cacheIt);
                freedSize += resSize;
                if (m_CurrentGPUUsage >= resSize) {
                    m_CurrentGPUUsage -= resSize;
                } else {
                    m_CurrentGPUUsage = 0; // defensive
                }
                if (m_OnEvict) { m_OnEvict(path); }
            }
            m_LRUList.pop_back();
            m_LRUMap.erase(oldestId);
        }
        SAGE_INFO("ResourceManager: Evicted {:.2f}MB", freedSize / (1024.0f * 1024.0f));
    }
    
    /// @brief Оценить размер ресурса перед загрузкой
    template<typename T>
    size_t EstimateResourceSize([[maybe_unused]] const std::string& path) const {
        // Simple heuristic. Tests use MockResource of 1MB; default to 1MB to avoid over-eviction.
        // Real implementations can specialize or override via explicit template specializations.
        return 1 * 1024 * 1024;  // 1MB default
    }
    
    /// @brief Загрузить ресурс из файла (перегружается для каждого типа)
    template<typename T>
    Ref<T> LoadResource(const std::string& path);
    
    size_t m_MaxGPUMemory;      // Максимальный бюджет GPU памяти (байты)
    size_t m_CurrentGPUUsage;   // Текущее использование GPU памяти
    bool m_EnableGpuResources = true; // Флаг загрузки GPU (для headless режимов)
    size_t m_Hits = 0;
    size_t m_Misses = 0;
    std::string m_BaseAssetsDir;
    std::unordered_set<ResourceID> m_Pinned;
    std::function<void(const std::string&)> m_OnEvict;
    
    std::unordered_map<ResourceID, Ref<IResource>> m_ResourceCache;  // Кэш ресурсов
    std::list<ResourceID> m_LRUList;                                  // LRU список (front = newest)
    std::unordered_map<ResourceID, std::list<ResourceID>::iterator> m_LRUMap;  // Быстрый доступ к позиции в LRU
    
    mutable std::mutex m_Mutex;  // Thread-safety for resource cache
    
    // Async loading infrastructure
    std::priority_queue<LoadTask> m_LoadQueue;           // Pending decode tasks (priority queue)
    std::queue<std::function<void()>> m_UploadQueue;     // Pending GPU upload tasks
    std::thread m_WorkerThread;                          // Background worker for decode
    std::mutex m_AsyncMutex;                             // Thread-safety for async queues
    std::condition_variable m_AsyncCV;                   // Notify worker thread
    std::atomic<bool> m_WorkerRunning;                   // Worker thread running flag

    template<typename T>
    Ref<T> GetStub() {
        if constexpr (std::is_same_v<T, Texture>) {
            static Ref<Texture> stub;
            if (!stub) {
                unsigned char pixel[4] = {0,0,0,0};
                stub = CreateRef<Texture>(1u,1u, Texture::Format::RGBA8, pixel, false);
                stub->MarkStub();
            }
            return stub;
        } else {
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
            return nullptr;
#pragma warning(pop)
        }
    }
};

template<>
inline size_t ResourceManager::EstimateResourceSize<Texture>(const std::string& /*path*/) const {
    constexpr size_t estimatedWidth = 2048;
    constexpr size_t estimatedHeight = 2048;
    constexpr size_t bytesPerPixel = 4;
    return estimatedWidth * estimatedHeight * bytesPerPixel * 4 / 3;
}

} // namespace SAGE
