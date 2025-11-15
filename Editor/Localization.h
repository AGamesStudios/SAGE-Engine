#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace SAGE {
namespace Editor {

enum class Language {
    English = 0,
    Russian = 1
};

enum class TextID {
    Menu_File,
    Menu_NewScene,
    Menu_OpenScene,
    Menu_SaveScene,
    Menu_SaveSceneAs,
    Menu_Exit,
    Menu_View,
    Menu_Viewport,
    Menu_Hierarchy,
    Menu_Inspector,
    Menu_Help,
    Menu_About,
    Menu_RecentProjects,
    Menu_ClearRecent,
    Menu_HelpShortcuts,
    Menu_HelpDocs,
    Menu_Language,
    Language_English,
    Language_Russian,
    Viewport_WindowTitle,
    SceneStatus_NewSceneCreated,
    SceneStatus_SpecifyScenePath,
    SceneStatus_LoadFailed,
    SceneStatus_Loaded,
    SceneStatus_NoActiveScene,
    SceneStatus_SpecifySavePath,
    SceneStatus_SaveFailed,
    SceneStatus_Saved,
    SceneLabel_Format,
    SceneLabel_NewPlaceholder,
    Dialog_OpenScene_Title,
    Dialog_SaveScene_Title,
    Dialog_OpenScene_Prompt,
    Dialog_SaveScene_Prompt,
    Dialog_Open_Button,
    Dialog_Save_Button,
    Dialog_Cancel_Button,
    Dialog_PathEmptyError,
    Inspector_WindowTitle,
    Inspector_NoEntitySelected,
    Inspector_SelectedEntityMissing,
    Inspector_EntityLabel,
    Inspector_IDLabel,
    Inspector_TransformHeader,
    Inspector_SpriteHeader,
    Inspector_Position,
    Inspector_Rotation,
    Inspector_Scale,
    Inspector_NoTransform,
    Inspector_AddTransform,
    Inspector_Visible,
    Inspector_FlipX,
    Inspector_FlipY,
    Inspector_Size,
    Inspector_Tint,
    Inspector_TextureLabel,
    Inspector_TextureNone,
    Inspector_LoadTexture,
    Inspector_ClearTexture,
    Inspector_NoSprite,
    Inspector_AddSprite,
    Inspector_AddComponent,
    Inspector_TextureDialog_Title,
    Inspector_TextureDialog_Prompt,
    Inspector_TextureDialog_Submit,
    Inspector_TextureDialog_Cancel,
    Inspector_TextureDialog_LoadFailed,
    Hierarchy_WindowTitle,
    Hierarchy_NoScene,
    Hierarchy_CreateEntity,
    Hierarchy_DefaultEntityName,
    Hierarchy_DefaultSpriteName,
    Hierarchy_NoEntities,
    Hierarchy_ContextRename,
    Hierarchy_ContextDelete,
    Count
};

class Localization {
public:
    static Localization& Instance();

    void SetLanguage(Language language);
    Language GetLanguage() const;

    const std::string& Get(TextID id) const;

    template <typename... Args>
    std::string Format(TextID id, Args&&... args) const {
        const auto index = static_cast<std::size_t>(id);
        const auto& entry = m_Entries[index];
        const std::string& fmt = (m_CurrentLanguage == Language::Russian)
            ? entry.russian
            : entry.english;

        std::array<std::string_view, sizeof...(Args)> parameters{
            std::string_view(std::forward<Args>(args))...
        };

        std::string result;
        result.reserve(fmt.size() + parameters.size() * 16);

        std::size_t last = 0;
        std::size_t argIndex = 0;
        while (true) {
            std::size_t placeholder = fmt.find("{}", last);
            if (placeholder == std::string::npos) {
                result.append(fmt, last, std::string::npos);
                break;
            }

            result.append(fmt, last, placeholder - last);
            if (argIndex < parameters.size()) {
                const std::string_view value = parameters[argIndex++];
                result.append(value.data(), value.size());
            } else {
                result.append("{}");
            }
            last = placeholder + 2;
        }

        return result;
    }

    static std::string_view GetLanguageCode(Language language);
    static Language FromLanguageCode(std::string_view code);

private:
    Localization();

    struct LocalizedEntry {
        std::string english;
        std::string russian;
    };

    std::array<LocalizedEntry, static_cast<std::size_t>(TextID::Count)> m_Entries{};
    Language m_CurrentLanguage = Language::English;
};

inline Localization& Localization::Instance() {
    static Localization instance;
    return instance;
}

inline void Localization::SetLanguage(Language language) {
    m_CurrentLanguage = language;
}

inline const std::string& Localization::Get(TextID id) const {
    const auto index = static_cast<std::size_t>(id);
    if (index >= m_Entries.size()) {
        static const std::string kMissing = "<missing>";
        return kMissing;
    }
    const auto& entry = m_Entries[index];
    if (m_CurrentLanguage == Language::Russian) {
        if (!entry.russian.empty()) {
            return entry.russian;
        }
        // Fallback: English if Russian missing
        if (!entry.english.empty()) {
            return entry.english;
        }
        static const std::string kNoEntry = "<no-entry>";
        return kNoEntry;
    }
    // English branch
    if (!entry.english.empty()) {
        return entry.english;
    }
    if (!entry.russian.empty()) {
        return entry.russian; // fallback the other way if English missing
    }
    static const std::string kNoEntry = "<no-entry>";
    return kNoEntry;
}

inline Language Localization::GetLanguage() const {
    return m_CurrentLanguage;
}

} // namespace Editor
} // namespace SAGE
