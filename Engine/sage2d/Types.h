#pragma once

#include <cstdint>

namespace sage2d {

    using ObjectId = std::uint32_t;
    constexpr ObjectId kInvalidObjectId = 0;

    struct Vec2 {
        float x{ 0.0f };
        float y{ 0.0f };

        constexpr Vec2() = default;
        constexpr Vec2(float inX, float inY) : x(inX), y(inY) {}

        constexpr Vec2& operator+=(const Vec2& rhs) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        constexpr Vec2& operator-=(const Vec2& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        friend constexpr Vec2 operator+(Vec2 lhs, const Vec2& rhs) {
            lhs += rhs;
            return lhs;
        }

        friend constexpr Vec2 operator-(Vec2 lhs, const Vec2& rhs) {
            lhs -= rhs;
            return lhs;
        }

        friend constexpr Vec2 operator*(Vec2 lhs, float scalar) {
            lhs.x *= scalar;
            lhs.y *= scalar;
            return lhs;
        }

        friend constexpr Vec2 operator*(float scalar, Vec2 rhs) {
            rhs.x *= scalar;
            rhs.y *= scalar;
            return rhs;
        }

        friend constexpr Vec2 operator/(Vec2 lhs, float scalar) {
            // Защита от деления на ноль (constexpr-совместимо)
            if (scalar != 0.0f && scalar != -0.0f) {
                lhs.x /= scalar;
                lhs.y /= scalar;
            } else {
                lhs.x = 0.0f;
                lhs.y = 0.0f;
            }
            return lhs;
        }
    };

} // namespace sage2d
