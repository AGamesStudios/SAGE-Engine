#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Resources/Material.h"
#include "Memory/Ref.h"

namespace SAGE {

class Font;

namespace Graphics {

/// Command type enumeration
enum class RenderCommandType {
    Clear,
    SetViewport,
    SetCamera,
    SetMaterial,
    SetBlendMode,
    SetDepthState,
    PushEffect,
    PopEffect,
    DrawQuad,
    DrawText,
    ScreenShake
};

/// Base render command
struct RenderCommand {
    RenderCommandType type;
    
    virtual ~RenderCommand() = default;
    
protected:
    explicit RenderCommand(RenderCommandType cmdType) : type(cmdType) {}
};

/// Clear command
struct ClearCommand : RenderCommand {
    float r, g, b, a;
    
    ClearCommand() : RenderCommand(RenderCommandType::Clear), r(0), g(0), b(0), a(1) {}
    ClearCommand(float red, float green, float blue, float alpha)
        : RenderCommand(RenderCommandType::Clear), r(red), g(green), b(blue), a(alpha) {}
};

/// Set camera command
struct SetCameraCommand : RenderCommand {
    Camera2D camera;
    
    SetCameraCommand() : RenderCommand(RenderCommandType::SetCamera) {}
    explicit SetCameraCommand(const Camera2D& cam)
        : RenderCommand(RenderCommandType::SetCamera), camera(cam) {}
};

/// Set material command
struct SetMaterialCommand : RenderCommand {
    MaterialId materialId;
    
    SetMaterialCommand() : RenderCommand(RenderCommandType::SetMaterial), materialId(0) {}
    explicit SetMaterialCommand(MaterialId id)
        : RenderCommand(RenderCommandType::SetMaterial), materialId(id) {}
};

/// Set blend mode command
struct SetBlendModeCommand : RenderCommand {
    BlendMode mode;
    
    SetBlendModeCommand() : RenderCommand(RenderCommandType::SetBlendMode), mode(BlendMode::Alpha) {}
    explicit SetBlendModeCommand(BlendMode blendMode)
        : RenderCommand(RenderCommandType::SetBlendMode), mode(blendMode) {}
};

/// Set depth state command
struct SetDepthStateCommand : RenderCommand {
    DepthSettings settings;
    
    SetDepthStateCommand() : RenderCommand(RenderCommandType::SetDepthState) {}
    explicit SetDepthStateCommand(const DepthSettings& depthSettings)
        : RenderCommand(RenderCommandType::SetDepthState), settings(depthSettings) {}
};

/// Push effect command
struct PushEffectCommand : RenderCommand {
    QuadEffect effect;
    
    PushEffectCommand() : RenderCommand(RenderCommandType::PushEffect) {}
    explicit PushEffectCommand(const QuadEffect& quadEffect)
        : RenderCommand(RenderCommandType::PushEffect), effect(quadEffect) {}
};

/// Pop effect command
struct PopEffectCommand : RenderCommand {
    PopEffectCommand() : RenderCommand(RenderCommandType::PopEffect) {}
};

/// Draw quad command
struct DrawQuadCommand : RenderCommand {
    QuadDesc desc;
    
    DrawQuadCommand() : RenderCommand(RenderCommandType::DrawQuad) {}
    explicit DrawQuadCommand(const QuadDesc& quadDesc)
        : RenderCommand(RenderCommandType::DrawQuad), desc(quadDesc) {}
};

/// Draw text command
struct DrawTextCommand : RenderCommand {
    TextDesc desc;
    
    DrawTextCommand() : RenderCommand(RenderCommandType::DrawText) {}
    explicit DrawTextCommand(const TextDesc& textDesc)
        : RenderCommand(RenderCommandType::DrawText), desc(textDesc) {}
};

/// Screen shake command
struct ScreenShakeCommand : RenderCommand {
    float amplitude;
    float frequency;
    float duration;
    
    ScreenShakeCommand()
        : RenderCommand(RenderCommandType::ScreenShake)
        , amplitude(0), frequency(0), duration(0) {}
    
    ScreenShakeCommand(float amp, float freq, float dur)
        : RenderCommand(RenderCommandType::ScreenShake)
        , amplitude(amp), frequency(freq), duration(dur) {}
};

} // namespace Graphics
} // namespace SAGE
