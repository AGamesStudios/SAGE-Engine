#pragma once

#include "Core/Assets/FileWatcher.h"
#include "Core/ResourceManager.h"
#include "Core/Logger.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace SAGE {

/// @brief Asset metadata
struct AssetMetadata {
    std::string Path;
    std::string Type;  // "texture", "shader", "scene", etc.
    size_t FileSize = 0;
    std::filesystem::file_time_type LastModified;
    bool IsLoaded = false;
    
    AssetMetadata() = default;
    AssetMetadata(const std::string& path, const std::string& type)
        : Path(path), Type(type) {
        try {
            if (std::filesystem::exists(path)) {
                FileSize = std::filesystem::file_size(path);
                LastModified = std::filesystem::last_write_time(path);
            }
        } catch (...) {}
    }
};

/// @brief Centralized asset management
class AssetManager {
public:
    static AssetManager& Get() {
        static AssetManager instance;
        return instance;
    }
    
    /// @brief Инициализация asset manager
    void Initialize(const std::string& assetsRoot) {
        m_AssetsRoot = assetsRoot;
        
        if (!std::filesystem::exists(m_AssetsRoot)) {
            SAGE_ERROR("AssetManager: Assets root does not exist: {}", m_AssetsRoot);
            return;
        }
        
        ScanAssets();
        
        SAGE_INFO("AssetManager: Initialized with {} assets", m_Assets.size());
    }
    
    /// @brief Включить hot-reload
    void EnableHotReload() {
        HotReloadManager::Get().WatchDirectory(m_AssetsRoot);
        SAGE_INFO("AssetManager: Hot-reload enabled");
    }
    
    /// @brief Сканировать все ассеты в директории
    void ScanAssets() {
        m_Assets.clear();
        
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsRoot)) {
                if (!entry.is_regular_file()) continue;
                
                std::string path = entry.path().string();
                std::string extension = entry.path().extension().string();
                std::string type = GetAssetType(extension);
                
                if (!type.empty()) {
                    m_Assets[path] = AssetMetadata(path, type);
                }
            }
        } catch (const std::exception& e) {
            SAGE_ERROR("AssetManager: Scan error: {}", e.what());
        }
        
        SAGE_INFO("AssetManager: Found {} assets", m_Assets.size());
    }
    
    /// @brief Получить все ассеты определенного типа
    std::vector<std::string> GetAssetsByType(const std::string& type) const {
        std::vector<std::string> results;
        
        for (const auto& [path, metadata] : m_Assets) {
            if (metadata.Type == type) {
                results.push_back(path);
            }
        }
        
        return results;
    }
    
    /// @brief Получить metadata ассета
    const AssetMetadata* GetMetadata(const std::string& path) const {
        auto it = m_Assets.find(path);
        return (it != m_Assets.end()) ? &it->second : nullptr;
    }
    
    /// @brief Проверить существует ли ассет
    bool Exists(const std::string& path) const {
        return m_Assets.find(path) != m_Assets.end();
    }
    
    /// @brief Получить общий размер всех ассетов
    size_t GetTotalSize() const {
        size_t total = 0;
        for (const auto& [path, metadata] : m_Assets) {
            total += metadata.FileSize;
        }
        return total;
    }
    
    /// @brief Получить количество ассетов по типам
    std::map<std::string, size_t> GetAssetCountByType() const {
        std::map<std::string, size_t> counts;
        
        for (const auto& [path, metadata] : m_Assets) {
            counts[metadata.Type]++;
        }
        
        return counts;
    }
    
    /// @brief Логировать статистику
    void LogStats() const {
        SAGE_INFO("AssetManager Statistics:");
        SAGE_INFO("  Total Assets: {}", m_Assets.size());
        SAGE_INFO("  Total Size: {:.2f} MB", GetTotalSize() / (1024.0 * 1024.0));
        
        auto counts = GetAssetCountByType();
        for (const auto& [type, count] : counts) {
            SAGE_INFO("  {}: {}", type, count);
        }
    }
    
    /// @brief Экспортировать manifest (список всех ассетов)
    void ExportManifest(const std::string& outputPath) const {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            SAGE_ERROR("AssetManager: Failed to create manifest: {}", outputPath);
            return;
        }
        
        file << "# Asset Manifest\n";
        file << "# Total: " << m_Assets.size() << " assets\n\n";
        
        for (const auto& [path, metadata] : m_Assets) {
            file << metadata.Type << "\t" 
                 << metadata.FileSize << "\t" 
                 << path << "\n";
        }
        
        file.close();
        SAGE_INFO("AssetManager: Manifest exported to '{}'", outputPath);
    }

private:
    AssetManager() = default;
    
    std::string GetAssetType(const std::string& extension) const {
        if (extension == ".png" || extension == ".jpg" || 
            extension == ".jpeg" || extension == ".bmp" || extension == ".tga") {
            return "texture";
        }
        if (extension == ".shader" || extension == ".glsl" || 
            extension == ".vert" || extension == ".frag") {
            return "shader";
        }
        if (extension == ".json") {
            return "scene";
        }
        if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
            return "audio";
        }
        if (extension == ".ttf" || extension == ".otf") {
            return "font";
        }
        if (extension == ".obj" || extension == ".fbx" || extension == ".gltf") {
            return "model";
        }
        
        return "";  // Unknown type
    }

private:
    std::string m_AssetsRoot;
    std::map<std::string, AssetMetadata> m_Assets;
};

} // namespace SAGE
