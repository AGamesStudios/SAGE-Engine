#pragma once

#include "SAGE/Math/Vector2.h"

namespace SAGE {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height) {}
    
    constexpr Rect(const Vector2& position, const Vector2& size)
        : x(position.x), y(position.y), width(size.x), height(size.y) {}

    constexpr Vector2 GetPosition() const { return {x, y}; }
    constexpr Vector2 GetSize() const { return {width, height}; }
    constexpr Vector2 GetCenter() const { return {x + width * 0.5f, y + height * 0.5f}; }

    constexpr float Left() const { return x; }
    constexpr float Right() const { return x + width; }
    constexpr float Top() const { return y + height; }
    constexpr float Bottom() const { return y; }

    constexpr bool Contains(const Vector2& point) const {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }

    constexpr bool Intersects(const Rect& other) const {
        return !(Right() < other.Left() || Left() > other.Right() ||
                 Top() < other.Bottom() || Bottom() > other.Top());
    }

    static constexpr Rect FromCenter(const Vector2& center, const Vector2& size) {
        return {center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y};
    }

    static constexpr Rect Zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
};

} // namespace SAGE
