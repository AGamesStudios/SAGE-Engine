#pragma once

#include "Core/Core.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Memory/Ref.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace SAGE {

/// @brief Texture manager with caching and hot-reload support
class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    static TextureManager& Get();

    void Init();
    void Shutdown();

    Ref<Texture> Load(const std::string& name, const std::string& filepath);
    Ref<Texture> Get(const std::string& name) const;
    bool Reload(const std::string& name);
    void Remove(const std::string& name);
    void Clear();
    void UnloadUnused();
    size_t GetLoadedCount() const;
    bool IsLoaded(const std::string& name) const;

private:
    struct TextureEntry {
        Ref<Texture> texture;
        std::string filepath;
    };

    std::unordered_map<std::string, TextureEntry> m_Textures;
    bool m_Initialized = false;
    mutable std::mutex m_Mutex;
};

} // namespace SAGE
