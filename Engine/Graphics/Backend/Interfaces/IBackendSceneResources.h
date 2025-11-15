#pragma once

#include <cstdint>

namespace SAGE {
namespace Graphics {

/// Abstract interface for backend-specific scene resources
/// Encapsulates FBOs, textures, VAOs used across render passes
/// Allows PostFX and other passes to request resources without direct backend coupling
class IBackendSceneResources {
public:
    virtual ~IBackendSceneResources() = default;
    
    /// Initialize resources (allocate FBOs, textures, etc.)
    virtual void Initialize(int width, int height) = 0;
    
    /// Cleanup resources
    virtual void Shutdown() = 0;
    
    /// Check if resources are ready
    virtual bool IsInitialized() const = 0;
    
    /// Resize resources (e.g., on viewport change)
    virtual void Resize(int width, int height) = 0;
    
    // PostFX resource accessors
    /// Get main scene color texture (output of geometry passes)
    virtual uint32_t GetSceneColorTexture() const = 0;
    
    /// Get scene framebuffer object
    virtual uint32_t GetSceneFramebuffer() const = 0;
    
    /// Get blur intermediate texture (for multi-pass blur)
    virtual uint32_t GetBlurTexture(int index) const = 0;
    
    /// Get fullscreen quad VAO for post-processing
    virtual uint32_t GetFullscreenQuadVAO() const = 0;
    
    // Future extensions:
    // virtual uint32_t GetShadowMapTexture() const = 0;
    // virtual uint32_t GetGBufferTexture(int index) const = 0;
    // virtual uint32_t GetDepthTexture() const = 0;
};

} // namespace Graphics
} // namespace SAGE
