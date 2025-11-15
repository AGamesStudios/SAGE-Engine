#pragma once

#include <cstdint>
#include <string>

// Include actual types from engine to avoid duplication
#include "../../Math/Vector2.h"
#include "../../Math/Vector3.h"
#include "../../Graphics/Core/Types/Color.h"
#include "../../Graphics/Core/Types/MathTypes.h"

namespace SAGE {

// Forward declarations
class IEngine;
class IScene;
class IEntity;
class IRenderer;
class IResourceManager;

// Handle types (opaque handles for type safety)
using EntityHandle = uint64_t;
using TextureHandle = uint64_t;
using ShaderHandle = uint64_t;
using SceneHandle = uint64_t;
using ComponentHandle = uint64_t;

constexpr EntityHandle NullEntity = 0;
constexpr TextureHandle NullTexture = 0;
constexpr ShaderHandle NullShader = 0;
constexpr SceneHandle NullScene = 0;

// Use engine's math types directly (avoid duplication)
// Vector2 is already in ::SAGE namespace from Math/Vector2.h
// Color is already in ::SAGE namespace from Graphics/Core/Types/Color.h

// Engine configuration
struct EngineConfig {
    std::string windowTitle = "SAGE Engine";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool vsync = true;
    bool fullscreen = false;
    bool resizable = true;
    std::string logDirectory = "logs/";
    std::string assetsDirectory = "assets/";
};

// Component data structures (POD for serialization)
struct TransformData {
    Vector3 position;
    Vector3 rotation;  // Euler angles in degrees
    Vector3 scale;
    
    TransformData() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}
};

struct SpriteData {
    TextureHandle texture = NullTexture;
    Color color;
    Vector2 size;
    Vector2 uvOffset;
    Vector2 uvScale;
    int layer = 0;
    bool flipX = false;
    bool flipY = false;
    
    SpriteData() : color(Color::White()), size(1, 1), uvOffset(0, 0), uvScale(1, 1) {}
};

struct CameraData {
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isOrthographic = true;
    float orthographicSize = 10.0f;
    
    CameraData() = default;
};

} // namespace SAGE
