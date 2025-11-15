#pragma once

#include "Graphics/NineSlice.h"
#include "Math/Vector2.h"

namespace SAGE::ECS {

/**
 * @brief Компонент для 9-slice спрайтов (UI панели, кнопки, окна)
 */
struct NineSliceComponent {
    NineSliceSprite sprite;
    
    // Параметры отрисовки
    int layer = 0;                  ///< Слой отрисовки
    float opacity = 1.0f;           ///< Прозрачность (0-1)
    bool visible = true;            ///< Видимость
    
    // Цвет модуляции
    float colorR = 1.0f;
    float colorG = 1.0f;
    float colorB = 1.0f;
    float colorA = 1.0f;
    
    NineSliceComponent() = default;
    
    /**
     * @brief Создать 9-slice компонент
     * @param texture Текстура
     * @param borders Размер границ (все одинаковые)
     */
    NineSliceComponent(std::shared_ptr<Texture> texture, float borders)
        : sprite(texture, borders, borders, borders, borders) 
    {
    }
    
    /**
     * @brief Создать 9-slice компонент с разными границами
     */
    NineSliceComponent(std::shared_ptr<Texture> texture, float left, float right, float top, float bottom)
        : sprite(texture, left, right, top, bottom)
    {
    }
    
    /**
     * @brief Установить размер элемента
     */
    void SetSize(float width, float height) {
        sprite.SetSize(width, height);
    }
    
    /**
     * @brief Установить цвет
     */
    void SetColor(float r, float g, float b, float a = 1.0f) {
        colorR = r;
        colorG = g;
        colorB = b;
        colorA = a;
    }
};

} // namespace SAGE::ECS
