#include "FontManager.h"

#include "EmbeddedFonts.h"
#include "../Core/Logger.h"
#include "../Graphics/Font.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace SAGE {

    namespace {
        struct RegisteredFont {
            std::filesystem::path path;
            std::string displayName;
        };

        std::unordered_map<std::string, Ref<Font>> s_Fonts;
        std::unordered_map<std::string, RegisteredFont> s_RegisteredFonts;
        std::vector<unsigned char> s_DefaultFontData;
        bool s_DefaultFontDecoded = false;
        std::optional<std::filesystem::path> s_SystemFontPath;
        bool s_SystemFontChecked = false;
        std::optional<std::filesystem::path> s_DefaultOverridePath;

        std::string MakeDefaultKey(const std::string& prefix, float pixelHeight) {
            int keyValue = static_cast<int>(pixelHeight * 100.0f + 0.5f);
            return prefix + std::to_string(keyValue);
        }

        std::string NormalizeKey(const std::string& name) {
            std::string result;
            result.reserve(name.size());
            for (char ch : name) {
                unsigned char uch = static_cast<unsigned char>(ch);
                if (std::isalnum(uch)) {
                    result.push_back(static_cast<char>(std::tolower(uch)));
                }
                else if (ch == '_' || ch == '-' || ch == ' ' || ch == '.') {
                    if (!result.empty() && result.back() != '_') {
                        result.push_back('_');
                    }
                }
            }
            if (result.empty()) {
                result = "font";
            }
            return result;
        }

        const RegisteredFont* FindRegistered(const std::string& name) {
            std::string key = NormalizeKey(name);
            auto it = s_RegisteredFonts.find(key);
            if (it != s_RegisteredFonts.end()) {
                return &it->second;
            }
            return nullptr;
        }

        std::optional<std::string> GetEnvironmentUtf8(const char* name) {
#ifdef _WIN32
            char* buffer = nullptr;
            std::size_t length = 0;
            if (_dupenv_s(&buffer, &length, name) != 0 || !buffer) {
                if (buffer) {
                    free(buffer);
                }
                return std::nullopt;
            }
            std::string value(buffer, length > 0 ? length - 1 : 0);
            free(buffer);
            return value;
#else
            if (const char* value = std::getenv(name)) {
                return std::string(value);
            }
            return std::nullopt;
#endif
        }

        std::optional<std::filesystem::path> LocateSystemFont() {
            namespace fs = std::filesystem;

            std::vector<fs::path> candidates;
            candidates.reserve(16);

            if (auto envFont = GetEnvironmentUtf8("SAGE_DEFAULT_FONT")) {
                candidates.emplace_back(fs::path(*envFont));
            }

            auto pushCandidate = [&](const fs::path& path) {
                if (!path.empty()) {
                    candidates.push_back(path);
                }
            };

            fs::path cwd = fs::current_path();
            pushCandidate(cwd / "Demo" / "assets" / "fonts" / "Default.ttf");
            pushCandidate(cwd / "assets" / "fonts" / "Default.ttf");

#ifdef _WIN32
            auto windir = GetEnvironmentUtf8("WINDIR");
            fs::path fontsDir = windir ? fs::path(*windir) / "Fonts" : fs::path("C:/Windows/Fonts");
            pushCandidate(fontsDir / "segoeui.ttf");
            pushCandidate(fontsDir / "arial.ttf");
            pushCandidate(fontsDir / "calibri.ttf");
            pushCandidate(fontsDir / "tahoma.ttf");
            pushCandidate(fontsDir / "verdana.ttf");
#elif defined(__APPLE__)
            pushCandidate("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
            pushCandidate("/System/Library/Fonts/Supplemental/Arial.ttf");
            pushCandidate("/System/Library/Fonts/Supplemental/GillSans.ttf");
#else
            pushCandidate("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");
            pushCandidate("/usr/share/fonts/truetype/noto/NotoSansUI-Regular.ttf");
            pushCandidate("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
            pushCandidate("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif

            for (const auto& candidate : candidates) {
                if (!candidate.empty() && fs::exists(candidate)) {
                    if (candidate.extension() == ".ttc") {
                        continue; // stb_truetype не поддерживает TrueType Collection напрямую
                    }
                    try {
                        return fs::weakly_canonical(candidate);
                    }
                    catch (const std::exception&) {
                        return candidate;
                    }
                }
            }

            return std::nullopt;
        }

        bool IsSupportedFontExtension(const std::filesystem::path& file) {
            if (!file.has_extension()) {
                return false;
            }

            std::string ext = file.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return ext == ".ttf" || ext == ".otf";
        }
    } // namespace

    Ref<Font> FontManager::Load(const std::string& name, const std::string& path, float pixelHeight) {
        if (auto it = s_Fonts.find(name); it != s_Fonts.end()) {
            return it->second;
        }

        Ref<Font> font = CreateRef<Font>(path, pixelHeight);
        if (!font || !font->IsLoaded()) {
            SAGE_ERROR("Не удалось загрузить шрифт '{}'.", path);
            return font;
        }

        s_Fonts[name] = font;
        return font;
    }

    Ref<Font> FontManager::LoadFromMemory(const std::string& name, const unsigned char* data, std::size_t size, float pixelHeight) {
        if (auto it = s_Fonts.find(name); it != s_Fonts.end()) {
            return it->second;
        }

        Ref<Font> font = CreateRef<Font>(data, size, pixelHeight);
        if (!font || !font->IsLoaded()) {
            SAGE_ERROR("Не удалось загрузить шрифт из памяти '{}'.", name);
            return font;
        }

        s_Fonts[name] = font;
        return font;
    }

    Ref<Font> FontManager::Get(const std::string& name) {
        if (auto it = s_Fonts.find(name); it != s_Fonts.end()) {
            return it->second;
        }
        return nullptr;
    }

    Ref<Font> FontManager::GetDefault(float pixelHeight) {
        if (s_DefaultOverridePath) {
            const std::string overrideKey = MakeDefaultKey("__override_default_", pixelHeight);
            if (auto it = s_Fonts.find(overrideKey); it != s_Fonts.end()) {
                return it->second;
            }

            Ref<Font> overrideFont = CreateRef<Font>(s_DefaultOverridePath->string(), pixelHeight);
            if (overrideFont && overrideFont->IsLoaded()) {
                s_Fonts[overrideKey] = overrideFont;
                return overrideFont;
            }

            SAGE_WARNING("Не удалось загрузить заданный шрифт по умолчанию '{}'.", s_DefaultOverridePath->string());
            s_DefaultOverridePath.reset();
        }

        if (!s_SystemFontChecked) {
            s_SystemFontPath = LocateSystemFont();
            s_SystemFontChecked = true;
        }

        if (s_SystemFontPath) {
            const std::string systemKey = MakeDefaultKey("__system_default_", pixelHeight);
            if (auto it = s_Fonts.find(systemKey); it != s_Fonts.end()) {
                return it->second;
            }

            Ref<Font> systemFont = CreateRef<Font>(s_SystemFontPath->string(), pixelHeight);
            if (systemFont && systemFont->IsLoaded()) {
                s_Fonts[systemKey] = systemFont;
                return systemFont;
            }

            SAGE_WARNING("Не удалось загрузить системный шрифт '{}'. Будет использован встроенный ProggyClean.", s_SystemFontPath->string());
            s_SystemFontPath.reset();
        }

        const std::string embeddedKey = MakeDefaultKey("__embedded_default_", pixelHeight);
        if (auto it = s_Fonts.find(embeddedKey); it != s_Fonts.end()) {
            return it->second;
        }

        if (!s_DefaultFontDecoded) {
            s_DefaultFontData = EmbeddedFonts::GetProggyCleanTTF();
            s_DefaultFontDecoded = true;
        }

        if (s_DefaultFontData.empty()) {
            SAGE_ERROR("Не удалось декодировать встроенный шрифт");
            return nullptr;
        }

        Ref<Font> font = CreateRef<Font>(s_DefaultFontData.data(), s_DefaultFontData.size(), pixelHeight);
        if (!font || !font->IsLoaded()) {
            SAGE_ERROR("Не удалось инициализировать встроенный шрифт");
            return font;
        }

        s_Fonts[embeddedKey] = font;
        return font;
    }

    bool FontManager::Exists(const std::string& name) {
        return s_Fonts.find(name) != s_Fonts.end();
    }

    std::optional<std::string> FontManager::RegisterFont(const std::string& name, const std::filesystem::path& path) {
        namespace fs = std::filesystem;

        if (name.empty()) {
            SAGE_WARNING("RegisterFont: пустое имя");
            return std::nullopt;
        }

        if (path.empty() || !fs::exists(path)) {
            SAGE_WARNING("RegisterFont: путь '{}' не существует", path.string());
            return std::nullopt;
        }

        if (!IsSupportedFontExtension(path)) {
            SAGE_WARNING("RegisterFont: '{}': расширение '{}' не поддерживается", path.string(), path.extension().string());
            return std::nullopt;
        }

        fs::path canonical = path;
        try {
            canonical = fs::weakly_canonical(path);
        }
        catch (const std::exception&) {
            canonical = path;
        }

        for (const auto& [existingKey, record] : s_RegisteredFonts) {
            if (record.path == canonical) {
                return existingKey;
            }
        }

        std::string keyBase = NormalizeKey(name);
        std::string key = keyBase;
        int suffix = 1;
        while (s_RegisteredFonts.find(key) != s_RegisteredFonts.end()) {
            key = keyBase + "_" + std::to_string(++suffix);
        }

        s_RegisteredFonts[key] = RegisteredFont{ canonical, name };
        SAGE_INFO("Зарегистрирован шрифт '{}' -> '{}'", key, canonical.string());
        return key;
    }

    std::optional<std::string> FontManager::RegisterFontFile(const std::filesystem::path& path) {
        namespace fs = std::filesystem;

        if (path.empty()) {
            SAGE_WARNING("RegisterFontFile: пустой путь");
            return std::nullopt;
        }

        if (!fs::exists(path)) {
            SAGE_WARNING("RegisterFontFile: путь '{}' не найден", path.string());
            return std::nullopt;
        }

        if (!IsSupportedFontExtension(path)) {
            SAGE_WARNING("RegisterFontFile: '{}' имеет неподдерживаемое расширение", path.string());
            return std::nullopt;
        }

        const std::string displayName = path.stem().string();
        return RegisterFont(displayName, path);
    }

    std::vector<std::string> FontManager::RegisterFontsInDirectory(const std::filesystem::path& directory, bool recursive) {
        namespace fs = std::filesystem;
        std::vector<std::string> registered;

        if (directory.empty() || !fs::exists(directory)) {
            return registered;
        }

        auto processEntry = [&](const fs::path& file) {
            if (!IsSupportedFontExtension(file)) {
                return;
            }
            const std::string displayName = file.stem().string();
            if (auto key = RegisterFont(displayName, file)) {
                registered.push_back(*key);
            }
        };

        try {
            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                    if (entry.is_regular_file()) {
                        processEntry(entry.path());
                    }
                }
            }
            else {
                for (const auto& entry : fs::directory_iterator(directory)) {
                    if (entry.is_regular_file()) {
                        processEntry(entry.path());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            SAGE_WARNING("RegisterFontsInDirectory: ошибка чтения '{}': {}", directory.string(), e.what());
        }

        if (!registered.empty()) {
            SAGE_INFO("Найдено пользовательских шрифтов: {} (каталог '{}')", registered.size(), directory.string());
        }
        else {
            SAGE_TRACE("В каталоге '{}' не найдено шрифтов TTF/OTF", directory.string());
        }

        return registered;
    }

    std::size_t FontManager::RegisterFontSearchPaths(const std::vector<std::filesystem::path>& paths, bool recursive) {
        namespace fs = std::filesystem;
        std::size_t totalRegistered = 0;

        for (const auto& entry : paths) {
            if (entry.empty()) {
                continue;
            }

            std::error_code ec;
            const bool exists = fs::exists(entry, ec);
            if (!exists || ec) {
                SAGE_TRACE("RegisterFontSearchPaths: '{}' пропущен (отсутствует)", entry.string());
                continue;
            }

            if (fs::is_directory(entry, ec) && !ec) {
                const auto registered = RegisterFontsInDirectory(entry, recursive);
                totalRegistered += registered.size();
            }
            else if (fs::is_regular_file(entry, ec) && !ec) {
                if (auto key = RegisterFontFile(entry)) {
                    ++totalRegistered;
                }
            }
        }

        return totalRegistered;
    }

    bool FontManager::IsRegistered(const std::string& name) {
        return FindRegistered(name) != nullptr;
    }

    std::optional<std::filesystem::path> FontManager::GetRegisteredPath(const std::string& name) {
        if (const RegisteredFont* record = FindRegistered(name)) {
            return record->path;
        }
        return std::nullopt;
    }

    std::vector<std::string> FontManager::GetRegisteredFonts(bool includeDisplayNames) {
        std::vector<std::string> names;
        names.reserve(s_RegisteredFonts.size());
        for (const auto& [key, record] : s_RegisteredFonts) {
            names.push_back(includeDisplayNames ? record.displayName : key);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    Ref<Font> FontManager::LoadRegistered(const std::string& name, float pixelHeight) {
        const RegisteredFont* record = FindRegistered(name);
        if (!record) {
            SAGE_WARNING("LoadRegistered: шрифт '{}' не найден в реестре", name);
            return nullptr;
        }

        std::string key = MakeDefaultKey(NormalizeKey(name) + "_", pixelHeight);
        if (auto it = s_Fonts.find(key); it != s_Fonts.end()) {
            return it->second;
        }

        Ref<Font> font = CreateRef<Font>(record->path.string(), pixelHeight);
        if (!font || !font->IsLoaded()) {
            SAGE_ERROR("Не удалось загрузить зарегистрированный шрифт '{}' из '{}'", record->displayName, record->path.string());
            Ref<Font> fallback = GetDefault(pixelHeight);
            if (fallback && fallback->IsLoaded()) {
                s_Fonts[key] = fallback;
            }
            return fallback;
        }

        s_Fonts[key] = font;
        return font;
    }

    bool FontManager::SetDefaultFontOverride(const std::filesystem::path& path) {
        namespace fs = std::filesystem;

        if (path.empty() || !fs::exists(path)) {
            SAGE_WARNING("SetDefaultFontOverride: путь '{}' не существует", path.string());
            return false;
        }

        try {
            s_DefaultOverridePath = fs::weakly_canonical(path);
        }
        catch (const std::exception&) {
            s_DefaultOverridePath = path;
        }

        for (auto it = s_Fonts.begin(); it != s_Fonts.end();) {
            if (it->first.find("__override_default_") == 0) {
                it = s_Fonts.erase(it);
            }
            else {
                ++it;
            }
        }

        s_SystemFontPath.reset();
        s_SystemFontChecked = false;

        SAGE_INFO("Шрифт по умолчанию переопределён: '{}'", s_DefaultOverridePath->string());
        return true;
    }

    bool FontManager::SetDefaultFontOverrideByName(const std::string& name) {
        if (auto path = GetRegisteredPath(name)) {
            return SetDefaultFontOverride(*path);
        }
        SAGE_WARNING("SetDefaultFontOverrideByName: шрифт '{}' не зарегистрирован", name);
        return false;
    }

    void FontManager::Unload(const std::string& name) {
        s_Fonts.erase(name);
    }

    void FontManager::Clear() {
        SAGE_INFO("[FontManager] Очистка всех шрифтов ({} загружено)...", s_Fonts.size());
        s_Fonts.clear();
        s_DefaultFontData.clear();
        s_DefaultFontDecoded = false;
        s_SystemFontPath.reset();
        s_SystemFontChecked = false;
        s_DefaultOverridePath.reset();
        s_RegisteredFonts.clear();
    }

    size_t FontManager::GetLoadedCount() {
        return s_Fonts.size();
    }

    void FontManager::UnloadUnused() {
        size_t unloadedCount = 0;
        for (auto it = s_Fonts.begin(); it != s_Fonts.end();) {
            if (it->second.use_count() == 1) {
                SAGE_INFO("[FontManager] Выгрузка неиспользуемого шрифта '{}'...", it->first);
                it = s_Fonts.erase(it);
                ++unloadedCount;
            } else {
                ++it;
            }
        }
        if (unloadedCount > 0) {
            SAGE_INFO("[FontManager] Выгружено {} неиспользуемых шрифтов.", unloadedCount);
        }
    }

    void FontManager::LogStatus() {
        SAGE_INFO("[FontManager] Загружено шрифтов: {}", s_Fonts.size());
        for (const auto& [name, font] : s_Fonts) {
            SAGE_INFO("  - '{}': ref_count={}", name, font.use_count());
        }
    }

} // namespace SAGE
