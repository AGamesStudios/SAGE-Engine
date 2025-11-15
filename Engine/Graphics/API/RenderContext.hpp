#pragma once

#include <memory>

namespace SAGE::Graphics {

class RenderSystemRegistry;
struct RenderSystemConfig;
class IRenderDevice;
class IRenderContext;
class IResourceManager;

// Forward declaration of concrete instance-based context defined in Core/RenderContext.h
class RenderContext; // SAGE::Graphics::RenderContext (core implementation)

// Factory that wires backend device/context/resource manager into RenderSystemRegistry
std::unique_ptr<RenderContext> CreateDefaultRenderContext(const RenderSystemConfig& config,
                                                          RenderSystemRegistry& registry);

} // namespace SAGE::Graphics
