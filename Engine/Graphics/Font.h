#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "../Graphics/MathTypes.h"
#include "../Memory/Ref.h"

namespace SAGE {

    class Texture;

    struct Glyph {
        Vector2 uvMin{0.0f, 0.0f};
        Vector2 uvMax{1.0f, 1.0f};
        Vector2 size{0.0f, 0.0f};
        Vector2 bearing{0.0f, 0.0f};
        float advance = 0.0f;
    };

    class Font {
    public:
        Font(const std::string& path, float pixelHeight = 32.0f);
        Font(const unsigned char* data, std::size_t size, float pixelHeight = 32.0f);

        bool IsLoaded() const { return m_Loaded; }
        float GetLineHeight() const { return m_LineHeight; }
        float GetAscent() const { return m_Ascent; }
        float GetDescent() const { return m_Descent; }
        float GetPixelHeight() const { return m_PixelHeight; }

        const Glyph& GetGlyph(uint32_t codepoint) const;
        Ref<Texture> GetAtlasTexture() const { return m_AtlasTexture; }

    private:
        bool LoadFromFile(const std::string& path, float pixelHeight);
        bool LoadFromBuffer(const unsigned char* data, std::size_t size, float pixelHeight);

        std::unordered_map<uint32_t, Glyph> m_Glyphs;
        Glyph m_FallbackGlyph{};
        Ref<Texture> m_AtlasTexture;

        bool m_Loaded = false;
        float m_LineHeight = 0.0f;
        float m_Ascent = 0.0f;
        float m_Descent = 0.0f;
        float m_PixelHeight = 0.0f;
    };

} // namespace SAGE
