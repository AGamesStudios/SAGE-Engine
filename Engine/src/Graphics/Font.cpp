#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Texture.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Log.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <fstream>
#include <vector>
#include <iostream>

namespace SAGE {

namespace {
    std::shared_ptr<Font> s_DefaultFont = nullptr;
}

bool Font::Load(const std::string& filepath, int fontSize) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        SAGE_ERROR("Failed to open font file: {}", filepath);
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        SAGE_ERROR("Failed to read font file: {}", filepath);
        return false;
    }

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, buffer.data(), 0)) {
        SAGE_ERROR("Failed to initialize font info");
        return false;
    }

    m_FontSize = fontSize;
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(fontSize));

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    m_LineHeight = (ascent - descent + lineGap) * scale;

    // Create atlas
    const int atlasWidth = 1024;
    const int atlasHeight = 1024;
    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight);

    // Pack characters (ASCII + Cyrillic)
    stbtt_pack_context packContext;
    if (!stbtt_PackBegin(&packContext, atlasData.data(), atlasWidth, atlasHeight, 0, 1, nullptr)) {
        SAGE_ERROR("Failed to begin font packing");
        return false;
    }
    SAGE_INFO("Font packing begun");

    stbtt_PackSetOversampling(&packContext, 1, 1);

    // Define ranges
    struct Range {
        int first;
        int count;
        std::vector<stbtt_packedchar> chars;
    };

    Range ranges[] = {
        { 32, 96, std::vector<stbtt_packedchar>(96) },      // ASCII
        { 0x0400, 256, std::vector<stbtt_packedchar>(256) } // Cyrillic
    };

    SAGE_INFO("Packing ranges manually");
    for (int i = 0; i < 2; ++i) {
        if (!stbtt_PackFontRange(&packContext, buffer.data(), 0, static_cast<float>(fontSize), 
                                 ranges[i].first, ranges[i].count, ranges[i].chars.data())) {
            SAGE_WARN("Failed to pack range {} (count {}) for font: {}", i, ranges[i].count, filepath);
        }
    }
    SAGE_INFO("Packing done");

    stbtt_PackEnd(&packContext);

    // Create texture (R8 format -> RGBA)
    std::vector<unsigned char> rgbaData(atlasWidth * atlasHeight * 4);
    for (int i = 0; i < atlasWidth * atlasHeight; ++i) {
        unsigned char alpha = atlasData[i];
        rgbaData[i * 4 + 0] = 255;
        rgbaData[i * 4 + 1] = 255;
        rgbaData[i * 4 + 2] = 255;
        rgbaData[i * 4 + 3] = alpha;
    }

    m_Texture = std::make_shared<Texture>(atlasWidth, atlasHeight, rgbaData.data());

    // Store glyph info
    for (int r = 0; r < 2; ++r) {
        for (int i = 0; i < ranges[r].count; ++i) {
            uint32_t c = ranges[r].first + i;
            const auto& pc = ranges[r].chars[i];
            
            Glyph glyph;
            glyph.position = Vector2(static_cast<float>(pc.x0), static_cast<float>(pc.y0));
            glyph.size = Vector2(static_cast<float>(pc.x1 - pc.x0), static_cast<float>(pc.y1 - pc.y0));
            glyph.bearing = Vector2(static_cast<float>(pc.xoff), static_cast<float>(pc.yoff));
            glyph.advance = pc.xadvance;
            
            m_Glyphs[c] = glyph;
        }
    }
    
    SAGE_INFO("Font loaded: {} (size: {})", filepath, fontSize);
    return true;
}

const Glyph* Font::GetGlyph(uint32_t c) const {
    auto it = m_Glyphs.find(c);
    return (it != m_Glyphs.end()) ? &it->second : nullptr;
}

// Helper for UTF-8 decoding
static uint32_t DecodeUTF8(const char*& str) {
    uint32_t c = (unsigned char)*str;
    if (c < 0x80) {
        str++;
        return c;
    }
    if ((c & 0xE0) == 0xC0) {
        uint32_t c2 = (unsigned char)*++str;
        str++;
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    }
    if ((c & 0xF0) == 0xE0) {
        uint32_t c2 = (unsigned char)*++str;
        uint32_t c3 = (unsigned char)*++str;
        str++;
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }
    if ((c & 0xF8) == 0xF0) {
        uint32_t c2 = (unsigned char)*++str;
        uint32_t c3 = (unsigned char)*++str;
        uint32_t c4 = (unsigned char)*++str;
        str++;
        return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    }
    str++;
    return 0; // Invalid
}

Vector2 Font::MeasureText(const std::string& text) const {
    float width = 0.0f;
    float height = static_cast<float>(m_FontSize);
    
    const char* ptr = text.c_str();
    while (*ptr) {
        uint32_t c = DecodeUTF8(ptr);
        const Glyph* glyph = GetGlyph(c);
        if (glyph) {
            width += glyph->advance;
        }
    }
    
    return Vector2(width, height);
}

std::shared_ptr<Font> Font::CreateDefault() {
    // For now, return nullptr - default font will be created in TextRenderer::Init
    return nullptr;
}

// TextRenderer implementation
void TextRenderer::Init() {
    SAGE_INFO("TextRenderer: Initializing...");
    
    // Try to load a default system font
    s_DefaultFont = std::make_shared<Font>();
    
    // Try common Windows font paths
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "assets/fonts/default.ttf"
    };
    
    bool loaded = false;
    for (const auto& path : fontPaths) {
        if (s_DefaultFont->Load(path, 24)) {
            loaded = true;
            break;
        }
    }
    
    if (!loaded) {
        SAGE_WARN("TextRenderer: No default font found, text rendering will be disabled");
        s_DefaultFont = nullptr;
    }
}

void TextRenderer::Shutdown() {
    s_DefaultFont = nullptr;
    SAGE_INFO("TextRenderer: Shutdown complete");
}

void TextRenderer::DrawText(const std::string& text, 
                           const Vector2& position, 
                           const Color& color,
                           std::shared_ptr<Font> font) {
    DrawTextScaled(text, position, 1.0f, color, font);
}

void TextRenderer::DrawTextScaled(const std::string& text,
                              const Vector2& position,
                              float scale,
                              const Color& color,
                              std::shared_ptr<Font> font) {
    if (!font) {
        font = s_DefaultFont;
    }
    
    if (!font || !font->GetTexture()) {
        return;
    }

    std::shared_ptr<Texture> texture = font->GetTexture();
    float texWidth = static_cast<float>(texture->GetWidth());
    float texHeight = static_cast<float>(texture->GetHeight());

    float x = position.x;
    float y = position.y;

    // Check projection orientation
    // m[4] is the Y scaling factor (1,1 element in 0-indexed 3x3 matrix)
    // If positive, Y is up (Game). If negative, Y is down (UI).
    bool isYUp = Renderer::GetProjectionMatrix().m[4] > 0.0f;
    
    const char* ptr = text.c_str();
    while (*ptr) {
        uint32_t c = DecodeUTF8(ptr);
        const Glyph* glyph = font->GetGlyph(c);
        if (!glyph) continue;
        
        float xpos = x + glyph->bearing.x * scale;
        float ypos = y + glyph->bearing.y * scale;
        
        float w = glyph->size.x * scale;
        float h = glyph->size.y * scale;

        if (isYUp) {
            // Adjust for Y-up coordinate system
            // bearing.y is negative (distance from baseline to top in Y-down)
            // In Y-up, we want to draw from (y - bearing.y - h) to (y - bearing.y)
            ypos = y - glyph->bearing.y * scale - h;
        }

        Sprite sprite(texture);
        sprite.transform.position = Vector2(xpos, ypos);
        sprite.transform.scale = Vector2(w, h);
        
        sprite.transform.origin = Vector2(0.0f, 0.0f); // Top-left origin
        sprite.tint = color;
        
        // Calculate UVs
        sprite.textureRect.x = glyph->position.x / texWidth;
        sprite.textureRect.y = glyph->position.y / texHeight;
        sprite.textureRect.width = glyph->size.x / texWidth;
        sprite.textureRect.height = glyph->size.y / texHeight;
        
        Renderer::DrawSprite(sprite);
        
        x += glyph->advance * scale;
    }
}

void TextRenderer::DrawTextAligned(const std::string& text,
                               const Vector2& position,
                               TextAlign align,
                               const Color& color,
                               std::shared_ptr<Font> font) {
    if (!font) font = s_DefaultFont;
    if (!font) return;

    Vector2 size = font->MeasureText(text);
    Vector2 pos = position;

    if (align == TextAlign::Center) {
        pos.x -= size.x * 0.5f;
    } else if (align == TextAlign::Right) {
        pos.x -= size.x;
    }

    DrawText(text, pos, color, font);
}

void TextRenderer::SetDefaultFont(std::shared_ptr<Font> font) {
    s_DefaultFont = font;
}

std::shared_ptr<Font> TextRenderer::GetDefaultFont() {
    return s_DefaultFont;
}

} // namespace SAGE
