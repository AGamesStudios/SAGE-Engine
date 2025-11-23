#pragma once

#include "SAGE/Math/Vector2.h"
#include <array>
#include <cmath>

namespace SAGE {

class Matrix3 {
public:
    std::array<float, 9> m = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    Matrix3() = default;

    float& operator()(int row, int col) {
        return m[row * 3 + col];
    }

    const float& operator()(int row, int col) const {
        return m[row * 3 + col];
    }

    // Matrix multiplication (optimized for 2D affine transforms)
    Matrix3 operator*(const Matrix3& other) const {
        Matrix3 result;
        
        // Row-major multiplication for clarity
        // result[i][j] = sum(this[i][k] * other[k][j])
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                float sum = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    sum += (*this)(row, k) * other(k, col);
                }
                result(row, col) = sum;
            }
        }
        return result;
    }

    // Transform a point (uses translation component)
    // Point is treated as homogeneous coordinate (x, y, 1)
    Vector2 TransformPoint(const Vector2& point) const {
        const float x = m[0] * point.x + m[1] * point.y + m[2];
        const float y = m[3] * point.x + m[4] * point.y + m[5];
        const float w = m[6] * point.x + m[7] * point.y + m[8];
        
        // Perspective divide for homogeneous coordinates
        if (std::abs(w) > 1e-6f && std::abs(w - 1.0f) > 1e-6f) {
            return {x / w, y / w};
        }
        
        return {x, y};
    }

    // Transform a vector (ignores translation)
    // Vector is treated as homogeneous coordinate (x, y, 0)
    Vector2 TransformVector(const Vector2& vec) const {
        return {
            m[0] * vec.x + m[1] * vec.y,
            m[3] * vec.x + m[4] * vec.y
        };
    }

    static Matrix3 Identity() {
        return Matrix3{};
    }

    static Matrix3 Translation(const Vector2& offset) {
        Matrix3 mat;
        mat.m[2] = offset.x;
        mat.m[5] = offset.y;
        return mat;
    }

    static Matrix3 Rotation(float angleRadians) {
        const float c = std::cos(angleRadians);
        const float s = std::sin(angleRadians);
        Matrix3 mat;
        mat.m[0] = c;  mat.m[1] = -s;
        mat.m[3] = s;  mat.m[4] = c;
        return mat;
    }

    static Matrix3 Scale(const Vector2& scale) {
        Matrix3 mat;
        mat.m[0] = scale.x;
        mat.m[4] = scale.y;
        return mat;
    }

    static Matrix3 Scale(float uniformScale) {
        return Scale({uniformScale, uniformScale});
    }

    // 2D orthographic projection (improved precision)
    // Maps world coordinates to NDC [-1, 1] x [-1, 1]
    static Matrix3 Ortho(float left, float right, float bottom, float top) {
        Matrix3 mat;
        
        // Avoid division by zero
        const float width = right - left;
        const float height = top - bottom;
        
        if (std::abs(width) < 1e-6f || std::abs(height) < 1e-6f) {
            return Identity();
        }
        
        // Scale: map [left, right] -> [-1, 1] and [bottom, top] -> [-1, 1]
        mat.m[0] = 2.0f / width;
        mat.m[4] = 2.0f / height;
        
        // Translation: center the viewport
        mat.m[2] = -(right + left) / width;
        mat.m[5] = -(top + bottom) / height;
        
        // Z-component stays at 1.0 (homogeneous coordinate)
        mat.m[8] = 1.0f;
        
        return mat;
    }

    // Matrix inversion (for 2D affine transforms)
    Matrix3 Inverse() const {
        Matrix3 inv;
        
        // Calculate determinant
        float det = m[0] * (m[4] * m[8] - m[7] * m[5])
                  - m[1] * (m[3] * m[8] - m[5] * m[6])
                  + m[2] * (m[3] * m[7] - m[4] * m[6]);
        
        if (std::abs(det) < 1e-6f) {
            // Matrix is singular, return identity
            return Identity();
        }
        
        float invDet = 1.0f / det;
        
        inv.m[0] = (m[4] * m[8] - m[7] * m[5]) * invDet;
        inv.m[1] = (m[2] * m[7] - m[1] * m[8]) * invDet;
        inv.m[2] = (m[1] * m[5] - m[2] * m[4]) * invDet;
        inv.m[3] = (m[5] * m[6] - m[3] * m[8]) * invDet;
        inv.m[4] = (m[0] * m[8] - m[2] * m[6]) * invDet;
        inv.m[5] = (m[3] * m[2] - m[0] * m[5]) * invDet;
        inv.m[6] = (m[3] * m[7] - m[6] * m[4]) * invDet;
        inv.m[7] = (m[6] * m[1] - m[0] * m[7]) * invDet;
        inv.m[8] = (m[0] * m[4] - m[3] * m[1]) * invDet;
        
        return inv;
    }
};

} // namespace SAGE
