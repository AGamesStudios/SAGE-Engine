#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Rect.h"
#include <cstdint>

namespace SAGE {

// UV Coordinate System for texture mapping
struct UV {
    float u = 0.0f;
    float v = 0.0f;

    constexpr UV() = default;
    constexpr UV(float u, float v) : u(u), v(v) {}

    // Common UV coordinates
    static constexpr UV TopLeft() { return {0.0f, 0.0f}; }
    static constexpr UV TopRight() { return {1.0f, 0.0f}; }
    static constexpr UV BottomLeft() { return {0.0f, 1.0f}; }
    static constexpr UV BottomRight() { return {1.0f, 1.0f}; }
    static constexpr UV Center() { return {0.5f, 0.5f}; }

    // Flipping
    constexpr UV FlipHorizontal() const { return {1.0f - u, v}; }
    constexpr UV FlipVertical() const { return {u, 1.0f - v}; }
    constexpr UV Flip() const { return {1.0f - u, 1.0f - v}; }

    // Operators
    constexpr UV operator+(const UV& other) const { return {u + other.u, v + other.v}; }
    constexpr UV operator-(const UV& other) const { return {u - other.u, v - other.v}; }
    constexpr UV operator*(float scalar) const { return {u * scalar, v * scalar}; }
    constexpr UV operator/(float scalar) const { return {u / scalar, v / scalar}; }
};

// UV Rectangle for texture atlases
struct UVRect {
    float u = 0.0f;      // Left UV coordinate
    float v = 0.0f;      // Top UV coordinate
    float width = 1.0f;  // UV width
    float height = 1.0f; // UV height

    constexpr UVRect() = default;
    constexpr UVRect(float u, float v, float width, float height)
        : u(u), v(v), width(width), height(height) {}
    
    constexpr UVRect(const UV& topLeft, const UV& size)
        : u(topLeft.u), v(topLeft.v), width(size.u), height(size.v) {}

    // Get corners
    constexpr UV TopLeft() const { return {u, v}; }
    constexpr UV TopRight() const { return {u + width, v}; }
    constexpr UV BottomLeft() const { return {u, v + height}; }
    constexpr UV BottomRight() const { return {u + width, v + height}; }

    // Get all four corners in order (TL, TR, BR, BL)
    constexpr void GetCorners(UV corners[4]) const {
        corners[0] = TopLeft();
        corners[1] = TopRight();
        corners[2] = BottomRight();
        corners[3] = BottomLeft();
    }

    // Create from pixel coordinates in texture
    static constexpr UVRect FromPixels(int x, int y, int width, int height, 
                                       int textureWidth, int textureHeight) {
        if (textureWidth == 0 || textureHeight == 0) {
            return {0.0f, 0.0f, 1.0f, 1.0f};
        }
        return {
            static_cast<float>(x) / textureWidth,
            static_cast<float>(y) / textureHeight,
            static_cast<float>(width) / textureWidth,
            static_cast<float>(height) / textureHeight
        };
    }

    // Flip operations
    constexpr UVRect FlipHorizontal() const {
        return {u + width, v, -width, height};
    }

    constexpr UVRect FlipVertical() const {
        return {u, v + height, width, -height};
    }

    // Full texture
    static constexpr UVRect Full() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
};

// Texture Atlas helper - manages sprite sheets
class TextureAtlas {
public:
    TextureAtlas() = default;
    TextureAtlas(int textureWidth, int textureHeight, int spriteWidth, int spriteHeight);

    // Get UV rect for sprite at grid position
    UVRect GetSpriteUV(int gridX, int gridY) const;
    
    // Get UV rect for sprite by index (row-major order)
    UVRect GetSpriteUVByIndex(int index) const;

    // Grid dimensions
    int GetColumns() const { return m_Columns; }
    int GetRows() const { return m_Rows; }
    int GetTotalSprites() const { return m_Columns * m_Rows; }

    // Sprite size
    int GetSpriteWidth() const { return m_SpriteWidth; }
    int GetSpriteHeight() const { return m_SpriteHeight; }

    // Texture size
    int GetTextureWidth() const { return m_TextureWidth; }
    int GetTextureHeight() const { return m_TextureHeight; }

    // Set padding/spacing between sprites
    void SetSpacing(int horizontal, int vertical) {
        m_SpacingX = horizontal;
        m_SpacingY = vertical;
    }

    // Set margin from texture edge
    void SetMargin(int horizontal, int vertical) {
        m_MarginX = horizontal;
        m_MarginY = vertical;
    }

private:
    int m_TextureWidth = 0;
    int m_TextureHeight = 0;
    int m_SpriteWidth = 0;
    int m_SpriteHeight = 0;
    int m_Columns = 0;
    int m_Rows = 0;
    int m_SpacingX = 0;
    int m_SpacingY = 0;
    int m_MarginX = 0;
    int m_MarginY = 0;
};

} // namespace SAGE
