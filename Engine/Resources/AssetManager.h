#pragma once

#include "../Core/Core.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Font.h"
#include "../Audio/Sound.h"
#include "../Core/Logger.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <future>
#include <functional>
#include <mutex>
#include <vector>

namespace SAGE {

    // Типы ассетов
    enum class AssetType {
        Texture,
        Shader,
        Sound,
        Font,
        Unknown
    };

    // Метаданные ассета
    struct AssetMetadata {
        std::string path;
        AssetType type;
        size_t memorySize = 0; // Размер в байтах
        bool isLoaded = false;
        bool isLoading = false; // Асинхронная загрузка в процессе
        float lastAccessTime = 0.0f;
    };

    // Колбэк для асинхронной загрузки
    template<typename T>
    using AssetLoadCallback = std::function<void(Ref<T>)>;

    // Централизованный менеджер ассетов
    class AssetManager {
    public:
        // Инициализация
        static void Init();
        static void Shutdown();

        // Загрузка ассетов
        static Ref<Texture> LoadTexture(const std::string& name, const std::string& path);
        static Ref<Shader> LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        static Ref<Sound> LoadSound(const std::string& name, const std::string& path, bool streaming = false);
        static Ref<Font> LoadFont(const std::string& name, const std::string& path, unsigned int fontSize = 48);

        // Асинхронная загрузка (для больших ассетов)
        static void LoadTextureAsync(const std::string& name, const std::string& path, AssetLoadCallback<Texture> callback = nullptr);
        static void LoadSoundAsync(const std::string& name, const std::string& path, bool streaming, AssetLoadCallback<Sound> callback = nullptr);
        
        // Проверка готовности асинхронной загрузки
        static bool IsAssetLoading(const std::string& name);
        static void ProcessAsyncLoads(); // Вызывать каждый кадр для обработки завершённых загрузок

        // Получение уже загруженных ассетов
        static Ref<Texture> GetTexture(const std::string& name);
        static Ref<Shader> GetShader(const std::string& name);
        static Ref<Sound> GetSound(const std::string& name);
        static Ref<Font> GetFont(const std::string& name);

        // Проверка существования
        static bool HasTexture(const std::string& name);
        static bool HasShader(const std::string& name);
        static bool HasSound(const std::string& name);
        static bool HasFont(const std::string& name);

        // Выгрузка ассетов
        static void UnloadTexture(const std::string& name);
        static void UnloadShader(const std::string& name);
        static void UnloadSound(const std::string& name);
        static void UnloadFont(const std::string& name);
        static void UnloadAll();

        // Метаданные и статистика
        static AssetMetadata GetMetadata(const std::string& name, AssetType type);
        static size_t GetTotalMemoryUsage();
        static size_t GetAssetCount(AssetType type);
        static void PrintStatistics();

        // Путь к папке ассетов
        static void SetAssetDirectory(const std::string& directory);
        static std::string GetAssetDirectory();

        // Автоматическое определение типа по расширению
        static AssetType GetAssetTypeFromExtension(const std::string& path);

    private:
        AssetManager() = default;

        static std::unordered_map<std::string, Ref<Texture>> s_Textures;
        static std::unordered_map<std::string, Ref<Shader>> s_Shaders;
        static std::unordered_map<std::string, Ref<Sound>> s_Sounds;
        static std::unordered_map<std::string, Ref<Font>> s_Fonts;

        static std::unordered_map<std::string, AssetMetadata> s_Metadata;
        static std::string s_AssetDirectory;
        static bool s_Initialized;

        // Асинхронная загрузка
        struct AsyncLoadTask {
            std::future<void> future;
            std::function<void()> onComplete;
        };
        static std::vector<AsyncLoadTask> s_AsyncTasks;
        static std::mutex s_AsyncMutex;

        // Вспомогательные функции
        static std::string ResolvePath(const std::string& path);
        static void UpdateMetadata(const std::string& name, AssetType type, const std::string& path, size_t memorySize);
    };

} // namespace SAGE
