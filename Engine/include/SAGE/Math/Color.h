#pragma once

#include <cstdint>

namespace SAGE {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r, float g, float b, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    // Create from 0-255 values
    static constexpr Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return {
            r / 255.0f,
            g / 255.0f,
            b / 255.0f,
            a / 255.0f
        };
    }

    // Create from hex
    static constexpr Color FromHex(uint32_t hex) {
        return {
            ((hex >> 24) & 0xFF) / 255.0f,
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f
        };
    }

    // Predefined colors
    static constexpr Color White()   { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color Black()   { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color Red()     { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color Green()   { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color Blue()    { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color Yellow()  { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color Magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color Cyan()    { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color Gray()    { return {0.5f, 0.5f, 0.5f, 1.0f}; }
    static constexpr Color Transparent() { return {0.0f, 0.0f, 0.0f, 0.0f}; }

    // Operators
    constexpr Color operator*(float scalar) const {
        return {r * scalar, g * scalar, b * scalar, a * scalar};
    }

    constexpr Color operator+(const Color& other) const {
        return {r + other.r, g + other.g, b + other.b, a + other.a};
    }

    static Color Lerp(const Color& a, const Color& b, float t) {
        return {
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        };
    }
};

} // namespace SAGE
