#pragma once

#include <Math/Vector2.h>

namespace SAGE {

    using Float2 = Vector2;
    using Vector2f = Vector2;

    /// @brief Rectangle structure for viewport and bounds
    struct Rect {
        float x = 0.0f;      ///< X position (left)
        float y = 0.0f;      ///< Y position (top)
        float width = 0.0f;  ///< Width
        float height = 0.0f; ///< Height

        /// Default constructor
        Rect() = default;

        /// Constructor with all parameters
        Rect(float x_, float y_, float w_, float h_)
            : x(x_), y(y_), width(w_), height(h_) {}

        /// Get right edge position
        float Right() const { return x + width; }

        /// Get bottom edge position
        float Bottom() const { return y + height; }

        /// Get center position
        Vector2 Center() const { return Vector2(x + width * 0.5f, y + height * 0.5f); }

        /// Check if point is inside rectangle
        bool Contains(const Vector2& point) const {
            return point.x >= x && point.x <= (x + width) &&
                   point.y >= y && point.y <= (y + height);
        }

        /// Check if this rectangle intersects with another
        bool Intersects(const Rect& other) const {
            return !(x > other.Right() || Right() < other.x ||
                     y > other.Bottom() || Bottom() < other.y);
        }
    };

} // namespace SAGE
