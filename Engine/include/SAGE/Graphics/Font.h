#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace SAGE {

// Forward declarations
class Texture;

/// Character glyph information
struct Glyph {
    Vector2 position;       // Position in texture atlas
    Vector2 size;           // Size of glyph
    Vector2 bearing;        // Offset from baseline
    float advance;          // Horizontal advance to next glyph
};

/// Font class for text rendering using TrueType fonts
class Font {
public:
    Font() = default;
    ~Font() = default;

    /// Load font from file
    bool Load(const std::string& filepath, int fontSize = 32);
    
    /// Get glyph for character
    const Glyph* GetGlyph(uint32_t codepoint) const;
    
    /// Get font texture atlas
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }
    
    /// Get font size
    int GetFontSize() const { return m_FontSize; }
    
    /// Measure text dimensions
    Vector2 MeasureText(const std::string& text) const;

    /// Create default font
    static std::shared_ptr<Font> CreateDefault();

private:
    std::shared_ptr<Texture> m_Texture;
    std::unordered_map<uint32_t, Glyph> m_Glyphs;
    int m_FontSize = 32;
    float m_LineHeight = 0.0f;
};

/// Text alignment options
enum class TextAlign {
    Left,
    Center,
    Right
};

/// Text rendering utility
class TextRenderer {
public:
    static void Init();
    static void Shutdown();

    /// Draw text at position
    static void DrawText(const std::string& text, 
                        const Vector2& position, 
                        const Color& color = Color::White(),
                        std::shared_ptr<Font> font = nullptr);
    
    /// Draw text with alignment
    static void DrawTextAligned(const std::string& text,
                               const Vector2& position,
                               TextAlign align,
                               const Color& color = Color::White(),
                               std::shared_ptr<Font> font = nullptr);
    
    /// Draw text with scale
    static void DrawTextScaled(const std::string& text,
                              const Vector2& position,
                              float scale,
                              const Color& color = Color::White(),
                              std::shared_ptr<Font> font = nullptr);

    /// Set default font
    static void SetDefaultFont(std::shared_ptr<Font> font);
    
    /// Get default font
    static std::shared_ptr<Font> GetDefaultFont();

private:
    TextRenderer() = delete;
};

} // namespace SAGE
