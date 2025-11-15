#pragma once

#include "Core/Color.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Memory/Ref.h"

#include <string>

namespace SAGE {
class Texture;
class Material;
}

namespace SAGE::ECS {

/// @brief Компонент для рендеринга спрайтов
/// Заменяет поля GameObject: image, color, alpha, visible, flipX, flipY
struct SpriteComponent {
    std::string texturePath;                 // Путь к текстуре
    Ref<Texture> texture = nullptr;          // Загруженная текстура
    Ref<Material> material = nullptr;        // Материал для рендеринга
    
    Color tint = Color::White();             // Цвет тинта (включает alpha)
    bool visible = true;                     // Видимость
    bool flipX = false;                      // Отразить по X
    bool flipY = false;                      // Отразить по Y
    
    int layer = 0;                           // Слой рендеринга (для сортировки)
    
    // UV координаты для анимации и sprite sheet
    // По умолчанию (0,0) - (1,1) = полная текстура
    Float2 uvMin = {0.0f, 0.0f};             // Top-left UV
    Float2 uvMax = {1.0f, 1.0f};             // Bottom-right UV
    
    // Pivot point (нормализованный 0-1 относительно размера спрайта)
    Float2 pivot = {0.5f, 0.5f};             // Default: center

    SpriteComponent() = default;
    
    explicit SpriteComponent(const std::string& path)
        : texturePath(path) {}
    
    SpriteComponent(const std::string& path, const Color& color)
        : texturePath(path), tint(color) {}
    
    // Helper: Set UV region from pixel coordinates
    void SetUVRegion(float texWidth, float texHeight, float x, float y, float w, float h) {
        // Validate texture dimensions
        if (texWidth <= 0.0f || texHeight <= 0.0f) {
            std::fprintf(stderr, "SpriteComponent::SetUVRegion - Invalid texture dimensions: %.2f x %.2f\n", 
                         texWidth, texHeight);
            uvMin = Float2(0.0f, 0.0f);
            uvMax = Float2(1.0f, 1.0f);
            return;
        }
        
        // Handle negative width/height (flip)
        float actualX = x;
        float actualY = y;
        float actualW = w;
        float actualH = h;
        
        if (w < 0.0f) {
            actualX = x + w;
            actualW = -w;
        }
        if (h < 0.0f) {
            actualY = y + h;
            actualH = -h;
        }
        
        // Clamp to texture bounds
        actualX = std::max(0.0f, std::min(actualX, texWidth));
        actualY = std::max(0.0f, std::min(actualY, texHeight));
        actualW = std::max(0.0f, std::min(actualW, texWidth - actualX));
        actualH = std::max(0.0f, std::min(actualH, texHeight - actualY));
        
        uvMin.x = actualX / texWidth;
        uvMin.y = actualY / texHeight;
        uvMax.x = (actualX + actualW) / texWidth;
        uvMax.y = (actualY + actualH) / texHeight;
    }
};

} // namespace SAGE::ECS
