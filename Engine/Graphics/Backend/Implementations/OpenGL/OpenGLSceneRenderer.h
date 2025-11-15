#pragma once

#include "Graphics/Backend/Interfaces/ISceneRenderer.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include "Memory/Ref.h"

namespace SAGE {

class IRenderBackend;

namespace Graphics {

/// OpenGL-based scene renderer
/// Translates high-level scene operations (quads, text, effects) to low-level backend calls
class OpenGLSceneRenderer final : public ISceneRenderer {
public:
    explicit OpenGLSceneRenderer(IRenderBackend* backend);
    ~OpenGLSceneRenderer() override;

    // Lifecycle
    void Init() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const override;

    // Per-frame update
    void Update(float deltaTime) override;

    // Camera management
    void SetCamera(const Camera2D& camera) override;
    [[nodiscard]] const Camera2D& GetCamera() const override;
    void ResetCamera() override;

    // Screen shake
    void PushScreenShake(float amplitude, float frequency, float duration) override;

#ifdef SAGE_ENGINE_TESTING
    [[nodiscard]] Vector2 GetCameraShakeOffsetForTesting() const override;
    [[nodiscard]] float GetShakeStrengthForTesting() const override;
    [[nodiscard]] float GetShakeDurationForTesting() const override;
    [[nodiscard]] float GetShakeTimerForTesting() const override;
#endif

    // Frame lifecycle
    void BeginScene() override;
    [[nodiscard]] bool EndScene() override;
    // Explicit shake application (updates internal offset based on timer) so passes can call it before projection
    void ApplyShake();

    // Layer management
    void SetLayer(float layer) override;
    void PushLayer(float layer) override;
    void PopLayer() override;

    // Material
    MaterialId SetMaterial(MaterialId materialId) override;

    // Blend mode
    void PushBlendMode(BlendMode mode) override;
    void PopBlendMode() override;
    void SetBlendMode(BlendMode mode) override;
    [[nodiscard]] BlendMode GetBlendMode() const override;

    // Depth state
    void PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                        float biasConstant, float biasSlope) override;
    void PopDepthState() override;
    void SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                       float biasConstant, float biasSlope) override;
    [[nodiscard]] DepthSettings GetDepthState() const override;

    // Effects
    void PushEffect(const QuadEffect& effect) override;
    void PopEffect() override;

    // Post-processing
    void ConfigurePostFX(const PostFXSettings& settings) override;
    [[nodiscard]] const PostFXSettings& GetPostFXSettings() const override;
    void EnablePostFX(bool enabled) override;

    // High-level drawing
    [[nodiscard]] bool DrawQuad(const QuadDesc& desc) override;
    [[nodiscard]] bool DrawText(const TextDesc& desc) override;

    // Text measurement
    [[nodiscard]] Float2 MeasureText(const std::string& text,
                                     const Ref<Font>& font,
                                     float scale) override;
    
     // Stats accessors (not part of base interface yet)
     struct Stats {
          std::size_t drawCalls = 0;
          std::size_t vertices = 0;
          std::size_t triangles = 0;
          std::size_t requestedQuads = 0;
          std::size_t requestedTextGlyphs = 0;
        std::size_t requestedTiles = 0; // subset of requestedQuads originating from tilemap rendering
     };
     [[nodiscard]] Stats GetStats() const;

    // TEMPORARY: Public PostFX method for migration phase
    // TODO: Remove once ExposurePass has direct implementation
    void ApplyPostFX();

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
    // Projection recompute hook
    void RecomputeProjection();
    void MarkProjectionDirty();
    // PostFX helpers
    void CreatePostFXResources();
    void DestroyPostFXResources();
};

} // namespace Graphics
} // namespace SAGE
