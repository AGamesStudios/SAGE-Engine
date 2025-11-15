#pragma once

#include "RenderHandles.h"

#include <cstdint>

namespace SAGE::Graphics {

struct Viewport {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct ScissorRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

enum class BlendFactor : std::uint8_t {
    One,
    Zero,
    SourceColor,
    InverseSourceColor,
    DestinationColor,
    InverseDestinationColor,
    SourceAlpha,
    InverseSourceAlpha,
    DestinationAlpha,
    InverseDestinationAlpha
};

enum class BlendOperation : std::uint8_t {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

struct BlendStateDesc {
    bool enabled = false;
    BlendFactor sourceColor = BlendFactor::SourceAlpha;
    BlendFactor destinationColor = BlendFactor::InverseSourceAlpha;
    BlendOperation colorOp = BlendOperation::Add;
    BlendFactor sourceAlpha = BlendFactor::One;
    BlendFactor destinationAlpha = BlendFactor::Zero;
    BlendOperation alphaOp = BlendOperation::Add;
};

enum class DepthCompare : std::uint8_t {
    Less,
    LessEqual,
    Equal,
    Greater,
    GreaterEqual,
    Always,
    Never
};

struct DepthStateDesc {
    bool testEnabled = true;
    bool writeEnabled = true;
    DepthCompare compare = DepthCompare::LessEqual;
    float biasConstant = 0.0f;
    float biasSlope = 0.0f;
};

class IRenderContext {
public:
    virtual ~IRenderContext() = default;

    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetScissor(const ScissorRect& scissor) = 0;

    virtual void SetBlendState(const BlendStateDesc& state) = 0;
    virtual void SetDepthState(const DepthStateDesc& state) = 0;

    virtual void SetRenderTarget(RenderTargetHandle handle) = 0;
};

} // namespace SAGE::Graphics
