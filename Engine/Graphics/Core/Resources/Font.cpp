#include "Font.h"

#include "Texture.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Core/Logger.h"

#include <array>
#include <fstream>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace SAGE {

    namespace {
        constexpr int kTextureAtlasWidth = 1024;
        constexpr int kTextureAtlasHeight = 1024;
    }

    Font::Font(const std::string& path, float pixelHeight) {
        m_Loaded = LoadFromFile(path, pixelHeight);
        if (!m_Loaded) {
            SAGE_ERROR("Failed to load font from path '{}'", path);
        }
    }

    Font::Font(const unsigned char* data, std::size_t size, float pixelHeight) {
        m_Loaded = LoadFromBuffer(data, size, pixelHeight);
        if (!m_Loaded) {
            SAGE_ERROR("Failed to load font from memory buffer ({} bytes)", size);
        }
    }

    // Деструктор определяем здесь, чтобы компилятор знал полный тип stbtt_fontinfo
    Font::~Font() = default;

    const Glyph& Font::GetGlyph(uint32_t codepoint) const {
        if (auto it = m_Glyphs.find(codepoint); it != m_Glyphs.end()) {
            return it->second;
        }
        return m_FallbackGlyph;
    }

    float Font::GetKerning(uint32_t leftCodepoint, uint32_t rightCodepoint) const {
        if (!m_FontInfo || leftCodepoint == 0 || rightCodepoint == 0) {
            return 0.0f;
        }

        const int advance = stbtt_GetCodepointKernAdvance(
            m_FontInfo.get(),
            static_cast<int>(leftCodepoint),
            static_cast<int>(rightCodepoint));
        return static_cast<float>(advance) * m_Scale;
    }

    bool Font::LoadFromFile(const std::string& path, float pixelHeight) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            SAGE_ERROR("Unable to open font file '{}'", path);
            return false;
        }

        const std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (fileSize <= 0) {
            SAGE_ERROR("Font file '{}' is empty", path);
            return false;
        }

        std::vector<unsigned char> fontBuffer(static_cast<std::size_t>(fileSize));
        if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize)) {
            SAGE_ERROR("Failed to read font file '{}'", path);
            return false;
        }

        return LoadFromBuffer(fontBuffer.data(), fontBuffer.size(), pixelHeight);
    }

    bool Font::LoadFromBuffer(const unsigned char* data, std::size_t size, float pixelHeight) {
        if (!data || size == 0) {
            SAGE_ERROR("Invalid font buffer supplied");
            return false;
        }

        m_FontData.assign(data, data + size);
        m_FontInfo = std::make_unique<stbtt_fontinfo>();

        if (!stbtt_InitFont(m_FontInfo.get(), m_FontData.data(), 0)) {
            SAGE_ERROR("stbtt_InitFont failed to initialize font data");
            m_FontInfo.reset();
            m_FontData.clear();
            return false;
        }

        m_Scale = stbtt_ScaleForPixelHeight(m_FontInfo.get(), pixelHeight);
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(m_FontInfo.get(), &ascent, &descent, &lineGap);

        m_LineHeight = static_cast<float>(ascent - descent + lineGap) * m_Scale;
        m_Ascent = static_cast<float>(ascent) * m_Scale;
        m_Descent = static_cast<float>(descent) * m_Scale;
        m_PixelHeight = pixelHeight;

        std::vector<unsigned char> bitmap(static_cast<std::size_t>(kTextureAtlasWidth) *
                                          kTextureAtlasHeight, 0);

        stbtt_pack_context packContext;
        if (!stbtt_PackBegin(&packContext,
                             bitmap.data(),
                             kTextureAtlasWidth,
                             kTextureAtlasHeight,
                             0,
                             1,
                             nullptr)) {
            SAGE_ERROR("stbtt_PackBegin failed to create font atlas");
            return false;
        }

        stbtt_PackSetOversampling(&packContext, 1, 1);

        struct Range {
            int first;
            int count;
        };

        constexpr std::array<Range, 4> glyphRanges = {
            Range{32, 95},     // Basic Latin
            Range{0x00A0, 96}, // Latin-1 Supplement subset
            Range{0x0400, 96}, // Cyrillic
            Range{0x2010, 48}  // Common punctuation
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
            packRanges[i].h_oversample = 1;
            packRanges[i].v_oversample = 1;
            glyphOffset += static_cast<std::size_t>(glyphRanges[i].count);
        }

        if (!stbtt_PackFontRanges(&packContext,
                                  m_FontData.data(),
                                  0,
                                  packRanges.data(),
                                  static_cast<int>(packRanges.size()))) {
            stbtt_PackEnd(&packContext);
            SAGE_ERROR("stbtt_PackFontRanges failed to pack glyph atlas");
            return false;
        }

        stbtt_PackEnd(&packContext);

        m_AtlasTexture = CreateRef<Texture>(
            static_cast<unsigned int>(kTextureAtlasWidth),
            static_cast<unsigned int>(kTextureAtlasHeight),
            Texture::Format::Red8,
            bitmap.data());
        GraphicsResourceManager::TrackTexture(m_AtlasTexture);

        if (!m_AtlasTexture || !m_AtlasTexture->IsLoaded()) {
            SAGE_ERROR("Failed to create font atlas texture");
            return false;
        }

        m_Glyphs.clear();
        m_Glyphs.reserve(totalGlyphs + 1);

        glyphOffset = 0;
        for (const auto& range : glyphRanges) {
            for (int i = 0; i < range.count; ++i) {
                const stbtt_packedchar& pc = packedChars[glyphOffset + static_cast<std::size_t>(i)];

                Glyph glyph;
                glyph.uvMin = Vector2(static_cast<float>(pc.x0) / kTextureAtlasWidth,
                                      static_cast<float>(pc.y0) / kTextureAtlasHeight);
                glyph.uvMax = Vector2(static_cast<float>(pc.x1) / kTextureAtlasWidth,
                                      static_cast<float>(pc.y1) / kTextureAtlasHeight);
                glyph.bearing = Vector2(pc.xoff, pc.yoff);
                glyph.extent = Vector2(pc.xoff2, pc.yoff2);
                glyph.size = Vector2(glyph.extent.x - glyph.bearing.x,
                                     glyph.extent.y - glyph.bearing.y);
                glyph.advance = pc.xadvance;

                const uint32_t codepoint = static_cast<uint32_t>(range.first + i);
                m_Glyphs.emplace(codepoint, glyph);
            }
            glyphOffset += static_cast<std::size_t>(range.count);
        }

        if (auto it = m_Glyphs.find(static_cast<uint32_t>('?')); it != m_Glyphs.end()) {
            m_FallbackGlyph = it->second;
        } else if (!m_Glyphs.empty()) {
            m_FallbackGlyph = m_Glyphs.begin()->second;
        } else {
            m_FallbackGlyph = Glyph{};
        }

        return true;
    }

} // namespace SAGE
