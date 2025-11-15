#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Resources/Font.h"

#include <string>
#include <unordered_map>

namespace SAGE {
namespace UI {

/// @brief Font manager for UI system
class FontManager {
public:
    /// @brief Get default font
    static Ref<Font> GetDefaultFont();

    /// @brief Load font from file
    static Ref<Font> LoadFont(const std::string& name, const std::string& path, float pixelHeight = 32.0f);

    /// @brief Get loaded font by name
    static Ref<Font> GetFont(const std::string& name);

    /// @brief Check if font exists
    static bool HasFont(const std::string& name);

    /// @brief Clear all fonts
    static void Clear();

private:
    static std::unordered_map<std::string, Ref<Font>> s_Fonts;
    static Ref<Font> s_DefaultFont;
    static bool s_DefaultFontCreated;

    static void CreateDefaultFont();
};

} // namespace UI
} // namespace SAGE
