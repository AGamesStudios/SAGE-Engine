#include "TextureManager.h"

#include "../Graphics/Texture.h"
#include "../Core/Logger.h"

#include <unordered_map>

namespace SAGE {

    namespace {
        std::unordered_map<std::string, Ref<Texture>> s_Textures;
    }

    Ref<Texture> TextureManager::Load(const std::string& name, const std::string& path) {
        // Защита от повторной загрузки
        if (auto it = s_Textures.find(name); it != s_Textures.end()) {
            SAGE_WARNING("[TextureManager] Текстура '{}' уже загружена, возвращаем существующую.", name);
            return it->second;
        }

        SAGE_INFO("[TextureManager] Загрузка текстуры '{}' из '{}'...", name, path);
        
        Ref<Texture> texture = CreateRef<Texture>(path);
        if (!texture || !texture->IsLoaded()) {
            SAGE_ERROR("[TextureManager] Не удалось загрузить текстуру '{}' из '{}'.", name, path);
            return texture;
        }

        s_Textures[name] = texture;
        SAGE_INFO("[TextureManager] Текстура '{}' загружена успешно ({}x{}).", 
                  name, texture->GetWidth(), texture->GetHeight());
        return texture;
    }

    Ref<Texture> TextureManager::Get(const std::string& name) {
        if (auto it = s_Textures.find(name); it != s_Textures.end()) {
            return it->second;
        }
        SAGE_WARNING("[TextureManager] Текстура '{}' не найдена.", name);
        return nullptr;
    }

    bool TextureManager::Exists(const std::string& name) {
        return s_Textures.find(name) != s_Textures.end();
    }

    void TextureManager::Unload(const std::string& name) {
        auto it = s_Textures.find(name);
        if (it != s_Textures.end()) {
            SAGE_INFO("[TextureManager] Выгрузка текстуры '{}'...", name);
            s_Textures.erase(it);
        }
    }

    void TextureManager::Clear() {
        SAGE_INFO("[TextureManager] Очистка всех текстур ({} загружено)...", s_Textures.size());
        s_Textures.clear();
    }

    size_t TextureManager::GetLoadedCount() {
        return s_Textures.size();
    }

    void TextureManager::UnloadUnused() {
        size_t unloadedCount = 0;
        for (auto it = s_Textures.begin(); it != s_Textures.end();) {
            // use_count == 1 означает, что текстура хранится только в кэше
            if (it->second.use_count() == 1) {
                SAGE_INFO("[TextureManager] Выгрузка неиспользуемой текстуры '{}'...", it->first);
                it = s_Textures.erase(it);
                ++unloadedCount;
            } else {
                ++it;
            }
        }
        if (unloadedCount > 0) {
            SAGE_INFO("[TextureManager] Выгружено {} неиспользуемых текстур.", unloadedCount);
        }
    }

    void TextureManager::LogStatus() {
        SAGE_INFO("[TextureManager] Загружено текстур: {}", s_Textures.size());
        for (const auto& [name, texture] : s_Textures) {
            SAGE_INFO("  - '{}': {}x{}, ref_count={}", 
                      name, 
                      texture->GetWidth(), 
                      texture->GetHeight(),
                      texture.use_count());
        }
    }

} // namespace SAGE
