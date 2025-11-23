#pragma once

#include <cmath>

namespace SAGE {

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr Vector2(float x, float y) : x(x), y(y) {}

    // Arithmetic operators
    constexpr Vector2 operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }

    constexpr Vector2 operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    constexpr Vector2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    constexpr Vector2 operator*(const Vector2& other) const {
        return {x * other.x, y * other.y};
    }

    constexpr Vector2 operator/(float scalar) const {
        if (scalar == 0.0f) {
            return {0.0f, 0.0f};
        }
        return {x / scalar, y / scalar};
    }

    constexpr Vector2 operator-() const {
        return {-x, -y};
    }

    constexpr Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2& operator/=(float scalar) {
        if (scalar != 0.0f) {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    Vector2 Rotate(float angleRadians) const {
        float c = std::cos(angleRadians);
        float s = std::sin(angleRadians);
        return {x * c - y * s, x * s + y * c};
    }

    // Comparison
    constexpr bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    // Utility
    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    float LengthSquared() const {
        return x * x + y * y;
    }

    Vector2 Normalized() const {
        const float len = Length();
        return len > 0.0f ? (*this / len) : Vector2{};
    }

    void Normalize() {
        const float len = Length();
        if (len > 0.0f) {
            *this /= len;
        }
    }

    static float Dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static float Distance(const Vector2& a, const Vector2& b) {
        return (b - a).Length();
    }

    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) {
        return a + (b - a) * t;
    }

    // Common constants
    static constexpr Vector2 Zero() { return {0.0f, 0.0f}; }
    static constexpr Vector2 One() { return {1.0f, 1.0f}; }
    static constexpr Vector2 Up() { return {0.0f, 1.0f}; }
    static constexpr Vector2 Down() { return {0.0f, -1.0f}; }
    static constexpr Vector2 Left() { return {-1.0f, 0.0f}; }
    static constexpr Vector2 Right() { return {1.0f, 0.0f}; }
};

// Free function for scalar * vector
constexpr Vector2 operator*(float scalar, const Vector2& vec) {
    return vec * scalar;
}

} // namespace SAGE
