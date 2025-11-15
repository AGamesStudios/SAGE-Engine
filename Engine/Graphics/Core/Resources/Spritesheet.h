#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Types/MathTypes.h"
#include <vector>
#include <string>
#include <cstdint>

namespace SAGE {

    struct SpriteFrame {
        unsigned int Index = 0;      // Linear index
        unsigned int X = 0;          // Column
        unsigned int Y = 0;          // Row
        Float2 UVMin{0.0f, 0.0f};
        Float2 UVMax{1.0f, 1.0f};
        // Pixel-space rectangle for precision or collision usage
        unsigned int PixelX = 0;
        unsigned int PixelY = 0;
        unsigned int PixelW = 0;
        unsigned int PixelH = 0;
    };

    /**
     * Spritesheet: grid-based atlas description. Assumes evenly sized cells.
     * Provides UV lookup for a given frame index or (x,y) coordinate.
     */
    class Spritesheet {
    public:
        Spritesheet() = default;
        Spritesheet(const Ref<Texture>& texture, unsigned int cellWidth, unsigned int cellHeight, unsigned int margin = 0, unsigned int spacing = 0);

        void SetTexture(const Ref<Texture>& tex) { m_Texture = tex; if(m_CellWidth && m_CellHeight && tex) RebuildFrames(); }
        const Ref<Texture>& GetTexture() const { return m_Texture; }

        unsigned int GetColumns() const { return m_Columns; }
        unsigned int GetRows() const { return m_Rows; }
        unsigned int GetCellWidth() const { return m_CellWidth; }
        unsigned int GetCellHeight() const { return m_CellHeight; }
        unsigned int GetFrameCount() const { return static_cast<unsigned int>(m_Frames.size()); }

        const SpriteFrame& GetFrame(unsigned int index) const { return m_Frames[index]; }
        const SpriteFrame& GetFrame(unsigned int x, unsigned int y) const { return m_Frames[y * m_Columns + x]; }
        bool IsValidFrame(unsigned int index) const { return index < m_Frames.size(); }
        std::tuple<unsigned int,unsigned int,unsigned int,unsigned int> GetFramePixelRect(unsigned int index) const {
            const auto& f = GetFrame(index); return {f.PixelX,f.PixelY,f.PixelW,f.PixelH};
        }

        // Convenience: returns UV pair for index
        std::pair<Float2, Float2> GetUV(unsigned int index) const { const auto& f = GetFrame(index); return { f.UVMin, f.UVMax }; }

        // Static helper: attempt to detect uniform cell size; returns (w,h) or {0,0} if not divisible.
        static std::pair<unsigned int, unsigned int> DetectGrid(unsigned int texWidth, unsigned int texHeight, const std::vector<unsigned char>& pixels);

        struct GridCandidate {
            unsigned cellW = 0;
            unsigned cellH = 0;
            unsigned cols = 0;
            unsigned rows = 0;
            bool transparentSeparators = false;
            bool opaqueSeparators = false;
            float alphaMean = 0.0f;        // average alpha on separator lines (if transparent)
            float colorUniformity = 0.0f;  // % of dominant color on separator lines (if opaque)
            float edgeContrast = 0.0f;     // normalized contrast across boundaries
            float score = 0.0f;            // composite score
        };

        struct GridAnalysisResult {
            unsigned imageW = 0;
            unsigned imageH = 0;
            std::vector<GridCandidate> candidates;
            int bestIndex = -1;
        };

        // Advanced analysis enumerating multiple candidate cell sizes.
        static GridAnalysisResult Analyze(unsigned int texWidth, unsigned int texHeight, const std::vector<unsigned char>& pixels, unsigned minCell = 8, unsigned maxCell = 128, bool requireSquare = true, bool allowOpaqueLines = true);

        // Convenience factory
        static Spritesheet CreateFromTexture(const Ref<Texture>& texture,
                                             unsigned int cellWidth,
                                             unsigned int cellHeight,
                                             unsigned int margin = 0,
                                             unsigned int spacing = 0) {
            return Spritesheet(texture, cellWidth, cellHeight, margin, spacing);
        }

        // Basic auto-analysis: if analysis finds best candidate, build sheet from that cell size.
        // Returns sheet with frames or empty (texture null or no candidate).
        static Spritesheet AutoAnalyzeFromPixels(const Ref<Texture>& texture,
                                                 const std::vector<unsigned char>& pixels,
                                                 unsigned int minCell = 8,
                                                 unsigned int maxCell = 128) {
            Spritesheet sheet; if(!texture) return sheet;
            auto res = Analyze(texture->GetWidth(), texture->GetHeight(), pixels, minCell, maxCell, true, true);
            if(res.bestIndex >= 0) {
                const auto& cand = res.candidates[res.bestIndex];
                sheet = Spritesheet(texture, cand.cellW, cand.cellH, 0, 0);
            }
            return sheet;
        }

    private:
        void RebuildFrames();

        Ref<Texture> m_Texture;
        unsigned int m_CellWidth = 0;
        unsigned int m_CellHeight = 0;
        unsigned int m_Margin = 0;
        unsigned int m_Spacing = 0;
        unsigned int m_Columns = 0;
        unsigned int m_Rows = 0;
        std::vector<SpriteFrame> m_Frames;
    };

} // namespace SAGE
