#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Types/GraphicsTypes.h"
#include "Graphics/Core/Resources/Material.h"
#include "Memory/Ref.h"

#include <cstddef>
#include <memory>

namespace SAGE::Graphics {
    struct RenderSystemConfig;
    class ISceneRenderer;
}

namespace SAGE {

class Font;

/// Render backend interface
/// 
/// This interface combines both low-level rendering primitives (OpenGL-like operations)
/// and high-level scene rendering operations (quads, text, camera, effects).
/// 
/// Architecture Note:
/// This is a transitional design. In the future, this should be split into:
/// - IRenderContext: Low-level primitives (Bind*, Draw*, Enable*, Set*)
/// - ISceneRenderer: High-level scene operations (Camera, Layers, Effects, DrawQuad/Text)
/// 
/// Current implementations:
/// - OpenGLRenderBackend: Full OpenGL implementation
/// - Future: VulkanRenderBackend, DirectX12RenderBackend
/// 
/// Thread Safety: Not thread-safe. All calls must be from the render thread.
/// 
/// @see ISceneRenderer for high-level scene operations only
/// @see IRenderDevice for resource creation
/// @see IRenderContext for state management
class IRenderBackend {
public:
    virtual ~IRenderBackend() noexcept = default;

    //==============================================================================
    // Lifecycle Management
    //==============================================================================
    
    /// Initialize the render backend
    /// Must be called before any other methods
    /// @note Idempotent - multiple calls are safe
    virtual void Init() = 0;
    
    /// Shutdown the render backend and release all resources
    /// @note Safe to call multiple times
    virtual void Shutdown() = 0;
    
    /// Check if the backend is initialized
    /// @return true if Init() was called and Shutdown() was not
    [[nodiscard]] virtual bool IsInitialized() const = 0;
    
    /// Configure backend with system settings
    /// @param config Configuration including window size, vsync, MSAA, etc.
    /// @note Can be called while initialized to change settings
    virtual void Configure(const Graphics::RenderSystemConfig& config) = 0;

    //==============================================================================
    // Minimal Backend Update (optional internal housekeeping)
    //==============================================================================
    virtual void Update(float /*deltaTime*/) { /* optional */ }

    //==============================================================================
    // Clear Operations
    //==============================================================================
    
    /// Clear screen with specified color
    /// @param r Red component [0.0, 1.0]
    /// @param g Green component [0.0, 1.0]
    /// @param b Blue component [0.0, 1.0]
    /// @param a Alpha component [0.0, 1.0]
    virtual void Clear(float r, float g, float b, float a) = 0;
    
    /// Clear screen with default black color
    virtual void Clear() = 0;
    
    /// Clear depth buffer
    /// @param depth Depth value to clear to (default 1.0 = far plane)
    virtual void ClearDepth(float depth = 1.0f) = 0;
    
    /// Clear stencil buffer
    /// @param stencil Stencil value to clear to (default 0)
    virtual void ClearStencil(int stencil = 0) = 0;

    //==============================================================================
    // Viewport
    //==============================================================================
    
    /// Set rendering viewport
    /// @param x Bottom-left x coordinate
    /// @param y Bottom-left y coordinate
    /// @param width Viewport width in pixels
    /// @param height Viewport height in pixels
    virtual void SetViewport(int x, int y, std::size_t width, std::size_t height) = 0;

    //==============================================================================
    // Low-Level Binding Operations
    //==============================================================================
    
    /// Bind shader program for rendering
    /// @param shaderProgram OpenGL shader program handle
    virtual void BindShader(unsigned int shaderProgram) = 0;
    
    /// Unbind current shader
    virtual void UnbindShader() = 0;

    /// Bind texture to a texture unit
    /// @param slot Texture unit (0-31 typically)
    /// @param textureHandle OpenGL texture handle
    virtual void BindTexture(unsigned int slot, unsigned int textureHandle) = 0;
    
    /// Unbind texture from a slot
    /// @param slot Texture unit to unbind
    virtual void UnbindTexture(unsigned int slot) = 0;

    /// Bind vertex array object
    /// @param vao OpenGL VAO handle
    virtual void BindVertexArray(unsigned int vao) = 0;
    
    /// Unbind current VAO
    virtual void UnbindVertexArray() = 0;

    /// Bind vertex buffer
    /// @param vbo OpenGL VBO handle
    virtual void BindVertexBuffer(unsigned int vbo) = 0;
    
    /// Bind index buffer
    /// @param ebo OpenGL EBO handle
    virtual void BindIndexBuffer(unsigned int ebo) = 0;

    //==============================================================================
    // Draw Calls
    //==============================================================================
    
    /// Draw primitives from vertex array
    /// @param primitiveType GL_TRIANGLES, GL_LINES, etc.
    /// @param first First vertex index
    /// @param count Number of vertices to draw
    virtual void DrawArrays(unsigned int primitiveType, std::size_t first, std::size_t count) = 0;
    
    /// Draw primitives using index buffer
    /// @param primitiveType GL_TRIANGLES, GL_LINES, etc.
    /// @param indexCount Number of indices to draw
    /// @param indexType GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, etc.
    /// @param indices Offset into index buffer (or nullptr for start)
    virtual void DrawIndexed(unsigned int primitiveType, std::size_t indexCount, unsigned int indexType, const void* indices) = 0;

    //==============================================================================
    // State Management (Low-Level GL State)
    //==============================================================================
    
    /// Enable/disable blending
    /// @param enable true to enable, false to disable
    virtual void EnableBlend(bool enable) = 0;
    
    /// Set blend function
    /// @param srcFactor Source blend factor (GL_SRC_ALPHA, etc.)
    /// @param dstFactor Destination blend factor (GL_ONE_MINUS_SRC_ALPHA, etc.)
    virtual void SetBlendFunc(unsigned int srcFactor, unsigned int dstFactor) = 0;
    
    /// Set separate blend function for RGB and Alpha
    /// @param srcRGB Source RGB factor
    /// @param dstRGB Destination RGB factor
    /// @param srcAlpha Source alpha factor
    /// @param dstAlpha Destination alpha factor
    virtual void SetBlendFuncSeparate(unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha) = 0;
    
    /// Enable/disable depth testing
    /// @param enable true to enable, false to disable
    virtual void EnableDepthTest(bool enable) = 0;
    
    /// Set depth comparison function
    /// @param func GL_LESS, GL_LEQUAL, GL_GREATER, etc.
    virtual void SetDepthFunc(unsigned int func) = 0;
    
    /// Enable/disable depth buffer writes
    /// @param writeEnabled true to allow writes, false to make read-only
    virtual void SetDepthMask(bool writeEnabled) = 0;

    /// Enable/disable face culling
    /// @param enable true to enable, false to disable
    virtual void EnableCullFace(bool enable) = 0;
    
    /// Set which face to cull
    /// @param mode GL_BACK, GL_FRONT, GL_FRONT_AND_BACK
    virtual void SetCullFace(unsigned int mode) = 0;

    /// Enable/disable scissor test
    /// @param enable true to enable, false to disable
    virtual void EnableScissorTest(bool enable) = 0;
    
    /// Set scissor rectangle
    /// @param x Bottom-left x coordinate
    /// @param y Bottom-left y coordinate
    /// @param width Rectangle width
    /// @param height Rectangle height
    virtual void SetScissor(int x, int y, std::size_t width, std::size_t height) = 0;

    //==============================================================================
    // Resource Management (Backend-Agnostic)
    //==============================================================================
    
    /// Create a texture resource
    /// @param desc Texture creation descriptor
    /// @return Handle to created texture or InvalidTextureHandle on failure
    [[nodiscard]] virtual Graphics::TextureHandle CreateTexture(const Graphics::TextureDesc& desc) = 0;
    
    /// Destroy a texture resource
    /// @param handle Texture handle to destroy
    virtual void DestroyTexture(Graphics::TextureHandle handle) = 0;
    
    /// Update texture data
    /// @param handle Texture to update
    /// @param data Pointer to pixel data
    /// @param dataSize Size of data in bytes
    /// @param mipLevel Which mip level to update (0 = base)
    virtual void UpdateTexture(Graphics::TextureHandle handle, const void* data, size_t dataSize, uint32_t mipLevel = 0) = 0;
    
    /// Create a framebuffer resource
    /// @param desc Framebuffer creation descriptor
    /// @return Handle to created framebuffer or InvalidFramebufferHandle on failure
    [[nodiscard]] virtual Graphics::FramebufferHandle CreateFramebuffer(const Graphics::FramebufferDesc& desc) = 0;
    
    /// Destroy a framebuffer resource
    /// @param handle Framebuffer handle to destroy
    virtual void DestroyFramebuffer(Graphics::FramebufferHandle handle) = 0;
    
    /// Bind framebuffer for rendering
    /// @param handle Framebuffer to bind (InvalidFramebufferHandle = default/screen framebuffer)
    virtual void BindFramebuffer(Graphics::FramebufferHandle handle) = 0;
    
    /// Create a buffer resource
    /// @param desc Buffer creation descriptor
    /// @return Handle to created buffer or InvalidBufferHandle on failure
    [[nodiscard]] virtual Graphics::BufferHandle CreateBuffer(const Graphics::BufferDesc& desc) = 0;
    
    /// Destroy a buffer resource
    /// @param handle Buffer handle to destroy
    virtual void DestroyBuffer(Graphics::BufferHandle handle) = 0;
    
    /// Update buffer data
    /// @param handle Buffer to update
    /// @param data Pointer to data
    /// @param dataSize Size of data in bytes
    /// @param offset Offset in buffer to start writing
    virtual void UpdateBuffer(Graphics::BufferHandle handle, const void* data, size_t dataSize, size_t offset = 0) = 0;
    
    /// Bind texture to a slot (replaces direct glBindTexture calls)
    /// @param handle Texture handle
    /// @param slot Texture unit (0-31 typically)
    virtual void BindTextureToSlot(Graphics::TextureHandle handle, unsigned int slot) = 0;
    
    /// Unbind texture from a slot
    /// @param slot Texture unit to unbind
    virtual void UnbindTextureSlot(unsigned int slot) = 0;

    // High-level scene operations have moved to Graphics::ISceneRenderer.
    // IRenderBackend now focuses strictly on low-level GPU state & draw primitives.

    //==============================================================================
    // Diagnostics
    //==============================================================================
    
    /// Get number of draw calls this frame
    /// @return Draw call count
    [[nodiscard]] virtual std::size_t GetDrawCallCount() const = 0;
    
    /// Get number of vertices rendered this frame
    /// @return Vertex count
    [[nodiscard]] virtual std::size_t GetVertexCount() const = 0;
    
    /// Reset frame statistics
    /// Call at the beginning of each frame
    virtual void ResetStats() = 0;
};

} // namespace SAGE
