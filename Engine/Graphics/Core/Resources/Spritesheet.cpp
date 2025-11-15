#include "Spritesheet.h"
#include "Core/Logger.h"
#include <algorithm>

namespace SAGE {

    Spritesheet::Spritesheet(const Ref<Texture>& texture, unsigned int cellWidth, unsigned int cellHeight, unsigned int margin, unsigned int spacing)
        : m_Texture(texture), m_CellWidth(cellWidth), m_CellHeight(cellHeight), m_Margin(margin), m_Spacing(spacing) {
        if(texture && cellWidth && cellHeight) {
            RebuildFrames();
        }
    }

    void Spritesheet::RebuildFrames() {
        if(!m_Texture || m_CellWidth == 0 || m_CellHeight == 0) return;
        unsigned int texW = m_Texture->GetWidth();
        unsigned int texH = m_Texture->GetHeight();

        // Compute columns & rows considering margin + spacing
        if(texW < m_Margin || texH < m_Margin) return;
        unsigned int usableW = texW - 2 * m_Margin + m_Spacing; // add spacing once for formula
        unsigned int usableH = texH - 2 * m_Margin + m_Spacing;
        m_Columns = (usableW) / (m_CellWidth + m_Spacing);
        m_Rows    = (usableH) / (m_CellHeight + m_Spacing);
        m_Frames.clear();
        m_Frames.reserve(m_Columns * m_Rows);

        for(unsigned int y = 0; y < m_Rows; ++y) {
            for(unsigned int x = 0; x < m_Columns; ++x) {
                unsigned int px = m_Margin + x * (m_CellWidth + m_Spacing);
                unsigned int py = m_Margin + y * (m_CellHeight + m_Spacing);
                float u0 = static_cast<float>(px) / static_cast<float>(texW);
                float v0 = static_cast<float>(py) / static_cast<float>(texH);
                float u1 = static_cast<float>(px + m_CellWidth) / static_cast<float>(texW);
                float v1 = static_cast<float>(py + m_CellHeight) / static_cast<float>(texH);
                SpriteFrame frame;
                frame.Index = y * m_Columns + x;
                frame.X = x; frame.Y = y;
                frame.UVMin = Float2(u0, v0);
                frame.UVMax = Float2(u1, v1);
                frame.PixelX = px;
                frame.PixelY = py;
                frame.PixelW = m_CellWidth;
                frame.PixelH = m_CellHeight;
                m_Frames.push_back(frame);
            }
        }
        SAGE_INFO("Spritesheet: built {} frames ({}x{})", m_Frames.size(), m_Columns, m_Rows);
    }

    // Simple heuristic grid detection: try common cell sizes (8,16,24,32,48,64). We look for strong vertical & horizontal separator lines of fully transparent pixels.
    std::pair<unsigned int, unsigned int> Spritesheet::DetectGrid(unsigned int texWidth, unsigned int texHeight, const std::vector<unsigned char>& pixels) {
        if(pixels.empty() || pixels.size() < texWidth * texHeight * 4) return {0,0};
        const unsigned int candidates[] = {8,16,24,32,48,64};
        auto isTransparentColumn = [&](unsigned int x){
            for(unsigned int y=0; y<texHeight; ++y){
                const unsigned char* p = &pixels[(y*texWidth + x)*4];
                if(p[3] != 0) return false; // alpha not zero
            }
            return true;
        };
        auto isTransparentRow = [&](unsigned int y){
            for(unsigned int x=0; x<texWidth; ++x){
                const unsigned char* p = &pixels[(y*texWidth + x)*4];
                if(p[3] != 0) return false;
            }
            return true;
        };

        for(unsigned int size : candidates){
            if(texWidth % size != 0 || texHeight % size != 0) continue;
            // Count separator lines
            unsigned int verticalSeparators = 0;
            for(unsigned int x = size; x < texWidth; x += size){ if(isTransparentColumn(x-1)) ++verticalSeparators; }
            unsigned int horizontalSeparators = 0;
            for(unsigned int y = size; y < texHeight; y += size){ if(isTransparentRow(y-1)) ++horizontalSeparators; }
            unsigned int expectedV = (texWidth / size) - 1;
            unsigned int expectedH = (texHeight / size) - 1;
            if(verticalSeparators == expectedV && horizontalSeparators == expectedH){
                return {size,size};
            }
        }
        return {0,0};
    }

    Spritesheet::GridAnalysisResult Spritesheet::Analyze(unsigned int texWidth, unsigned int texHeight, const std::vector<unsigned char>& pixels, unsigned minCell, unsigned maxCell, bool requireSquare, bool allowOpaqueLines) {
        GridAnalysisResult result; result.imageW = texWidth; result.imageH = texHeight;
        if(pixels.empty() || pixels.size() < texWidth*texHeight*4) return result;
        auto isTransparentColumn = [&](unsigned int x){
            for(unsigned int y=0; y<texHeight; ++y){ const unsigned char* p=&pixels[(y*texWidth + x)*4]; if(p[3] != 0) return false; } return true; };
        auto isTransparentRow = [&](unsigned int y){
            for(unsigned int x=0; x<texWidth; ++x){ const unsigned char* p=&pixels[(y*texWidth + x)*4]; if(p[3] != 0) return false; } return true; };
        auto opaqueLineStatsColumn = [&](unsigned int x){
            unsigned same=0; unsigned total=texHeight; unsigned char rMode=0,gMode=0,bMode=0; // naive mode via first pixel
            const unsigned char* first=&pixels[(0*texWidth + x)*4]; rMode=first[0]; gMode=first[1]; bMode=first[2];
            for(unsigned int y=0;y<texHeight;++y){ const unsigned char* p=&pixels[(y*texWidth + x)*4]; if(p[0]==rMode && p[1]==gMode && p[2]==bMode) ++same; }
            return float(same)/float(total);
        };
        auto opaqueLineStatsRow = [&](unsigned int y){
            unsigned same=0; unsigned total=texWidth; const unsigned char* first=&pixels[(y*texWidth + 0)*4]; unsigned char rMode=first[0],gMode=first[1],bMode=first[2];
            for(unsigned int x=0;x<texWidth;++x){ const unsigned char* p=&pixels[(y*texWidth + x)*4]; if(p[0]==rMode && p[1]==gMode && p[2]==bMode) ++same; }
            return float(same)/float(total);
        };
        auto contrastColumn = [&](unsigned int x){
            // Compare column x vs x+1 if x+1 < width
            if(x+1>=texWidth) return 0.0f; double sum=0; unsigned cnt=0; for(unsigned int y=0;y<texHeight;++y){ const unsigned char* a=&pixels[(y*texWidth + x)*4]; const unsigned char* b=&pixels[(y*texWidth + x+1)*4]; sum += std::abs(int(a[0])-int(b[0])) + std::abs(int(a[1])-int(b[1])) + std::abs(int(a[2])-int(b[2])); ++cnt; } return float(sum/(cnt*255*3)); };
        auto contrastRow = [&](unsigned int y){ if(y+1>=texHeight) return 0.0f; double sum=0; unsigned cnt=0; for(unsigned int x=0;x<texWidth;++x){ const unsigned char* a=&pixels[(y*texWidth + x)*4]; const unsigned char* b=&pixels[((y+1)*texWidth + x)*4]; sum += std::abs(int(a[0])-int(b[0])) + std::abs(int(a[1])-int(b[1])) + std::abs(int(a[2])-int(b[2])); ++cnt; } return float(sum/(cnt*255*3)); };

        for(unsigned cell=minCell; cell<=maxCell; ++cell){
            if(texWidth % cell !=0 || texHeight % cell !=0) continue;
            unsigned cols = texWidth / cell; unsigned rows = texHeight / cell;
            if(requireSquare && cols!=rows) {/* allow but lower score later */}
            GridCandidate cand; cand.cellW=cell; cand.cellH=cell; cand.cols=cols; cand.rows=rows;
            unsigned expectedV = cols - 1; unsigned expectedH = rows - 1;
            unsigned transV=0, transH=0; unsigned opaqueV=0, opaqueH=0; double alphaSum=0; double uniformSum=0; double contrastSum=0; unsigned contrastLines=0;
            // Check vertical boundaries at multiples of cell (treat boundary as previous pixel column)
            for(unsigned x=cell; x<texWidth; x+=cell){ unsigned boundary = x-1; bool isTrans = isTransparentColumn(boundary); if(isTrans){ ++transV; alphaSum += 0.0; } else if(allowOpaqueLines){ float uniform=opaqueLineStatsColumn(boundary); if(uniform>0.9f){ ++opaqueV; uniformSum += uniform; }
                // compute alpha mean anyway
                double aSum=0; for(unsigned y=0;y<texHeight;++y){ const unsigned char* p=&pixels[(y*texWidth + boundary)*4]; aSum += p[3]/255.0; } alphaSum += aSum/texHeight; }
                contrastSum += contrastColumn(boundary); ++contrastLines; }
            for(unsigned y=cell; y<texHeight; y+=cell){ unsigned boundary = y-1; bool isTrans = isTransparentRow(boundary); if(isTrans){ ++transH; alphaSum += 0.0; } else if(allowOpaqueLines){ float uniform=opaqueLineStatsRow(boundary); if(uniform>0.9f){ ++opaqueH; uniformSum += uniform; }
                double aSum=0; for(unsigned x=0;x<texWidth;++x){ const unsigned char* p=&pixels[(y*texWidth + x)*4]; aSum += p[3]/255.0; } alphaSum += aSum/texWidth; }
                contrastSum += contrastRow(boundary); ++contrastLines; }
            if(expectedV>0 || expectedH>0){ cand.transparentSeparators = (transV==expectedV && transH==expectedH); cand.opaqueSeparators = (opaqueV==expectedV && opaqueH==expectedH); }
            unsigned totalBoundaries = expectedV + expectedH; if(totalBoundaries>0){ cand.alphaMean = float(alphaSum / double(totalBoundaries)); }
            unsigned opaqueLines = opaqueV + opaqueH; if(opaqueLines>0){ cand.colorUniformity = float(uniformSum / double(opaqueLines)); }
            cand.edgeContrast = contrastLines>0 ? float(contrastSum / contrastLines) : 0.0f;
            // Score composition
            float sepScore = cand.transparentSeparators ? 1.0f : (cand.opaqueSeparators?0.9f:0.0f);
            float uniformScore = std::min(1.0f, cand.colorUniformity);
            float contrastScore = cand.edgeContrast; // already normalized
            float squareBonus = (!requireSquare || cols==rows) ? 0.05f : 0.0f;
            cand.score = sepScore + 0.4f*uniformScore + 0.3f*contrastScore + squareBonus;
            result.candidates.push_back(cand);
        }
        // Select best
        float best = -1.0f; for(size_t i=0;i<result.candidates.size();++i){ if(result.candidates[i].score>best){ best=result.candidates[i].score; result.bestIndex=(int)i; } }
        return result;
    }

} // namespace SAGE
