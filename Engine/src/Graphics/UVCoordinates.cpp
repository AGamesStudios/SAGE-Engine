#include "SAGE/Graphics/UVCoordinates.h"
#include "SAGE/Log.h"
#include <algorithm>

namespace SAGE {

TextureAtlas::TextureAtlas(int textureWidth, int textureHeight, int spriteWidth, int spriteHeight)
    : m_TextureWidth(textureWidth)
    , m_TextureHeight(textureHeight)
    , m_SpriteWidth(spriteWidth)
    , m_SpriteHeight(spriteHeight)
{
    if (textureWidth <= 0 || textureHeight <= 0) {
        SAGE_ERROR("TextureAtlas: Invalid texture dimensions {}x{}", textureWidth, textureHeight);
        return;
    }

    if (spriteWidth <= 0 || spriteHeight <= 0) {
        SAGE_ERROR("TextureAtlas: Invalid sprite dimensions {}x{}", spriteWidth, spriteHeight);
        return;
    }

    if (spriteWidth > textureWidth || spriteHeight > textureHeight) {
        SAGE_WARNING("TextureAtlas: Sprite size ({}x{}) exceeds texture size ({}x{})",
                     spriteWidth, spriteHeight, textureWidth, textureHeight);
    }

    m_Columns = std::max(1, textureWidth / spriteWidth);
    m_Rows = std::max(1, textureHeight / spriteHeight);

    SAGE_INFO("TextureAtlas: Created {}x{} grid ({} sprites) from {}x{} texture with {}x{} sprites",
              m_Columns, m_Rows, GetTotalSprites(), textureWidth, textureHeight, spriteWidth, spriteHeight);
}

UVRect TextureAtlas::GetSpriteUV(int gridX, int gridY) const {
    if (m_TextureWidth == 0 || m_TextureHeight == 0) {
        SAGE_ERROR("TextureAtlas::GetSpriteUV - Invalid texture dimensions");
        return UVRect::Full();
    }

    if (gridX < 0 || gridX >= m_Columns || gridY < 0 || gridY >= m_Rows) {
        SAGE_WARNING("TextureAtlas::GetSpriteUV - Grid position ({}, {}) out of bounds ({}x{})",
                     gridX, gridY, m_Columns, m_Rows);
        return UVRect::Full();
    }

    // Calculate pixel coordinates with margin and spacing
    int pixelX = m_MarginX + gridX * (m_SpriteWidth + m_SpacingX);
    int pixelY = m_MarginY + gridY * (m_SpriteHeight + m_SpacingY);

    // Ensure we don't exceed texture bounds
    int actualWidth = std::min(m_SpriteWidth, m_TextureWidth - pixelX);
    int actualHeight = std::min(m_SpriteHeight, m_TextureHeight - pixelY);

    if (actualWidth <= 0 || actualHeight <= 0) {
        SAGE_ERROR("TextureAtlas::GetSpriteUV - Calculated sprite dimensions are invalid");
        return UVRect::Full();
    }

    return UVRect::FromPixels(pixelX, pixelY, actualWidth, actualHeight, 
                              m_TextureWidth, m_TextureHeight);
}

UVRect TextureAtlas::GetSpriteUVByIndex(int index) const {
    if (index < 0 || index >= GetTotalSprites()) {
        SAGE_WARNING("TextureAtlas::GetSpriteUVByIndex - Index {} out of range (0-{})",
                     index, GetTotalSprites() - 1);
        return UVRect::Full();
    }

    if (m_Columns == 0) {
        SAGE_ERROR("TextureAtlas::GetSpriteUVByIndex - Invalid column count");
        return UVRect::Full();
    }

    int gridX = index % m_Columns;
    int gridY = index / m_Columns;

    return GetSpriteUV(gridX, gridY);
}

} // namespace SAGE
