#pragma once

#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Matrix3.h"
#include "SAGE/Math/Rect.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace SAGE {

// Forward declarations
class Shader;
class Texture;
class Sprite;
class Camera2D;

struct Vertex {
    Vector2 position;
    Vector2 texCoord;
    Color color;
};

class Renderer {
public:
    static void Init(const RendererConfig& config = RendererConfig{});
    static void Shutdown();
    static RendererConfig GetConfig();
    using BackendFactory = std::function<std::unique_ptr<RenderBackend>(RenderBackendType)>;
    static void SetBackendFactory(BackendFactory factory);

    static void BeginFrame();
    static void EndFrame();

    static void Clear(const Color& color = Color::Black());
    static void SetViewport(int x, int y, int width, int height);
    static void SetRenderMode(RenderMode mode);
    static RenderMode GetRenderMode();
    static void EnableBlending(bool enabled);
    static void SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor);
    static void ConfigureAutoProjection(bool enabled, bool originTopLeft = true);
    
    static void SetScissor(int x, int y, int width, int height);
    static void DisableScissor();
    static void PushScissor(int x, int y, int width, int height);
    static void PopScissor();

    // Immediate mode drawing (for prototyping)
    static void DrawQuad(const Vector2& position, const Vector2& size, const Color& color);
    static void DrawQuad(const Vector2& position, const Vector2& size, Texture* texture);
    static void DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Texture* texture);
    static void DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Shader* shader);
    
    // Shape rendering with outlines
    // Position is the center of the rectangle
    static void DrawRect(const Vector2& position, const Vector2& size, const Color& fillColor, float outlineThickness = 0.0f, const Color& outlineColor = Color::Black());
    // Rect struct (x,y is top-left)
    static void DrawRect(const Rect& rect, const Color& fillColor, float outlineThickness = 0.0f, const Color& outlineColor = Color::Black());
    
    static void DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness = 1.0f);
    static void DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color);
    static void DrawCircle(const Vector2& center, float radius, const Color& color);
    static void DrawCircle(const Vector2& center, float radius, const Color& fillColor, float outlineThickness, const Color& outlineColor = Color::Black());
    
    // Sprite rendering
    static void DrawSprite(const Sprite& sprite);
    static void DrawSprite(const Sprite& sprite, const Camera2D& camera);

    // Batched sprite rendering
    static void BeginSpriteBatch(const Camera2D* camera = nullptr);
    static void SubmitSprite(const Sprite& sprite);
    static void FlushSpriteBatch();
    
    // Particle rendering
    static void DrawParticle(const Vector2& position, float size, const Color& color, float rotation = 0.0f);
    
    // Text rendering
    static void DrawText(const std::string& text, const Vector2& position, const Color& color = Color::White(), std::shared_ptr<Font> font = nullptr);
    static void DrawTextAligned(const std::string& text, const Vector2& position, TextAlign align, const Color& color = Color::White(), std::shared_ptr<Font> font = nullptr);

    // Camera/projection
    static void SetProjectionMatrix(const Matrix3& projection);
    static void SetViewMatrix(const Matrix3& view);
    static void SetCamera(const Camera2D& camera);
    static const Matrix3& GetProjectionMatrix();
    static const Matrix3& GetViewMatrix();
    static Matrix3 GetViewProjectionMatrix();

    // Stats
    static const RenderStats& GetStats();
    static void ResetStats();

    static RenderBackend* GetBackend();

private:
    Renderer() = delete;
};

} // namespace SAGE
