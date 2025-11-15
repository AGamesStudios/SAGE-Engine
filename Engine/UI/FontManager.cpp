#include "FontManager.h"
#include "Core/Logger.h"

#include <fstream>

namespace SAGE {
namespace UI {

// Static members
std::unordered_map<std::string, Ref<Font>> FontManager::s_Fonts;
Ref<Font> FontManager::s_DefaultFont;
bool FontManager::s_DefaultFontCreated = false;

// Minimal embedded font data (will be empty for now, but structure is ready)
// In production, you would embed a small TTF file here or load from resources
static const unsigned char DEFAULT_FONT_DATA[] = { 0 };

void FontManager::CreateDefaultFont() {
    if (s_DefaultFontCreated) return;

    // Try to load a system font or embedded font
    // For now, we'll try to find Arial or similar
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",  // Linux
        "/System/Library/Fonts/Helvetica.ttc",              // macOS
    };

    for (const char* path : fontPaths) {
        std::ifstream file(path);
        if (file.good()) {
            file.close();
            auto font = CreateRef<Font>(path, 24.0f);
            if (font && font->IsLoaded()) {
                s_DefaultFont = font;
                s_DefaultFontCreated = true;
                SAGE_INFO("Default font loaded from: {}", path);
                return;
            }
        }
    }

    // Fallback: Create empty font (will not render text but won't crash)
    SAGE_WARNING("Failed to find system font. Text rendering will be disabled.");
    s_DefaultFont = nullptr;
    s_DefaultFontCreated = true;
}

Ref<Font> FontManager::GetDefaultFont() {
    if (!s_DefaultFontCreated) {
        CreateDefaultFont();
    }
    return s_DefaultFont;
}

Ref<Font> FontManager::LoadFont(const std::string& name, const std::string& path, float pixelHeight) {
    // Check if already loaded
    auto it = s_Fonts.find(name);
    if (it != s_Fonts.end()) {
        return it->second;
    }

    // Load new font
    auto font = CreateRef<Font>(path, pixelHeight);
    if (font && font->IsLoaded()) {
        s_Fonts[name] = font;
        SAGE_INFO("Font '{}' loaded from: {}", name, path);
        return font;
    }

    SAGE_ERROR("Failed to load font '{}' from: {}", name, path);
    return nullptr;
}

Ref<Font> FontManager::GetFont(const std::string& name) {
    auto it = s_Fonts.find(name);
    if (it != s_Fonts.end()) {
        return it->second;
    }
    return nullptr;
}

bool FontManager::HasFont(const std::string& name) {
    return s_Fonts.find(name) != s_Fonts.end();
}

void FontManager::Clear() {
    s_Fonts.clear();
    s_DefaultFont = nullptr;
    s_DefaultFontCreated = false;
}

} // namespace UI
} // namespace SAGE
