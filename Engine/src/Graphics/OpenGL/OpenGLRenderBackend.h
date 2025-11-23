#pragma once

#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Shader.h"
#include "SAGE/Graphics/SpriteRenderer.h"

#include <memory>
#include <stack>

namespace SAGE {

struct ScissorRect {
    int x, y, width, height;
};

class OpenGLRenderBackend final : public RenderBackend {
public:
    void Initialize(const RendererConfig& config) override;
    void Shutdown() override;

    void BeginFrame() override;
    void EndFrame() override;

    void Clear(const Color& color) override;
    void SetViewport(int x, int y, int width, int height) override;
    void SetRenderMode(RenderMode mode) override;
    RenderMode GetRenderMode() const override;

    void EnableBlending(bool enabled) override;
    void SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor) override;

    void SetScissor(int x, int y, int width, int height) override;
    void DisableScissor() override;
    void PushScissor(int x, int y, int width, int height) override;
    void PopScissor() override;

    void DrawQuad(const Vector2& position, const Vector2& size, const Color& color) override;
    void DrawQuad(const Vector2& position, const Vector2& size, Texture* texture) override;
    void DrawQuadTinted(const Vector2& position, const Vector2& size, const Color& color, Texture* texture) override;
    void DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Shader* shader) override;
    void DrawQuadGradient(const Vector2& position, const Vector2& size, const Color& c1, const Color& c2, const Color& c3, const Color& c4) override;
    void DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness) override;

    void DrawSprite(const Sprite& sprite) override;
    void DrawSprite(const Sprite& sprite, const Camera2D& camera) override;

    void BeginSpriteBatch(const Camera2D* camera) override;
    void SubmitSprite(const Sprite& sprite) override;
    void FlushSpriteBatch() override;

    void DrawParticle(const Vector2& position, float size, const Color& color, float rotation) override;

    void SetProjectionMatrix(const Matrix3& projection) override;
    void SetViewMatrix(const Matrix3& view) override;
    void SetCamera(const Camera2D& camera) override;

    const Matrix3& GetProjectionMatrix() const override;
    const Matrix3& GetViewMatrix() const override;
    Matrix3 GetViewProjectionMatrix() const override;

    const RenderStats& GetStats() const override;
    void ResetStats() override;

    void DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) override;
    void DrawCircle(const Vector2& center, float radius, const Color& color) override;

private:
    void CreateQuadBuffers();
    void CreateDynamicBuffers();
    void UpdateScissor();

    RendererConfig m_Config{};
    RenderStats m_Stats{};

    std::shared_ptr<Shader> m_DefaultShader;
    Matrix3 m_Projection = Matrix3::Identity();
    Matrix3 m_View = Matrix3::Identity();
    Matrix3 m_ViewProjection = Matrix3::Identity();

    uint32_t m_QuadVAO = 0;
    uint32_t m_QuadVBO = 0;
    uint32_t m_QuadEBO = 0;

    uint32_t m_DynamicVAO = 0;
    uint32_t m_DynamicVBO = 0;

    RenderMode m_RenderMode = RenderMode::Solid;
    bool m_BlendingEnabled = true;
    uint32_t m_BlendSrc = 0;
    uint32_t m_BlendDst = 0;

    bool m_Initialized = false;
    SpriteRenderer m_SpriteRenderer;

    std::stack<ScissorRect> m_ScissorStack;
};

} // namespace SAGE
