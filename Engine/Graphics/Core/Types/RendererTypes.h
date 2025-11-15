#pragma once

#include "Color.h"
#include "MathTypes.h"
#include "Math/Vector2.h"
#include "Math/Constants.h"
#include "Memory/Ref.h"

#include <string>
#include <algorithm>
#include <cmath>

// Windows headers may define min/max macros that break std::min/std::max usage.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace SAGE {

    class Texture;
    class Font;

    // OLD Camera2D struct - DEPRECATED - Use Graphics/Core/Camera2D.h instead
    /*
    struct Camera2D {
        Vector2 position{ 0.0f, 0.0f };
        float zoom = 1.0f;
        float rotation = 0.0f;  // Rotation in radians
        Vector2 rotationOrigin{ 0.0f, 0.0f };  // Pivot point for rotation (camera space)
        
        // Viewport dimensions for aspect ratio and bounds calculations
        float viewportWidth = 1280.0f;
        float viewportHeight = 720.0f;
        
        /// Get viewport aspect ratio (width/height)
        float GetAspectRatio() const { 
            return viewportHeight > 0.0f ? viewportWidth / viewportHeight : 1.0f; 
        }
        
        /// Calculate visible world bounds in screen coordinates
        Rect GetVisibleBounds() const {
            float safeZoom = std::max(0.001f, zoom);  // Prevent division by zero
            float halfWidth = (viewportWidth / safeZoom) * 0.5f;
            float halfHeight = (viewportHeight / safeZoom) * 0.5f;
            return Rect{
                position.x - halfWidth,
                position.y - halfHeight,
                viewportWidth / safeZoom,
                viewportHeight / safeZoom
            };
        }
        
        /// Set zoom with validation (minimum 0.001f)
        void SetZoom(float z) {
            zoom = std::max(0.001f, z);
        }
        
        /// Get current zoom value
        float GetZoom() const {
            return zoom;
        }
        
        /// Set rotation with normalization to [0, 2PI] range
        void SetRotation(float radians) {
            constexpr float twoPi = 6.28318530718f; // 2 * PI
            rotation = std::fmod(radians, twoPi);
            if (rotation < 0.0f) {
                rotation += twoPi;
            }
        }
        
        /// Rotate by delta and normalize
        void Rotate(float deltaRadians) {
            SetRotation(rotation + deltaRadians);
        }
        
        /// Get current rotation in radians
        float GetRotation() const {
            return rotation;
        }
    };
    */

    enum class QuadEffectType {
        None = 0,
        Pulse,
        Tint,
        Custom
    };

    struct QuadEffect {
        QuadEffectType Type = QuadEffectType::None;
        Color Data0;  // multipurpose: Tint RGBA, pulse params, etc.
        Color Data1;  // reserved for advanced effects
        float pulseAmplitude = 0.0f; // legacy: kept for backward compat
        float pulseFrequency = 0.0f; // legacy
    };

    enum class DepthFunction {
        Less,
        LessEqual,
        Equal,
        Greater,
        Always
    };

    struct DepthSettings {
        bool testEnabled = false;
        bool writeEnabled = false;
        DepthFunction function = DepthFunction::LessEqual;
        float biasConstant = 0.0f;
        float biasSlope = 0.0f;
    };

    struct QuadDesc {
        Float2 position{ 0.0f, 0.0f };
        Float2 size{ 1.0f, 1.0f };
        Color color = Color::White();
        Ref<Texture> texture;
        Float2 uvMin{ 0.0f, 0.0f };
        Float2 uvMax{ 1.0f, 1.0f };
        float rotation = 0.0f;  // Rotation in degrees
        bool screenSpace = false;
        // Source classification for stats (avoids heuristic). Extend as needed.
        enum class QuadSource : uint8_t {
            Generic = 0,  // default unclassified quad
            Tile = 1,     // tilemap tile quad
            UI = 2,       // UI element quad (screen-space widgets)
            Debug = 3,    // debug visualization quad (bounds, overlays)
            Glyph = 4     // individual text glyph quad (if needed for fine-grained profiling)
        };
        QuadSource source = QuadSource::Generic;
    };

    struct TextDesc {
        std::string text;
        Float2 position{ 0.0f, 0.0f };
        Ref<Font> font;
        float scale = 1.0f;
        Color color = Color::White();
        bool screenSpace = false;
    };

struct PostFXSettings {
    bool enabled = false;
    Color tint = Color::Transparent();
    float intensity = 0.2f;
    float bloomThreshold = 0.7f;
    float bloomStrength = 0.5f;
    int blurIterations = 2;
    float gamma = 2.2f;
    float exposure = 1.0f;
    float pulseSpeed = 0.0f;
};

} // namespace SAGE
