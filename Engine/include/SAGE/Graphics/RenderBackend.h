#pragma once

#include "SAGE/Math/Color.h"
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Matrix3.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace SAGE {

class Texture;
class Shader;
class Sprite;
class Camera2D;

struct RenderStats {
    uint32_t drawCalls = 0;
    uint32_t vertices = 0;
    uint32_t triangles = 0;

    void Reset() {
        drawCalls = 0;
        vertices = 0;
        triangles = 0;
    }
};

enum class RenderMode {
    Solid = 0,
    Wireframe = 1
};

enum class RenderBackendType {
    OpenGL = 0,
    Vulkan = 1,
};

struct RendererConfig {
    RenderBackendType backend = RenderBackendType::OpenGL;
    std::filesystem::path configFile = std::filesystem::path("config") / "rendering.json";
    bool enableRuntimeOverrides = true;
    bool autoConfigurePixelProjection = true;
    bool pixelOriginTopLeft = true;
};

const char* ToString(RenderBackendType type);
RenderBackendType RenderBackendTypeFromString(const std::string& name, RenderBackendType fallback = RenderBackendType::OpenGL);

class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual void Initialize(const RendererConfig& config) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void Clear(const Color& color) = 0;
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void SetRenderMode(RenderMode mode) = 0;
    virtual RenderMode GetRenderMode() const = 0;

    virtual void EnableBlending(bool enabled) = 0;
    virtual void SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor) = 0;

    // Scissor Stack
    virtual void PushScissor(int x, int y, int width, int height) = 0;
    virtual void PopScissor() = 0;
    // Deprecated: Use Push/Pop instead
    virtual void SetScissor(int x, int y, int width, int height) = 0;
    virtual void DisableScissor() = 0;

    virtual void DrawQuad(const Vector2& position, const Vector2& size, const Color& color) = 0;
    virtual void DrawQuad(const Vector2& position, const Vector2& size, Texture* texture) = 0;
    virtual void DrawQuadTinted(const Vector2& position, const Vector2& size, const Color& color, Texture* texture) = 0;
    virtual void DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Shader* shader) = 0;
    virtual void DrawQuadGradient(const Vector2& position, const Vector2& size, const Color& c1, const Color& c2, const Color& c3, const Color& c4) = 0;
    virtual void DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness) = 0;

    virtual void DrawSprite(const Sprite& sprite) = 0;
    virtual void DrawSprite(const Sprite& sprite, const Camera2D& camera) = 0;

    virtual void BeginSpriteBatch(const Camera2D* camera) = 0;
    virtual void SubmitSprite(const Sprite& sprite) = 0;
    virtual void FlushSpriteBatch() = 0;

    virtual void DrawParticle(const Vector2& position, float size, const Color& color, float rotation) = 0;

    virtual void SetProjectionMatrix(const Matrix3& projection) = 0;
    virtual void SetViewMatrix(const Matrix3& view) = 0;
    virtual void SetCamera(const Camera2D& camera) = 0;

    virtual const Matrix3& GetProjectionMatrix() const = 0;
    virtual const Matrix3& GetViewMatrix() const = 0;
    virtual Matrix3 GetViewProjectionMatrix() const = 0;

    virtual const RenderStats& GetStats() const = 0;
    virtual void ResetStats() = 0;

    virtual void DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) = 0;
    virtual void DrawCircle(const Vector2& center, float radius, const Color& color) = 0;
};

} // namespace SAGE
