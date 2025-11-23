#include "SAGE/Core/AssetManager.h"

namespace SAGE {

AssetManager& AssetManager::Get() {
    static AssetManager instance;
    return instance;
}

AssetManager::AssetRecord* AssetManager::GetRecord(const std::string& path) {
    auto it = m_Assets.find(path);
    return it != m_Assets.end() ? &it->second : nullptr;
}

const AssetManager::AssetRecord* AssetManager::GetRecord(const std::string& path) const {
    auto it = m_Assets.find(path);
    return it != m_Assets.end() ? &it->second : nullptr;
}

void AssetManager::Unload(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Assets.find(path);
    if (it == m_Assets.end()) {
        return;
    }

    if (it->second.unloadFunc) {
        it->second.unloadFunc();
    }

    if (m_TotalMemoryUsage >= it->second.info.sizeBytes) {
        m_TotalMemoryUsage -= it->second.info.sizeBytes;
    }

    m_Assets.erase(it);
}

void AssetManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& [_, record] : m_Assets) {
        if (record.unloadFunc) {
            record.unloadFunc();
        }
    }
    m_Assets.clear();
    m_TotalMemoryUsage = 0;
    ResourceManager::Get().UnloadAll();
}

AssetInfo AssetManager::GetAssetInfo(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    const auto* record = GetRecord(path);
    if (record) {
        return record->info;
    }
    return {};
}

std::vector<std::string> AssetManager::GetLoadedAssets() const {
    std::vector<std::string> assets;
    std::lock_guard<std::mutex> lock(m_Mutex);
    assets.reserve(m_Assets.size());
    for (const auto& [path, record] : m_Assets) {
        if (record.info.loaded) {
            assets.push_back(path);
        }
    }
    return assets;
}

size_t AssetManager::GetTotalMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_TotalMemoryUsage;
}

void AssetManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    ResourceManager::Get().CleanupUnused();

    for (auto it = m_Assets.begin(); it != m_Assets.end();) {
        if (!it->second.info.loaded) {
            it = m_Assets.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace SAGE
