#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Log.h"

namespace SAGE {

ResourceManager& ResourceManager::Get() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    SAGE_INFO("Unloading all resources ({} cached)", m_Resources.size());
    m_Resources.clear();
}

void ResourceManager::CleanupUnused() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    size_t before = m_Resources.size();
    
    for (auto it = m_Resources.begin(); it != m_Resources.end();) {
        if (it->second.expired()) {
            it = m_Resources.erase(it);
        } else {
            ++it;
        }
    }
    
    size_t removed = before - m_Resources.size();
    if (removed > 0) {
        SAGE_INFO("Cleaned up {} unused resources", removed);
    }
}

} // namespace SAGE
