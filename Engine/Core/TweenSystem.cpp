#include "TweenSystem.h"
#include <cmath>

namespace SAGE {

float Easing::Apply(EasingType type, float t) {
    switch (type) {
        case EasingType::Linear: return Linear(t);
        
        case EasingType::QuadIn: return QuadIn(t);
        case EasingType::QuadOut: return QuadOut(t);
        case EasingType::QuadInOut: return QuadInOut(t);
        
        case EasingType::CubicIn: return CubicIn(t);
        case EasingType::CubicOut: return CubicOut(t);
        case EasingType::CubicInOut: return CubicInOut(t);
        
        case EasingType::SineIn: return SineIn(t);
        case EasingType::SineOut: return SineOut(t);
        case EasingType::SineInOut: return SineInOut(t);
        
        case EasingType::ExpoIn: return ExpoIn(t);
        case EasingType::ExpoOut: return ExpoOut(t);
        
        case EasingType::ElasticOut: return ElasticOut(t);
        case EasingType::BounceOut: return BounceOut(t);
        case EasingType::BackOut: return BackOut(t);
        
        default: return Linear(t);
    }
}

float Easing::ElasticOut(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    
    float p = 0.3f;
    return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * 3.14159265359f) / p) + 1.0f;
}

float Easing::BounceOut(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        float f = t - 1.5f / 2.75f;
        return 7.5625f * f * f + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        float f = t - 2.25f / 2.75f;
        return 7.5625f * f * f + 0.9375f;
    } else {
        float f = t - 2.625f / 2.75f;
        return 7.5625f * f * f + 0.984375f;
    }
}

float Easing::BackOut(float t) {
    float s = 1.70158f;
    float f = t - 1.0f;
    return f * f * ((s + 1.0f) * f + s) + 1.0f;
}

} // namespace SAGE
