#pragma once

namespace SAGE {

    struct Color {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        constexpr Color() = default;
        constexpr Color(float red, float green, float blue, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {}

        static constexpr Color White() { return Color(1.0f, 1.0f, 1.0f); }
        static constexpr Color Black() { return Color(0.0f, 0.0f, 0.0f); }
        static constexpr Color Red() { return Color(1.0f, 0.0f, 0.0f); }
        static constexpr Color Green() { return Color(0.0f, 1.0f, 0.0f); }
        static constexpr Color Blue() { return Color(0.0f, 0.0f, 1.0f); }
        static constexpr Color Yellow() { return Color(1.0f, 1.0f, 0.0f); }
        static constexpr Color Cyan() { return Color(0.0f, 1.0f, 1.0f); }
        static constexpr Color Magenta() { return Color(1.0f, 0.0f, 1.0f); }
        static constexpr Color Transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
    };

} // namespace SAGE
