#pragma once

#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Memory/Ref.h"

#include <filesystem>

namespace SAGE {

/// @brief Wrapper для Texture с поддержкой IResource
class ManagedTexture : public IResource {
public:
    ManagedTexture(const std::string& path)
        : m_Path(path)
        , m_Texture(nullptr)
        , m_Loaded(false)
    {
        Load();
    }
    
    ~ManagedTexture() override {
        Unload();
    }
    
    /// @brief Получить внутренний Texture объект
    Ref<Texture> GetTexture() const { return m_Texture; }
    
    /// @brief IResource interface
    size_t GetGPUMemorySize() const override {
        if (!m_Texture) return 0;
        
        // Расчёт размера текстуры в GPU
        size_t width = m_Texture->GetWidth();
        size_t height = m_Texture->GetHeight();
        size_t mipLevels = m_Texture->GetMipLevels();
        
        // RGBA8 = 4 bytes per pixel
        size_t baseSize = width * height * 4;
        
        // Mipmap chain добавляет ~33% памяти
        size_t totalSize = baseSize;
        if (mipLevels > 1) {
            totalSize = baseSize * 4 / 3;  // Геометрическая прогрессия 1 + 1/4 + 1/16 + ... ≈ 4/3
        }
        
        return totalSize;
    }
    
    const std::string& GetPath() const override {
        return m_Path;
    }
    
    bool Unload() noexcept override {
        if (!m_Texture) {
            m_Loaded = false;
            return true;
        }

        m_Texture.reset();
        m_Loaded = false;
        SAGE_INFO("ManagedTexture: Unloaded '{}'", m_Path);
        return true;
    }
    
    bool Reload() override {
        bool unloaded = Unload();
        Load();
        const bool loaded = IsLoaded();
        if (loaded) {
            SAGE_INFO("ManagedTexture: Reloaded '{}'", m_Path);
        }
        return unloaded && loaded;
    }
    
    bool IsLoaded() const override {
        return m_Loaded && m_Texture && m_Texture->IsLoaded();
    }

    ResourceState GetState() const override {
        return IsLoaded() ? ResourceState::Loaded : ResourceState::Unloaded;
    }

private:
    void Load() {
        if (!ResourceManager::Get().IsGpuLoadingEnabled()) {
            SAGE_TRACE("ManagedTexture: GPU loading disabled, skipping '{}'", m_Path);
            m_Texture.reset();
            m_Loaded = false;
            return;
        }

        try {
            m_Texture = CreateRef<Texture>(m_Path);
            m_Loaded = m_Texture->IsLoaded();
            
            if (m_Loaded) {
                SAGE_INFO("ManagedTexture: Loaded '{}' ({}x{}, {:.2f}MB)", 
                          m_Path, 
                          m_Texture->GetWidth(), 
                          m_Texture->GetHeight(),
                          GetGPUMemorySize() / (1024.0f * 1024.0f));
            } else {
                SAGE_ERROR("ManagedTexture: Failed to load '{}'", m_Path);
            }
        } catch (const std::exception& e) {
            SAGE_ERROR("ManagedTexture: Exception loading '{}': {}", m_Path, e.what());
            m_Loaded = false;
        }
    }
    
    std::string m_Path;
    Ref<Texture> m_Texture;
    bool m_Loaded;
};

// Специализация LoadResource для ManagedTexture
template<>
inline Ref<ManagedTexture> ResourceManager::LoadResource<ManagedTexture>(const std::string& path) {
    return CreateRef<ManagedTexture>(path);
}

template<>
inline size_t ResourceManager::EstimateResourceSize<ManagedTexture>(const std::string& path) const {
    namespace fs = std::filesystem;

    try {
        // C++20: используем конструктор path напрямую вместо устаревшего u8path
        const fs::path filePath(path);
        if (fs::exists(filePath)) {
            return static_cast<size_t>(fs::file_size(filePath));
        }
    } catch (const fs::filesystem_error& e) {
        SAGE_WARNING("ResourceManager: Unable to stat texture '{}': {}", path, e.what());
    }

    return 4 * 1024 * 1024; // Fallback heuristic
}

} // namespace SAGE
