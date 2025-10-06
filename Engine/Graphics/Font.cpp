#include "Font.h"

#include "Texture.h"
#include "../Core/Logger.h"

#include <fstream>
#include <vector>
#include <array>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace SAGE {

    Font::Font(const std::string& path, float pixelHeight) {
        m_Loaded = LoadFromFile(path, pixelHeight);
        if (!m_Loaded) {
            SAGE_ERROR("Не удалось загрузить шрифт: {}", path);
        }
    }

    Font::Font(const unsigned char* data, std::size_t size, float pixelHeight) {
        m_Loaded = LoadFromBuffer(data, size, pixelHeight);
        if (!m_Loaded) {
            SAGE_ERROR("Не удалось загрузить встроенный шрифт ({} байт)", size);
        }
    }

    const Glyph& Font::GetGlyph(uint32_t codepoint) const {
        if (auto it = m_Glyphs.find(codepoint); it != m_Glyphs.end()) {
            return it->second;
        }
        return m_FallbackGlyph;
    }

    bool Font::LoadFromFile(const std::string& path, float pixelHeight) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            SAGE_ERROR("Не удалось открыть файл шрифта: {}", path);
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> fontBuffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), size)) {
            SAGE_ERROR("Не удалось прочитать данные шрифта: {}", path);
            return false;
        }

        return LoadFromBuffer(fontBuffer.data(), fontBuffer.size(), pixelHeight);
    }

    bool Font::LoadFromBuffer(const unsigned char* data, std::size_t size, float pixelHeight) {
        if (!data || size == 0) {
            SAGE_ERROR("Пустые данные шрифта");
            return false;
        }

        std::vector<unsigned char> fontBuffer(data, data + size);

        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
            SAGE_ERROR("stbtt_InitFont не смог обработать данные шрифта");
            return false;
        }

        float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);
        int ascent = 0, descent = 0, lineGap = 0;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        m_LineHeight = (static_cast<float>(ascent - descent + lineGap)) * scale;
        m_Ascent = static_cast<float>(ascent) * scale;
        m_Descent = static_cast<float>(descent) * scale;
        m_PixelHeight = pixelHeight;

        const int atlasWidth = 1024;
        const int atlasHeight = 1024;
        std::vector<unsigned char> bitmap(static_cast<size_t>(atlasWidth) * atlasHeight, 0);

        stbtt_pack_context packContext;
        if (!stbtt_PackBegin(&packContext, bitmap.data(), atlasWidth, atlasHeight, 0, 1, nullptr)) {
            SAGE_ERROR("stbtt_PackBegin не смог инициализировать атлас");
            return false;
        }
        stbtt_PackSetOversampling(&packContext, 2, 2);

        struct Range {
            int first;
            int count;
        };

        const std::array<Range, 4> glyphRanges = {
            Range{32, 95},                // Basic Latin
            Range{0x00A0, 96},            // Latin-1 Supplement subset
            Range{0x0400, 96},            // Cyrillic
            Range{0x2010, 48}             // Common punctuation (en dash, quotes, etc.)
        };

        std::size_t totalGlyphs = 0;
        for (const auto& range : glyphRanges) {
            totalGlyphs += static_cast<std::size_t>(range.count);
        }

        std::vector<stbtt_packedchar> packedChars(totalGlyphs);
        std::vector<stbtt_pack_range> packRanges(glyphRanges.size());

        std::size_t glyphOffset = 0;
        for (std::size_t i = 0; i < glyphRanges.size(); ++i) {
            packRanges[i].font_size = pixelHeight;
            packRanges[i].first_unicode_codepoint_in_range = glyphRanges[i].first;
            packRanges[i].num_chars = glyphRanges[i].count;
            packRanges[i].array_of_unicode_codepoints = nullptr;
            packRanges[i].chardata_for_range = packedChars.data() + glyphOffset;
            packRanges[i].h_oversample = 2;
            packRanges[i].v_oversample = 2;
            glyphOffset += static_cast<std::size_t>(glyphRanges[i].count);
        }

        if (!stbtt_PackFontRanges(&packContext, fontBuffer.data(), 0, packRanges.data(), static_cast<int>(packRanges.size()))) {
            stbtt_PackEnd(&packContext);
            SAGE_ERROR("stbtt_PackFontRanges вернул ошибку для данных шрифта");
            return false;
        }

        stbtt_PackEnd(&packContext);

        m_AtlasTexture = CreateRef<Texture>(static_cast<unsigned int>(atlasWidth), static_cast<unsigned int>(atlasHeight), Texture::Format::Red, bitmap.data());
        if (!m_AtlasTexture || !m_AtlasTexture->IsLoaded()) {
            SAGE_ERROR("Не удалось создать текстуру атласа шрифта");
            return false;
        }

        m_Glyphs.clear();
        m_Glyphs.reserve(totalGlyphs + 1);

        glyphOffset = 0;
        for (const auto& range : glyphRanges) {
            for (int i = 0; i < range.count; ++i) {
                const stbtt_packedchar& pc = packedChars[glyphOffset + static_cast<std::size_t>(i)];
                const int x0 = pc.x0;
                const int y0 = pc.y0;
                const int x1 = pc.x1;
                const int y1 = pc.y1;

                Glyph glyph;
                glyph.uvMin = Vector2(static_cast<float>(x0) / atlasWidth, static_cast<float>(y0) / atlasHeight);
                glyph.uvMax = Vector2(static_cast<float>(x1) / atlasWidth, static_cast<float>(y1) / atlasHeight);
                glyph.size = Vector2(static_cast<float>(x1 - x0), static_cast<float>(y1 - y0));
                glyph.bearing = Vector2(pc.xoff, pc.yoff);
                glyph.advance = pc.xadvance;

                const uint32_t codepoint = static_cast<uint32_t>(range.first + i);
                m_Glyphs.emplace(codepoint, glyph);
            }
            glyphOffset += static_cast<std::size_t>(range.count);
        }

        if (auto it = m_Glyphs.find(static_cast<uint32_t>('?')); it != m_Glyphs.end()) {
            m_FallbackGlyph = it->second;
        }
        else if (!m_Glyphs.empty()) {
            m_FallbackGlyph = m_Glyphs.begin()->second;
        }
        else {
            m_FallbackGlyph = Glyph{};
        }

        return true;
    }

} // namespace SAGE
