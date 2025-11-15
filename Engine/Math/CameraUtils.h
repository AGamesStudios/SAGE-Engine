#pragma once

#include "Matrix4.h"
#include "Vector3.h"
#include <cmath>

namespace SAGE::CameraUtils {

inline Matrix4 MakePerspective(float fovYRadians, float aspect, float zNear, float zFar) {
    // Column-major matrix matching typical graphics conventions
    const float f = 1.0f / std::tan(fovYRadians / 2.0f);
    Matrix4 m; // identity
    float* d = m.Data();
    d[0] = f / aspect;
    d[5] = f;
    d[10] = (zFar + zNear) / (zNear - zFar);
    d[11] = -1.0f;
    d[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    d[15] = 0.0f;
    return m;
}

inline Matrix4 MakeLookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    const Vector3 f = Vector3::Normalize(target - eye);      // forward
    const Vector3 s = Vector3::Normalize(Vector3::Cross(f, up)); // right
    const Vector3 u = Vector3::Cross(s, f);                  // corrected up

    Matrix4 m; // identity
    float* d = m.Data();
    d[0] = s.x; d[4] = s.y; d[8]  = s.z;  d[12] = -Vector3::Dot(s, eye);
    d[1] = u.x; d[5] = u.y; d[9]  = u.z;  d[13] = -Vector3::Dot(u, eye);
    d[2] = -f.x; d[6] = -f.y; d[10] = -f.z; d[14] = Vector3::Dot(f, eye);
    d[3] = 0.0f; d[7] = 0.0f; d[11] = 0.0f; d[15] = 1.0f;
    return m;
}

} // namespace SAGE::CameraUtils
