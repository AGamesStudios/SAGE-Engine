#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Camera2D.h"  // Include Camera2D class
#include "Memory/Ref.h"

#include <string>

namespace SAGE {

class Font;

namespace Graphics {

/// High-level scene renderer interface
/// Handles scene-level operations: quads, text, effects, camera shake
/// Translates to low-level backend primitives
class ISceneRenderer {
public:
    virtual ~ISceneRenderer() noexcept = default;

    // Initialization
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    [[nodiscard]] virtual bool IsInitialized() const = 0;

    // Per-frame update (for shake, animations, etc.)
    virtual void Update(float deltaTime) = 0;

    // Camera management
    virtual void SetCamera(const Camera2D& camera) = 0;
    [[nodiscard]] virtual const Camera2D& GetCamera() const = 0;
    virtual void ResetCamera() = 0;

    // Screen shake
    virtual void PushScreenShake(float amplitude, float frequency, float duration) = 0;

#ifdef SAGE_ENGINE_TESTING
    [[nodiscard]] virtual Vector2 GetCameraShakeOffsetForTesting() const = 0;
    [[nodiscard]] virtual float GetShakeStrengthForTesting() const = 0;
    [[nodiscard]] virtual float GetShakeDurationForTesting() const = 0;
    [[nodiscard]] virtual float GetShakeTimerForTesting() const = 0;
#endif

    // Frame lifecycle
    virtual void BeginScene() = 0;
    [[nodiscard]] virtual bool EndScene() = 0;

    // Layer management
    virtual void SetLayer(float layer) = 0;
    virtual void PushLayer(float layer) = 0;
    virtual void PopLayer() = 0;

    // Material
    virtual MaterialId SetMaterial(MaterialId materialId) = 0;

    // Blend mode
    virtual void PushBlendMode(BlendMode mode) = 0;
    virtual void PopBlendMode() = 0;
    virtual void SetBlendMode(BlendMode mode) = 0;
    [[nodiscard]] virtual BlendMode GetBlendMode() const = 0;

    // Depth state
    virtual void PushDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                                float biasConstant, float biasSlope) = 0;
    virtual void PopDepthState() = 0;
    virtual void SetDepthState(bool enableTest, bool enableWrite, DepthFunction function,
                               float biasConstant, float biasSlope) = 0;
    [[nodiscard]] virtual DepthSettings GetDepthState() const = 0;

    // Effects
    virtual void PushEffect(const QuadEffect& effect) = 0;
    virtual void PopEffect() = 0;

    // Post-processing
    virtual void ConfigurePostFX(const PostFXSettings& settings) = 0;
    [[nodiscard]] virtual const PostFXSettings& GetPostFXSettings() const = 0;
    virtual void EnablePostFX(bool enabled) = 0;

    // High-level drawing
    [[nodiscard]] virtual bool DrawQuad(const QuadDesc& desc) = 0;
    [[nodiscard]] virtual bool DrawText(const TextDesc& desc) = 0;

    // Text measurement
    [[nodiscard]] virtual Float2 MeasureText(const std::string& text,
                                             const Ref<Font>& font,
                                             float scale) = 0;
};

} // namespace Graphics
} // namespace SAGE
