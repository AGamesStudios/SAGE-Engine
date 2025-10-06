#pragma once

#include "Color.h"
#include "MathTypes.h"
#include "../Math/Vector2.h"
#include "../Memory/Ref.h"

#include <string>

namespace SAGE {

    class Texture;
    class Font;

    struct Camera2D {
        Vector2 position{ 0.0f, 0.0f };
        float zoom = 1.0f;
    };

    struct QuadEffect {
        float pulseAmplitude = 0.0f;
        float pulseFrequency = 0.0f;
    };

    struct QuadDesc {
        Float2 position{ 0.0f, 0.0f };
        Float2 size{ 1.0f, 1.0f };
        Color color = Color::White();
        Ref<Texture> texture;
        Float2 uvMin{ 0.0f, 0.0f };
        Float2 uvMax{ 1.0f, 1.0f };
        bool screenSpace = false;
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
        float pulseSpeed = 0.0f;
    };

} // namespace SAGE
