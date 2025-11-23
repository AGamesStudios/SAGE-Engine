#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <mutex>

namespace SAGE {

// Resource interface
class IResource {
public:
    virtual ~IResource() = default;
    virtual bool Load(const std::string& path) = 0;
    virtual void Unload() = 0;
    virtual bool IsLoaded() const = 0;
    virtual const std::string& GetPath() const = 0;
};

class ResourceManager {
public:
    static ResourceManager& Get();

    template<typename T>
    std::shared_ptr<T> Load(const std::string& path, std::function<void(std::shared_ptr<T>)> config = nullptr) {
        static_assert(std::is_base_of<IResource, T>::value, 
                      "T must inherit from IResource");

        std::lock_guard<std::mutex> lock(m_Mutex);

        auto typeIndex = std::type_index(typeid(T));
        auto key = MakeKey(typeIndex, path);

        // Check cache
        auto it = m_Resources.find(key);
        if (it != m_Resources.end()) {
            if (auto resource = std::static_pointer_cast<T>(it->second.lock())) {
                return resource;
            }
            // Expired, remove from cache
            m_Resources.erase(it);
        }

        // Load new resource
        // Note: Loading might take time, but we hold the lock to prevent race on map insertion
        // If we want to allow parallel loading, we should release lock during Load()
        // But then we need to handle double-loading race condition.
        // For now, simple thread safety is enough.
        auto resource = std::make_shared<T>();
        if (config) {
            config(resource);
        }
        
        if (!resource->Load(path)) {
            return nullptr;
        }

        m_Resources[key] = resource;
        return resource;
    }

    template<typename T>
    void Unload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto typeIndex = std::type_index(typeid(T));
        auto key = MakeKey(typeIndex, path);
        m_Resources.erase(key);
    }

    void UnloadAll();
    void CleanupUnused();

    size_t GetLoadedCount() const { 
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Resources.size(); 
    }

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::string MakeKey(std::type_index type, const std::string& path) const {
        return std::to_string(type.hash_code()) + ":" + path;
    }

    std::unordered_map<std::string, std::weak_ptr<IResource>> m_Resources;
    mutable std::mutex m_Mutex;
};

} // namespace SAGE
