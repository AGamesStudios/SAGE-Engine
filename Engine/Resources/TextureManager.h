#pragma once

#include <string>
#include <unordered_map>

#include "../Memory/Ref.h"

namespace SAGE {

    class Texture;

    class TextureManager {
    public:
        static Ref<Texture> Load(const std::string& name, const std::string& path);
        static Ref<Texture> Get(const std::string& name);
        static bool Exists(const std::string& name);
        static void Unload(const std::string& name);
        static void Clear();
        
        // Новые функции для управления ресурсами
        static size_t GetLoadedCount();
        static void UnloadUnused(); // Выгрузить текстуры с use_count == 1 (только в кэше)
        static void LogStatus();    // Лог всех загруженных текстур
    };

} // namespace SAGE
