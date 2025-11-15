#include "Matrix4.h"

#include <cmath>
#include <limits>

namespace SAGE {

namespace {
constexpr int kMatrixSize = 4;
constexpr int kElementCount = kMatrixSize * kMatrixSize;
}

Matrix4::Matrix4() : m_Data{ 1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f } {
}

Matrix4 Matrix4::Identity() {
    return Matrix4();
}

Matrix4 Matrix4::Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Matrix4 result;

    const float rl = right - left;
    const float tb = top - bottom;
    const float fn = farPlane - nearPlane;

    // Check for degenerate parameters that would cause division by zero
    constexpr float kEpsilon = 1e-6f;
    if (std::abs(rl) < kEpsilon || std::abs(tb) < kEpsilon || std::abs(fn) < kEpsilon) {
        // Log error and return identity matrix as safe fallback
        // Note: Using printf for compatibility if logger not available in Math module
        std::fprintf(stderr, "Matrix4::Orthographic - Degenerate parameters: rl=%.6f, tb=%.6f, fn=%.6f\n", rl, tb, fn);
        return Matrix4::Identity();
    }

    result.m_Data[0] = 2.0f / rl;
    result.m_Data[5] = 2.0f / tb;
    result.m_Data[10] = -2.0f / fn;
    result.m_Data[12] = -(right + left) / rl;
    result.m_Data[13] = -(top + bottom) / tb;
    result.m_Data[14] = -(farPlane + nearPlane) / fn;
    return result;
}

Matrix4 Matrix4::Translate(float x, float y, float z) {
    Matrix4 result = Matrix4::Identity();
    result.m_Data[12] = x;
    result.m_Data[13] = y;
    result.m_Data[14] = z;
    return result;
}

Matrix4 Matrix4::Scale(float x, float y, float z) {
    Matrix4 result;
    result.m_Data[0] = x;
    result.m_Data[5] = y;
    result.m_Data[10] = z;
    return result;
}

Matrix4 Matrix4::RotateZ(float radians) {
    Matrix4 result;
    float c = std::cos(radians);
    float s = std::sin(radians);
    result.m_Data[0] = c;
    result.m_Data[1] = s;
    result.m_Data[4] = -s;
    result.m_Data[5] = c;
    return result;
}

Matrix4 Matrix4::Inverse(const Matrix4& mat) {
    Matrix4 result;
    const float* m = mat.m_Data.data();
    double inv[16];

    // Calculate 4x4 matrix inverse using cofactor expansion in double precision
    inv[0] = static_cast<double>(m[5])  * m[10] * m[15] - static_cast<double>(m[5])  * m[11] * m[14] - static_cast<double>(m[9])  * m[6]  * m[15] +
             static_cast<double>(m[9])  * m[7]  * m[14] + static_cast<double>(m[13]) * m[6]  * m[11] - static_cast<double>(m[13]) * m[7]  * m[10];
    inv[4] = -static_cast<double>(m[4])  * m[10] * m[15] + static_cast<double>(m[4])  * m[11] * m[14] + static_cast<double>(m[8])  * m[6]  * m[15] -
              static_cast<double>(m[8])  * m[7]  * m[14] - static_cast<double>(m[12]) * m[6]  * m[11] + static_cast<double>(m[12]) * m[7]  * m[10];
    inv[8] = static_cast<double>(m[4])  * m[9] * m[15] - static_cast<double>(m[4])  * m[11] * m[13] - static_cast<double>(m[8])  * m[5] * m[15] +
             static_cast<double>(m[8])  * m[7] * m[13] + static_cast<double>(m[12]) * m[5] * m[11] - static_cast<double>(m[12]) * m[7] * m[9];
    inv[12] = -static_cast<double>(m[4])  * m[9] * m[14] + static_cast<double>(m[4])  * m[10] * m[13] + static_cast<double>(m[8])  * m[5] * m[14] -
               static_cast<double>(m[8])  * m[6] * m[13] - static_cast<double>(m[12]) * m[5] * m[10] + static_cast<double>(m[12]) * m[6] * m[9];
    inv[1] = -static_cast<double>(m[1])  * m[10] * m[15] + static_cast<double>(m[1])  * m[11] * m[14] + static_cast<double>(m[9])  * m[2] * m[15] -
              static_cast<double>(m[9])  * m[3] * m[14] - static_cast<double>(m[13]) * m[2] * m[11] + static_cast<double>(m[13]) * m[3] * m[10];
    inv[5] = static_cast<double>(m[0])  * m[10] * m[15] - static_cast<double>(m[0])  * m[11] * m[14] - static_cast<double>(m[8])  * m[2] * m[15] +
             static_cast<double>(m[8])  * m[3] * m[14] + static_cast<double>(m[12]) * m[2] * m[11] - static_cast<double>(m[12]) * m[3] * m[10];
    inv[9] = -static_cast<double>(m[0])  * m[9] * m[15] + static_cast<double>(m[0])  * m[11] * m[13] + static_cast<double>(m[8])  * m[1] * m[15] -
              static_cast<double>(m[8])  * m[3] * m[13] - static_cast<double>(m[12]) * m[1] * m[11] + static_cast<double>(m[12]) * m[3] * m[9];
    inv[13] = static_cast<double>(m[0])  * m[9] * m[14] - static_cast<double>(m[0])  * m[10] * m[13] - static_cast<double>(m[8])  * m[1] * m[14] +
              static_cast<double>(m[8])  * m[2] * m[13] + static_cast<double>(m[12]) * m[1] * m[10] - static_cast<double>(m[12]) * m[2] * m[9];
    inv[2] = static_cast<double>(m[1])  * m[6] * m[15] - static_cast<double>(m[1])  * m[7] * m[14] - static_cast<double>(m[5])  * m[2] * m[15] +
             static_cast<double>(m[5])  * m[3] * m[14] + static_cast<double>(m[13]) * m[2] * m[7] - static_cast<double>(m[13]) * m[3] * m[6];
    inv[6] = -static_cast<double>(m[0])  * m[6] * m[15] + static_cast<double>(m[0])  * m[7] * m[14] + static_cast<double>(m[4])  * m[2] * m[15] -
              static_cast<double>(m[4])  * m[3] * m[14] - static_cast<double>(m[12]) * m[2] * m[7] + static_cast<double>(m[12]) * m[3] * m[6];
    inv[10] = static_cast<double>(m[0])  * m[5] * m[15] - static_cast<double>(m[0])  * m[7] * m[13] - static_cast<double>(m[4])  * m[1] * m[15] +
              static_cast<double>(m[4])  * m[3] * m[13] + static_cast<double>(m[12]) * m[1] * m[7] - static_cast<double>(m[12]) * m[3] * m[5];
    inv[14] = -static_cast<double>(m[0])  * m[5] * m[14] + static_cast<double>(m[0])  * m[6] * m[13] + static_cast<double>(m[4])  * m[1] * m[14] -
               static_cast<double>(m[4])  * m[2] * m[13] - static_cast<double>(m[12]) * m[1] * m[6] + static_cast<double>(m[12]) * m[2] * m[5];
    inv[3] = -static_cast<double>(m[1]) * m[6] * m[11] + static_cast<double>(m[1]) * m[7] * m[10] + static_cast<double>(m[5]) * m[2] * m[11] -
              static_cast<double>(m[5]) * m[3] * m[10] - static_cast<double>(m[9]) * m[2] * m[7] + static_cast<double>(m[9]) * m[3] * m[6];
    inv[7] = static_cast<double>(m[0]) * m[6] * m[11] - static_cast<double>(m[0]) * m[7] * m[10] - static_cast<double>(m[4]) * m[2] * m[11] +
             static_cast<double>(m[4]) * m[3] * m[10] + static_cast<double>(m[8]) * m[2] * m[7] - static_cast<double>(m[8]) * m[3] * m[6];
    inv[11] = -static_cast<double>(m[0]) * m[5] * m[11] + static_cast<double>(m[0]) * m[7] * m[9] + static_cast<double>(m[4]) * m[1] * m[11] -
               static_cast<double>(m[4]) * m[3] * m[9] - static_cast<double>(m[8]) * m[1] * m[7] + static_cast<double>(m[8]) * m[3] * m[5];
    inv[15] = static_cast<double>(m[0]) * m[5] * m[10] - static_cast<double>(m[0]) * m[6] * m[9] - static_cast<double>(m[4]) * m[1] * m[10] +
              static_cast<double>(m[4]) * m[2] * m[9] + static_cast<double>(m[8]) * m[1] * m[6] - static_cast<double>(m[8]) * m[2] * m[5];

    double det = static_cast<double>(m[0]) * inv[0] + static_cast<double>(m[1]) * inv[4] +
                 static_cast<double>(m[2]) * inv[8] + static_cast<double>(m[3]) * inv[12];

    constexpr double kSingularThreshold = 1e-7; // Adjusted threshold for float precision
    if (std::abs(det) < kSingularThreshold) {
        // Singular matrix, log warning and return identity as fallback
        std::fprintf(stderr, "Matrix4::Inverse - Singular matrix detected (det=%.10e), returning identity\n", det);
        return Matrix4::Identity();
    }

    const double detInv = 1.0 / det;
    for (int i = 0; i < 16; i++) {
        result.m_Data[i] = static_cast<float>(inv[i] * detInv);
    }

    return result;
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const {
    Matrix4 result = Matrix4::Identity();
    result.m_Data.fill(0.0f);

    for (int col = 0; col < kMatrixSize; ++col) {
        for (int row = 0; row < kMatrixSize; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < kMatrixSize; ++k) {
                sum += m_Data[k * kMatrixSize + row] * rhs.m_Data[col * kMatrixSize + k];
            }
            result.m_Data[col * kMatrixSize + row] = sum;
        }
    }

    return result;
}

Matrix4& Matrix4::operator*=(const Matrix4& rhs) {
    *this = *this * rhs;
    return *this;
}

// Perspective projection matrix (right-handed, maps to [-1,1] NDC)
Matrix4 Matrix4::Perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
    Matrix4 result;
    
    constexpr float kEpsilon = 1e-6f;
    if (std::abs(aspectRatio) < kEpsilon || std::abs(farPlane - nearPlane) < kEpsilon) {
        std::fprintf(stderr, "Matrix4::Perspective - Degenerate parameters: aspect=%.6f, near-far=%.6f\n",
                     aspectRatio, farPlane - nearPlane);
        return Matrix4::Identity();
    }
    
    const float tanHalfFov = std::tan(fovRadians * 0.5f);
    const float fn = farPlane - nearPlane;
    
    result.m_Data[0] = 1.0f / (aspectRatio * tanHalfFov);
    result.m_Data[5] = 1.0f / tanHalfFov;
    result.m_Data[10] = -(farPlane + nearPlane) / fn;
    result.m_Data[11] = -1.0f;
    result.m_Data[14] = -(2.0f * farPlane * nearPlane) / fn;
    result.m_Data[15] = 0.0f;
    
    return result;
}

// LookAt view matrix (right-handed coordinate system)
Matrix4 Matrix4::LookAt(float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ) {
    // Forward vector (eye to center)
    float fX = centerX - eyeX;
    float fY = centerY - eyeY;
    float fZ = centerZ - eyeZ;
    
    const float fLen = std::sqrt(fX*fX + fY*fY + fZ*fZ);
    constexpr float kEpsilon = 1e-6f;
    if (fLen < kEpsilon) {
        std::fprintf(stderr, "Matrix4::LookAt - Eye and center are too close\n");
        return Matrix4::Identity();
    }
    
    fX /= fLen;
    fY /= fLen;
    fZ /= fLen;
    
    // Right vector (forward × up)
    float rX = fY * upZ - fZ * upY;
    float rY = fZ * upX - fX * upZ;
    float rZ = fX * upY - fY * upX;
    
    const float rLen = std::sqrt(rX*rX + rY*rY + rZ*rZ);
    if (rLen < kEpsilon) {
        std::fprintf(stderr, "Matrix4::LookAt - Forward and up vectors are parallel\n");
        return Matrix4::Identity();
    }
    
    rX /= rLen;
    rY /= rLen;
    rZ /= rLen;
    
    // Up vector (right × forward)
    const float uX = rY * fZ - rZ * fY;
    const float uY = rZ * fX - rX * fZ;
    const float uZ = rX * fY - rY * fX;
    
    Matrix4 result;
    result.m_Data[0] = rX;
    result.m_Data[1] = uX;
    result.m_Data[2] = -fX;
    result.m_Data[4] = rY;
    result.m_Data[5] = uY;
    result.m_Data[6] = -fY;
    result.m_Data[8] = rZ;
    result.m_Data[9] = uZ;
    result.m_Data[10] = -fZ;
    result.m_Data[12] = -(rX * eyeX + rY * eyeY + rZ * eyeZ);
    result.m_Data[13] = -(uX * eyeX + uY * eyeY + uZ * eyeZ);
    result.m_Data[14] = fX * eyeX + fY * eyeY + fZ * eyeZ;
    
    return result;
}

// Rotation around X-axis
Matrix4 Matrix4::RotateX(float radians) {
    Matrix4 result = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    
    result.m_Data[5] = c;
    result.m_Data[6] = s;
    result.m_Data[9] = -s;
    result.m_Data[10] = c;
    
    return result;
}

// Rotation around Y-axis
Matrix4 Matrix4::RotateY(float radians) {
    Matrix4 result = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    
    result.m_Data[0] = c;
    result.m_Data[2] = -s;
    result.m_Data[8] = s;
    result.m_Data[10] = c;
    
    return result;
}

const float* Matrix4::Data() const {
    return m_Data.data();
}

float* Matrix4::Data() {
    return m_Data.data();
}

} // namespace SAGE
