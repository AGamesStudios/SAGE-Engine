#pragma once

#include "Vector2.h"

namespace SAGE {
    namespace Math {
        
        constexpr float PI = 3.14159265358979323846f;
        
        inline float ToRadians(float degrees) {
            return degrees * PI / 180.0f;
        }
        
        inline float ToDegrees(float radians) {
            return radians * 180.0f / PI;
        }
        
    }
}
