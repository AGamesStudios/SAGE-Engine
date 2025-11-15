#pragma once

namespace SAGE {
namespace Math {

/// @brief Математические константы
namespace Constants {
    constexpr float PI = 3.14159265359f;
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float HALF_PI = 1.57079632679f;
    constexpr float DEG_TO_RAD = 0.01745329251f; // PI / 180
    constexpr float RAD_TO_DEG = 57.2957795131f; // 180 / PI
    constexpr float EPSILON = 1e-6f;
}

/// @brief Конверсия градусов в радианы (compile-time)
constexpr float ToRadians(float degrees) {
    return degrees * Constants::DEG_TO_RAD;
}

/// @brief Конверсия радианов в градусы (compile-time)
constexpr float ToDegrees(float radians) {
    return radians * Constants::RAD_TO_DEG;
}

/// @brief Ограничить значение между min и max
template<typename T>
constexpr T Clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

/// @brief Ограничить значение между 0 и 1
constexpr float Clamp01(float value) {
    return Clamp(value, 0.0f, 1.0f);
}

} // namespace Math
} // namespace SAGE
