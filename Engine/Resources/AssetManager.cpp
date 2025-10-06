#include "AssetManager.h"
#include "../Core/Logger.h"
#include <algorithm>

namespace SAGE {

    // Статические члены
    std::unordered_map<std::string, Ref<Texture>> AssetManager::s_Textures;
    std::unordered_map<std::string, Ref<Shader>> AssetManager::s_Shaders;
    std::unordered_map<std::string, Ref<Sound>> AssetManager::s_Sounds;
    std::unordered_map<std::string, Ref<Font>> AssetManager::s_Fonts;
    std::unordered_map<std::string, AssetMetadata> AssetManager::s_Metadata;
    std::string AssetManager::s_AssetDirectory = "Assets/";
    bool AssetManager::s_Initialized = false;
    std::vector<AssetManager::AsyncLoadTask> AssetManager::s_AsyncTasks;
    std::mutex AssetManager::s_AsyncMutex;

    void AssetManager::Init() {
        if (s_Initialized) {
            SAGE_WARNING("AssetManager уже инициализирован");
            return;
        }

        SAGE_INFO("AssetManager инициализирован");
        SAGE_INFO("Директория ассетов: {}", s_AssetDirectory);
        s_Initialized = true;
    }

    void AssetManager::Shutdown() {
        if (!s_Initialized) {
            return;
        }

        SAGE_INFO("AssetManager: Выгрузка всех ассетов...");
        UnloadAll();
        s_Initialized = false;
        SAGE_INFO("AssetManager завершён");
    }

    void AssetManager::SetAssetDirectory(const std::string& directory) {
        s_AssetDirectory = directory;
        if (!s_AssetDirectory.empty() && s_AssetDirectory.back() != '/' && s_AssetDirectory.back() != '\\') {
            s_AssetDirectory += '/';
        }
        SAGE_INFO("Директория ассетов изменена на: {}", s_AssetDirectory);
    }

    std::string AssetManager::GetAssetDirectory() {
        return s_AssetDirectory;
    }

    std::string AssetManager::ResolvePath(const std::string& path) {
        // Если путь абсолютный или начинается с Assets/, возвращаем как есть
        if (path.size() >= 2 && path[1] == ':') { // Windows absolute path
            return path;
        }
        if (path.find("Assets/") == 0 || path.find("Assets\\") == 0) {
            return path;
        }
        // Иначе добавляем базовую директорию
        return s_AssetDirectory + path;
    }

    AssetType AssetManager::GetAssetTypeFromExtension(const std::string& path) {
        std::filesystem::path p(path);
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
            return AssetType::Texture;
        }
        else if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".shader") {
            return AssetType::Shader;
        }
        else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
            return AssetType::Sound;
        }
        else if (ext == ".ttf" || ext == ".otf") {
            return AssetType::Font;
        }

        return AssetType::Unknown;
    }

    void AssetManager::UpdateMetadata(const std::string& name, AssetType type, const std::string& path, size_t memorySize) {
        AssetMetadata metadata;
        metadata.path = path;
        metadata.type = type;
        metadata.memorySize = memorySize;
        metadata.isLoaded = true;
        metadata.lastAccessTime = 0.0f; // Можно подключить Timer для реального времени

        s_Metadata[name] = metadata;
    }

    // ============ ТЕКСТУРЫ ============

    Ref<Texture> AssetManager::LoadTexture(const std::string& name, const std::string& path) {
        // Проверка на дубликат
        if (HasTexture(name)) {
            SAGE_WARNING("Текстура '{}' уже загружена, возвращаем существующую", name);
            return GetTexture(name);
        }

        std::string fullPath = ResolvePath(path);
        auto texture = CreateRef<Texture>(fullPath);

        if (!texture) {
            SAGE_ERROR("Не удалось загрузить текстуру: {}", fullPath);
            return nullptr;
        }

        s_Textures[name] = texture;

        // Примерный размер: width * height * 4 (RGBA)
        size_t estimatedSize = texture->GetWidth() * texture->GetHeight() * 4;
        UpdateMetadata(name, AssetType::Texture, fullPath, estimatedSize);

        SAGE_INFO("Текстура загружена: {} ({}x{}, ~{} KB)", name, 
                  texture->GetWidth(), texture->GetHeight(), estimatedSize / 1024);

        return texture;
    }

    Ref<Texture> AssetManager::GetTexture(const std::string& name) {
        auto it = s_Textures.find(name);
        if (it != s_Textures.end()) {
            return it->second;
        }
        SAGE_WARNING("Текстура '{}' не найдена", name);
        return nullptr;
    }

    bool AssetManager::HasTexture(const std::string& name) {
        return s_Textures.find(name) != s_Textures.end();
    }

    void AssetManager::UnloadTexture(const std::string& name) {
        auto it = s_Textures.find(name);
        if (it != s_Textures.end()) {
            SAGE_INFO("Выгрузка текстуры: {}", name);
            s_Textures.erase(it);
            s_Metadata.erase(name);
        }
    }

    // ============ ШЕЙДЕРЫ ============

    Ref<Shader> AssetManager::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        if (HasShader(name)) {
            SAGE_WARNING("Шейдер '{}' уже загружен, возвращаем существующий", name);
            return GetShader(name);
        }

        std::string vertPath = ResolvePath(vertexPath);
        std::string fragPath = ResolvePath(fragmentPath);

        auto shader = CreateRef<Shader>(vertPath, fragPath);

        if (!shader) {
            SAGE_ERROR("Не удалось загрузить шейдер: {} + {}", vertPath, fragPath);
            return nullptr;
        }

        s_Shaders[name] = shader;

        // Шейдеры малы, примерно 1-10 KB
        size_t estimatedSize = 5 * 1024;
        UpdateMetadata(name, AssetType::Shader, vertPath + " + " + fragPath, estimatedSize);

        SAGE_INFO("Шейдер загружен: {}", name);

        return shader;
    }

    Ref<Shader> AssetManager::GetShader(const std::string& name) {
        auto it = s_Shaders.find(name);
        if (it != s_Shaders.end()) {
            return it->second;
        }
        SAGE_WARNING("Шейдер '{}' не найден", name);
        return nullptr;
    }

    bool AssetManager::HasShader(const std::string& name) {
        return s_Shaders.find(name) != s_Shaders.end();
    }

    void AssetManager::UnloadShader(const std::string& name) {
        auto it = s_Shaders.find(name);
        if (it != s_Shaders.end()) {
            SAGE_INFO("Выгрузка шейдера: {}", name);
            s_Shaders.erase(it);
            s_Metadata.erase(name);
        }
    }

    // ============ ЗВУКИ ============

    Ref<Sound> AssetManager::LoadSound(const std::string& name, const std::string& path, bool streaming) {
        if (HasSound(name)) {
            SAGE_WARNING("Звук '{}' уже загружен, возвращаем существующий", name);
            return GetSound(name);
        }

        std::string fullPath = ResolvePath(path);
        auto sound = CreateRef<Sound>(fullPath, streaming);

        if (!sound) {
            SAGE_ERROR("Не удалось загрузить звук: {}", fullPath);
            return nullptr;
        }

        s_Sounds[name] = sound;

        // Звуки могут быть большими (1-10 MB для музыки)
        size_t estimatedSize = streaming ? 100 * 1024 : 1024 * 1024; // Streaming меньше в памяти
        UpdateMetadata(name, AssetType::Sound, fullPath, estimatedSize);

        SAGE_INFO("Звук загружен: {} (streaming: {})", name, streaming);

        return sound;
    }

    Ref<Sound> AssetManager::GetSound(const std::string& name) {
        auto it = s_Sounds.find(name);
        if (it != s_Sounds.end()) {
            return it->second;
        }
        SAGE_WARNING("Звук '{}' не найден", name);
        return nullptr;
    }

    bool AssetManager::HasSound(const std::string& name) {
        return s_Sounds.find(name) != s_Sounds.end();
    }

    void AssetManager::UnloadSound(const std::string& name) {
        auto it = s_Sounds.find(name);
        if (it != s_Sounds.end()) {
            SAGE_INFO("Выгрузка звука: {}", name);
            s_Sounds.erase(it);
            s_Metadata.erase(name);
        }
    }

    // ============ ШРИФТЫ ============

    Ref<Font> AssetManager::LoadFont(const std::string& name, const std::string& path, unsigned int fontSize) {
        if (HasFont(name)) {
            SAGE_WARNING("Шрифт '{}' уже загружен, возвращаем существующий", name);
            return GetFont(name);
        }

        std::string fullPath = ResolvePath(path);
        auto font = CreateRef<Font>(fullPath, fontSize);

        if (!font) {
            SAGE_ERROR("Не удалось загрузить шрифт: {}", fullPath);
            return nullptr;
        }

        s_Fonts[name] = font;

        // Шрифты: примерно 128 символов * 64x64 * 4 байта
        size_t estimatedSize = 128 * 64 * 64 * 4;
        UpdateMetadata(name, AssetType::Font, fullPath, estimatedSize);

        SAGE_INFO("Шрифт загружен: {} (размер: {})", name, fontSize);

        return font;
    }

    Ref<Font> AssetManager::GetFont(const std::string& name) {
        auto it = s_Fonts.find(name);
        if (it != s_Fonts.end()) {
            return it->second;
        }
        SAGE_WARNING("Шрифт '{}' не найден", name);
        return nullptr;
    }

    bool AssetManager::HasFont(const std::string& name) {
        return s_Fonts.find(name) != s_Fonts.end();
    }

    void AssetManager::UnloadFont(const std::string& name) {
        auto it = s_Fonts.find(name);
        if (it != s_Fonts.end()) {
            SAGE_INFO("Выгрузка шрифта: {}", name);
            s_Fonts.erase(it);
            s_Metadata.erase(name);
        }
    }

    // ============ ОБЩИЕ ФУНКЦИИ ============

    void AssetManager::UnloadAll() {
        SAGE_INFO("Выгрузка {} текстур", s_Textures.size());
        s_Textures.clear();

        SAGE_INFO("Выгрузка {} шейдеров", s_Shaders.size());
        s_Shaders.clear();

        SAGE_INFO("Выгрузка {} звуков", s_Sounds.size());
        s_Sounds.clear();

        SAGE_INFO("Выгрузка {} шрифтов", s_Fonts.size());
        s_Fonts.clear();

        s_Metadata.clear();
    }

    AssetMetadata AssetManager::GetMetadata(const std::string& name, AssetType type) {
        auto it = s_Metadata.find(name);
        if (it != s_Metadata.end()) {
            // Проверяем что тип соответствует запрошенному
            if (it->second.type == type) {
                return it->second;
            }
            SAGE_WARNING("Тип ассета '{}' не соответствует запрошенному", name);
        }
        return AssetMetadata{}; // Пустая метаданные
    }

    size_t AssetManager::GetTotalMemoryUsage() {
        size_t total = 0;
        for (const auto& [name, metadata] : s_Metadata) {
            total += metadata.memorySize;
        }
        return total;
    }

    size_t AssetManager::GetAssetCount(AssetType type) {
        switch (type) {
            case AssetType::Texture: return s_Textures.size();
            case AssetType::Shader:  return s_Shaders.size();
            case AssetType::Sound:   return s_Sounds.size();
            case AssetType::Font:    return s_Fonts.size();
            default: return 0;
        }
    }

    void AssetManager::PrintStatistics() {
        SAGE_INFO("========== Asset Manager Statistics ==========");
        SAGE_INFO("Текстуры: {}", s_Textures.size());
        SAGE_INFO("Шейдеры:  {}", s_Shaders.size());
        SAGE_INFO("Звуки:    {}", s_Sounds.size());
        SAGE_INFO("Шрифты:   {}", s_Fonts.size());
        
        size_t totalMemory = GetTotalMemoryUsage();
        SAGE_INFO("Общая память: {:.2f} MB", totalMemory / (1024.0f * 1024.0f));
        SAGE_INFO("==============================================");
    }

    // ============ АСИНХРОННАЯ ЗАГРУЗКА ============

    void AssetManager::LoadTextureAsync(const std::string& name, const std::string& path, AssetLoadCallback<Texture> callback) {
        if (HasTexture(name)) {
            SAGE_WARNING("Текстура '{}' уже загружена", name);
            if (callback) callback(GetTexture(name));
            return;
        }

        // Отметить как загружающуюся
        AssetMetadata metadata;
        metadata.path = ResolvePath(path);
        metadata.type = AssetType::Texture;
        metadata.isLoading = true;
        s_Metadata[name] = metadata;

        // Запустить асинхронную загрузку
        AsyncLoadTask task;
        task.future = std::async(std::launch::async, [name, path, callback]() {
            std::string fullPath = ResolvePath(path);
            auto texture = CreateRef<Texture>(fullPath);

            // После загрузки - сохранить в основной поток через onComplete
            if (texture) {
                SAGE_INFO("Асинхронная загрузка текстуры '{}' завершена", name);
            } else {
                SAGE_ERROR("Асинхронная загрузка текстуры '{}' не удалась", name);
            }

            // onComplete будет вызван в основном потоке через ProcessAsyncLoads
        });

        task.onComplete = [name, path, callback]() {
            std::string fullPath = ResolvePath(path);
            auto texture = CreateRef<Texture>(fullPath);
            
            if (texture) {
                std::lock_guard<std::mutex> lock(s_AsyncMutex);
                s_Textures[name] = texture;
                
                // Обновляем метаданные сразу внутри критической секции
                size_t estimatedSize = texture->GetWidth() * texture->GetHeight() * 4;
                AssetMetadata& meta = s_Metadata[name];
                meta.path = fullPath;
                meta.type = AssetType::Texture;
                meta.memorySize = estimatedSize;
                meta.isLoading = false;
                meta.isLoaded = true;
                meta.lastAccessTime = 0.0f;
                
                if (callback) callback(texture);
            } else {
                std::lock_guard<std::mutex> lock(s_AsyncMutex);
                s_Metadata.erase(name);
            }
        };

        std::lock_guard<std::mutex> lock(s_AsyncMutex);
        s_AsyncTasks.push_back(std::move(task));
    }

    void AssetManager::LoadSoundAsync(const std::string& name, const std::string& path, bool streaming, AssetLoadCallback<Sound> callback) {
        if (HasSound(name)) {
            SAGE_WARNING("Звук '{}' уже загружен", name);
            if (callback) callback(GetSound(name));
            return;
        }

        AssetMetadata metadata;
        metadata.path = ResolvePath(path);
        metadata.type = AssetType::Sound;
        metadata.isLoading = true;
        s_Metadata[name] = metadata;

        AsyncLoadTask task;
        task.future = std::async(std::launch::async, [name, path, streaming, callback]() {
            std::string fullPath = ResolvePath(path);
            auto sound = CreateRef<Sound>(fullPath, streaming);

            if (sound) {
                SAGE_INFO("Асинхронная загрузка звука '{}' завершена", name);
            } else {
                SAGE_ERROR("Асинхронная загрузка звука '{}' не удалась", name);
            }
        });

        task.onComplete = [name, path, streaming, callback]() {
            std::string fullPath = ResolvePath(path);
            auto sound = CreateRef<Sound>(fullPath, streaming);
            
            if (sound) {
                std::lock_guard<std::mutex> lock(s_AsyncMutex);
                s_Sounds[name] = sound;
                
                // Обновляем метаданные сразу внутри критической секции
                size_t estimatedSize = streaming ? 100 * 1024 : 1024 * 1024;
                AssetMetadata& meta = s_Metadata[name];
                meta.path = fullPath;
                meta.type = AssetType::Sound;
                meta.memorySize = estimatedSize;
                meta.isLoading = false;
                meta.isLoaded = true;
                meta.lastAccessTime = 0.0f;
                
                if (callback) callback(sound);
            } else {
                std::lock_guard<std::mutex> lock(s_AsyncMutex);
                s_Metadata.erase(name);
            }
        };

        std::lock_guard<std::mutex> lock(s_AsyncMutex);
        s_AsyncTasks.push_back(std::move(task));
    }

    bool AssetManager::IsAssetLoading(const std::string& name) {
        auto it = s_Metadata.find(name);
        if (it != s_Metadata.end()) {
            return it->second.isLoading;
        }
        return false;
    }

    void AssetManager::ProcessAsyncLoads() {
        std::lock_guard<std::mutex> lock(s_AsyncMutex);
        
        // Обработать завершённые задачи
        s_AsyncTasks.erase(
            std::remove_if(s_AsyncTasks.begin(), s_AsyncTasks.end(), [](AsyncLoadTask& task) {
                if (task.future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                    // Задача завершена - вызвать колбэк в основном потоке
                    if (task.onComplete) {
                        task.onComplete();
                    }
                    return true; // Удалить из списка
                }
                return false; // Оставить в списке
            }),
            s_AsyncTasks.end()
        );
    }

} // namespace SAGE
