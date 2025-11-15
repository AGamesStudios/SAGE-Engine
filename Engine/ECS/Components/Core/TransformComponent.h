#pragma once

#include "Math/Vector2.h"
#include <cmath>
#include <algorithm>

namespace SAGE::ECS {

/// @brief Компонент позиции, поворота и масштаба
/// Заменяет поля GameObject: x, y, angle, width, height
struct TransformComponent {
    static constexpr float DefaultSize = 32.0f;

    Vector2 position = Vector2::Zero();        // Позиция (x, y) - ВСЕГДА ЦЕНТР ОБЪЕКТА для физики
    Vector2 scale = Vector2(1.0f, 1.0f);       // Масштаб (множители для size)
    Vector2 size = Vector2(DefaultSize, DefaultSize); // Базовые размеры (до применения scale)
    Vector2 pivot = Vector2(0.5f, 0.5f);       // Точка вращения/origin (0.5, 0.5 = центр)
    float rotation = 0.0f;                     // Угол поворота в ГРАДУСАХ (публичный для сериализации)

    TransformComponent() = default;
    
    TransformComponent(float x, float y, float rot = 0.0f)
        : position(x, y) {
        SetRotation(rot);
    }
    
    TransformComponent(const Vector2& pos, float rot = 0.0f, const Vector2& scl = Vector2(1.0f, 1.0f),
                       const Vector2& baseSize = Vector2(DefaultSize, DefaultSize))
        : position(pos), scale(scl), size(baseSize) {
        SetRotation(rot);
    }
    
    /// @brief Получить мировую позицию (центр объекта в экранных координатах)
    /// Position всегда указывает на центр объекта для корректной работы с физикой
    Vector2 GetWorldPosition() const {
        return position;
    }
    
    /// @brief Получить позицию с учетом pivot (для рендеринга)
    /// Pivot (0.5, 0.5) = центр, (0, 0) = левый верхний угол
    Vector2 GetRenderPosition() const {
        Vector2 worldSize(size.x * scale.x, size.y * scale.y);
        Vector2 pivotDiff = Vector2(0.5f, 0.5f) - pivot;
        Vector2 pivotOffset(worldSize.x * pivotDiff.x, worldSize.y * pivotDiff.y);
        return position + pivotOffset;
    }
    
    /// @brief Установить pivot точку (0,0 = top-left, 0.5,0.5 = center, 1,1 = bottom-right)
    void SetPivot(float x, float y) {
        pivot.x = std::clamp(x, 0.0f, 1.0f);
        pivot.y = std::clamp(y, 0.0f, 1.0f);
    }
    
    /// @brief Установить pivot в центр (по умолчанию для физики)
    void SetPivotCenter() {
        pivot = Vector2(0.5f, 0.5f);
    }
    
    /// @brief Установить rotation с нормализацией к [0, 360)
    void SetRotation(float degrees) {
        rotation = NormalizeAngle(degrees);
    }
    
    /// @brief Повернуть на заданный угол с нормализацией
    void Rotate(float deltaDegrees) {
        rotation = NormalizeAngle(rotation + deltaDegrees);
    }
    
    /// @brief Получить текущий угол rotation
    float GetRotation() const {
        return rotation;
    }

private:
    
    /// @brief Нормализует угол к диапазону [0, 360)
    static float NormalizeAngle(float degrees) {
        degrees = std::fmod(degrees, 360.0f);
        if (degrees < 0.0f) {
            degrees += 360.0f;
        }
        return degrees;
    }
};

} // namespace SAGE::ECS
