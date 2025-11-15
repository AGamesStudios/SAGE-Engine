// Legacy Math.h kept for backward compatibility; prefer Constants.h
#pragma once

#include "Vector2.h"
#include "Math/Constants.h"

namespace SAGE {
namespace Math {

// Deprecated: use Math::Constants::PI instead
constexpr float PI = Constants::PI;

// Conversion helpers removed (use Math::ToRadians/ToDegrees from Constants.h)

} // namespace Math
} // namespace SAGE
