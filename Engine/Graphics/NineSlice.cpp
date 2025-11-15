#include "NineSlice.h"
#include <algorithm>

namespace SAGE {

void NineSliceSlicer::Slice(const NineSliceSprite& sprite, const Vector2& position, NineSlicePart parts[9]) {
    if (!sprite.IsValid()) {
        // Пометить все части как не рендерящиеся
        for (int i = 0; i < 9; ++i) {
            parts[i].shouldRender = false;
        }
        return;
    }
    
    // Размеры текстуры
    float texWidth = sprite.texture ? static_cast<float>(sprite.texture->GetWidth()) : sprite.width;
    float texHeight = sprite.texture ? static_cast<float>(sprite.texture->GetHeight()) : sprite.height;
    
    // Рассчитать UV координаты
    float uvLeft, uvRight, uvTop, uvBottom;
    float uvCenterLeft, uvCenterRight, uvCenterTop, uvCenterBottom;
    CalculateUVs(sprite, uvLeft, uvRight, uvTop, uvBottom, 
                 uvCenterLeft, uvCenterRight, uvCenterTop, uvCenterBottom);
    
    // Размеры центральной части
    float centerWidth = sprite.width - sprite.leftBorder - sprite.rightBorder;
    float centerHeight = sprite.height - sprite.topBorder - sprite.bottomBorder;
    
    // Проверить минимальный размер
    if (centerWidth < 0.0f) centerWidth = 0.0f;
    if (centerHeight < 0.0f) centerHeight = 0.0f;
    
    // === Top Left (угол) ===
    parts[TopLeft].position = position;
    parts[TopLeft].size = Vector2(sprite.leftBorder, sprite.topBorder);
    parts[TopLeft].uvMin = Vector2(uvLeft, uvTop);
    parts[TopLeft].uvMax = Vector2(uvCenterLeft, uvCenterTop);
    parts[TopLeft].shouldRender = (sprite.leftBorder > 0.0f && sprite.topBorder > 0.0f);
    
    // === Top (масштабируется по X) ===
    parts[Top].position = Vector2(position.x + sprite.leftBorder, position.y);
    parts[Top].size = Vector2(centerWidth, sprite.topBorder);
    parts[Top].uvMin = Vector2(uvCenterLeft, uvTop);
    parts[Top].uvMax = Vector2(uvCenterRight, uvCenterTop);
    parts[Top].shouldRender = (centerWidth > 0.0f && sprite.topBorder > 0.0f);
    
    // === Top Right (угол) ===
    parts[TopRight].position = Vector2(position.x + sprite.leftBorder + centerWidth, position.y);
    parts[TopRight].size = Vector2(sprite.rightBorder, sprite.topBorder);
    parts[TopRight].uvMin = Vector2(uvCenterRight, uvTop);
    parts[TopRight].uvMax = Vector2(uvRight, uvCenterTop);
    parts[TopRight].shouldRender = (sprite.rightBorder > 0.0f && sprite.topBorder > 0.0f);
    
    // === Left (масштабируется по Y) ===
    parts[Left].position = Vector2(position.x, position.y + sprite.topBorder);
    parts[Left].size = Vector2(sprite.leftBorder, centerHeight);
    parts[Left].uvMin = Vector2(uvLeft, uvCenterTop);
    parts[Left].uvMax = Vector2(uvCenterLeft, uvCenterBottom);
    parts[Left].shouldRender = (sprite.leftBorder > 0.0f && centerHeight > 0.0f);
    
    // === Center (масштабируется по X и Y) ===
    parts[Center].position = Vector2(position.x + sprite.leftBorder, position.y + sprite.topBorder);
    parts[Center].size = Vector2(centerWidth, centerHeight);
    parts[Center].uvMin = Vector2(uvCenterLeft, uvCenterTop);
    parts[Center].uvMax = Vector2(uvCenterRight, uvCenterBottom);
    parts[Center].shouldRender = sprite.fillCenter && (centerWidth > 0.0f && centerHeight > 0.0f);
    
    // === Right (масштабируется по Y) ===
    parts[Right].position = Vector2(position.x + sprite.leftBorder + centerWidth, position.y + sprite.topBorder);
    parts[Right].size = Vector2(sprite.rightBorder, centerHeight);
    parts[Right].uvMin = Vector2(uvCenterRight, uvCenterTop);
    parts[Right].uvMax = Vector2(uvRight, uvCenterBottom);
    parts[Right].shouldRender = (sprite.rightBorder > 0.0f && centerHeight > 0.0f);
    
    // === Bottom Left (угол) ===
    parts[BottomLeft].position = Vector2(position.x, position.y + sprite.topBorder + centerHeight);
    parts[BottomLeft].size = Vector2(sprite.leftBorder, sprite.bottomBorder);
    parts[BottomLeft].uvMin = Vector2(uvLeft, uvCenterBottom);
    parts[BottomLeft].uvMax = Vector2(uvCenterLeft, uvBottom);
    parts[BottomLeft].shouldRender = (sprite.leftBorder > 0.0f && sprite.bottomBorder > 0.0f);
    
    // === Bottom (масштабируется по X) ===
    parts[Bottom].position = Vector2(position.x + sprite.leftBorder, position.y + sprite.topBorder + centerHeight);
    parts[Bottom].size = Vector2(centerWidth, sprite.bottomBorder);
    parts[Bottom].uvMin = Vector2(uvCenterLeft, uvCenterBottom);
    parts[Bottom].uvMax = Vector2(uvCenterRight, uvBottom);
    parts[Bottom].shouldRender = (centerWidth > 0.0f && sprite.bottomBorder > 0.0f);
    
    // === Bottom Right (угол) ===
    parts[BottomRight].position = Vector2(position.x + sprite.leftBorder + centerWidth, 
                                          position.y + sprite.topBorder + centerHeight);
    parts[BottomRight].size = Vector2(sprite.rightBorder, sprite.bottomBorder);
    parts[BottomRight].uvMin = Vector2(uvCenterRight, uvCenterBottom);
    parts[BottomRight].uvMax = Vector2(uvRight, uvBottom);
    parts[BottomRight].shouldRender = (sprite.rightBorder > 0.0f && sprite.bottomBorder > 0.0f);
}

void NineSliceSlicer::CalculateUVs(const NineSliceSprite& sprite, 
                                   float& uvLeft, float& uvRight, float& uvTop, float& uvBottom,
                                   float& uvCenterLeft, float& uvCenterRight, 
                                   float& uvCenterTop, float& uvCenterBottom) {
    // Полный диапазон UV
    uvLeft = sprite.uvLeft;
    uvRight = sprite.uvRight;
    uvTop = sprite.uvTop;
    uvBottom = sprite.uvBottom;
    
    // Размеры текстуры
    float texWidth = sprite.texture ? static_cast<float>(sprite.texture->GetWidth()) : sprite.width;
    float texHeight = sprite.texture ? static_cast<float>(sprite.texture->GetHeight()) : sprite.height;
    
    // Рассчитать UV для границ
    float uvRangeX = uvRight - uvLeft;
    float uvRangeY = uvBottom - uvTop;
    
    uvCenterLeft = uvLeft + (sprite.leftBorder / texWidth) * uvRangeX;
    uvCenterRight = uvRight - (sprite.rightBorder / texWidth) * uvRangeX;
    uvCenterTop = uvTop + (sprite.topBorder / texHeight) * uvRangeY;
    uvCenterBottom = uvBottom - (sprite.bottomBorder / texHeight) * uvRangeY;
}

} // namespace SAGE
