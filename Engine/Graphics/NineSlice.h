#pragma once

#include "Math/Vector2.h"
#include "Graphics/Core/Resources/Texture.h"
#include <memory>

namespace SAGE {

/**
 * @brief 9-Slice (Nine-Patch) система для масштабирования UI элементов
 * 
 * Разбивает спрайт на 9 частей:
 * ┌─────┬─────┬─────┐
 * │ TL  │  T  │ TR  │  TL/TR/BL/BR = углы (не масштабируются)
 * ├─────┼─────┼─────┤  T/B = верх/низ (масштабируются по X)
 * │  L  │  C  │  R  │  L/R = лево/право (масштабируются по Y)
 * ├─────┼─────┼─────┤  C = центр (масштабируется по X и Y)
 * │ BL  │  B  │ BR  │
 * └─────┴─────┴─────┘
 */
struct NineSliceSprite {
    std::shared_ptr<Texture> texture;
    
    // Границы в пикселях (отступы от краёв текстуры)
    float leftBorder = 0.0f;    ///< Ширина левого края
    float rightBorder = 0.0f;   ///< Ширина правого края
    float topBorder = 0.0f;     ///< Высота верхнего края
    float bottomBorder = 0.0f;  ///< Высота нижнего края
    
    // Размер результирующего элемента
    float width = 100.0f;
    float height = 100.0f;
    
    // UV координаты (если текстура из атласа)
    float uvLeft = 0.0f;
    float uvRight = 1.0f;
    float uvTop = 0.0f;
    float uvBottom = 1.0f;
    
    // Опции отрисовки
    bool fillCenter = true;     ///< Рисовать центральную часть
    bool tileCenter = false;    ///< Тайлить центр вместо растягивания
    
    NineSliceSprite() = default;
    
    /**
     * @brief Создать 9-slice спрайт
     * @param tex Текстура
     * @param left Ширина левого края
     * @param right Ширина правого края
     * @param top Высота верхнего края
     * @param bottom Высота нижнего края
     */
    NineSliceSprite(std::shared_ptr<Texture> tex, float left, float right, float top, float bottom)
        : texture(tex)
        , leftBorder(left)
        , rightBorder(right)
        , topBorder(top)
        , bottomBorder(bottom)
    {
        if (texture) {
            width = static_cast<float>(texture->GetWidth());
            height = static_cast<float>(texture->GetHeight());
        }
    }
    
    /**
     * @brief Установить размер элемента
     */
    void SetSize(float w, float h) {
        width = w;
        height = h;
    }
    
    /**
     * @brief Установить границы (все одинаковые)
     */
    void SetBorders(float border) {
        leftBorder = rightBorder = topBorder = bottomBorder = border;
    }
    
    /**
     * @brief Получить минимальный размер (сумма границ)
     */
    Vector2 GetMinimumSize() const {
        return Vector2(leftBorder + rightBorder, topBorder + bottomBorder);
    }
    
    /**
     * @brief Проверить валидность
     */
    bool IsValid() const {
        if (!texture) return false;
        
        float minWidth = leftBorder + rightBorder;
        float minHeight = topBorder + bottomBorder;
        
        return width >= minWidth && height >= minHeight;
    }
};

/**
 * @brief Информация о части 9-slice спрайта для отрисовки
 */
struct NineSlicePart {
    Vector2 position;       ///< Позиция в мировых координатах
    Vector2 size;           ///< Размер части
    Vector2 uvMin;          ///< UV координаты (левый верхний угол)
    Vector2 uvMax;          ///< UV координаты (правый нижний угол)
    bool shouldRender;      ///< Нужно ли рисовать эту часть
    
    NineSlicePart() : shouldRender(false) {}
};

/**
 * @brief Разбивает 9-slice спрайт на части для рендеринга
 */
class NineSliceSlicer {
public:
    /**
     * @brief Рассчитать все 9 частей спрайта
     * @param sprite 9-slice спрайт
     * @param position Позиция в мировых координатах
     * @param parts Выходной массив из 9 частей
     */
    static void Slice(const NineSliceSprite& sprite, const Vector2& position, NineSlicePart parts[9]);
    
    /**
     * @brief Индексы частей в массиве
     */
    enum PartIndex {
        TopLeft = 0,
        Top = 1,
        TopRight = 2,
        Left = 3,
        Center = 4,
        Right = 5,
        BottomLeft = 6,
        Bottom = 7,
        BottomRight = 8
    };
    
private:
    static void CalculateUVs(const NineSliceSprite& sprite, 
                            float& uvLeft, float& uvRight, float& uvTop, float& uvBottom,
                            float& uvCenterLeft, float& uvCenterRight, 
                            float& uvCenterTop, float& uvCenterBottom);
};

} // namespace SAGE
