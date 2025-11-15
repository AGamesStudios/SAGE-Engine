#pragma once

#include <array>

namespace SAGE {

class Matrix4 {
public:
    Matrix4();

    static Matrix4 Identity();
    static Matrix4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Matrix4 Perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane);
    static Matrix4 LookAt(float eyeX, float eyeY, float eyeZ, 
                          float centerX, float centerY, float centerZ, 
                          float upX, float upY, float upZ);
    static Matrix4 Translate(float x, float y, float z);
    static Matrix4 Scale(float x, float y, float z);
    static Matrix4 RotateX(float radians);
    static Matrix4 RotateY(float radians);
    static Matrix4 RotateZ(float radians);
    static Matrix4 Inverse(const Matrix4& mat);

    Matrix4 operator*(const Matrix4& rhs) const;
    Matrix4& operator*=(const Matrix4& rhs);

    const float* Data() const;
    float* Data();

private:
    std::array<float, 16> m_Data;
};

} // namespace SAGE
