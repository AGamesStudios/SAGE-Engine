#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "../Memory/Ref.h"

namespace SAGE {

    class Font;

    class FontManager {
    public:
        static Ref<Font> Load(const std::string& name, const std::string& path, float pixelHeight = 32.0f);
        static Ref<Font> LoadFromMemory(const std::string& name, const unsigned char* data, std::size_t size, float pixelHeight = 32.0f);
        static Ref<Font> Get(const std::string& name);
        static Ref<Font> GetDefault(float pixelHeight = 32.0f);

        static bool Exists(const std::string& name);
        static void Unload(const std::string& name);

        // Custom font registry helpers
        static std::optional<std::string> RegisterFont(const std::string& name, const std::filesystem::path& path);
        static std::optional<std::string> RegisterFontFile(const std::filesystem::path& path);
        static std::vector<std::string> RegisterFontsInDirectory(const std::filesystem::path& directory, bool recursive = false);
        static std::size_t RegisterFontSearchPaths(const std::vector<std::filesystem::path>& paths, bool recursive = false);
        static bool IsRegistered(const std::string& name);
        static std::optional<std::filesystem::path> GetRegisteredPath(const std::string& name);
        static Ref<Font> LoadRegistered(const std::string& name, float pixelHeight = 32.0f);
    static std::vector<std::string> GetRegisteredFonts(bool includeDisplayNames = false);

        static bool SetDefaultFontOverride(const std::filesystem::path& path);
        static bool SetDefaultFontOverrideByName(const std::string& name);

        static void Clear();
        
        // Управление ресурсами
        static size_t GetLoadedCount();
        static void UnloadUnused();
        static void LogStatus();
    };

} // namespace SAGE
