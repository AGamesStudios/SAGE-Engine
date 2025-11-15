#pragma once

#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Graphics/Rendering/Commands/RenderCommand.h"
#include "OpenGLStateCache.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace SAGE { namespace Graphics {

class OpenGLRenderBackend final : public IRenderBackend {
public:
    void Init() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const override;
    void Configure(const Graphics::RenderSystemConfig& config) override;

    void Clear(float r, float g, float b, float a) override;
    void Clear() override;
    void ClearDepth(float depth = 1.0f) override;
    void ClearStencil(int stencil = 0) override;

    void SetViewport(int x, int y, std::size_t width, std::size_t height) override;

    // Low-level binding and drawing (required by IRenderBackend)
    void BindShader(unsigned int programId) override;
    void UnbindShader() override;
    void BindTexture(unsigned int slot, unsigned int textureId) override;
    void UnbindTexture(unsigned int slot) override;
    void BindVertexArray(unsigned int vao) override;
    void UnbindVertexArray() override;
    void BindVertexBuffer(unsigned int vbo) override;
    void BindIndexBuffer(unsigned int ebo) override;

    void DrawArrays(unsigned int primitiveType, std::size_t first, std::size_t count) override;
    void DrawIndexed(unsigned int primitiveType, std::size_t indexCount, unsigned int indexType, const void* indices) override;

    void EnableBlend(bool enable) override;
    void SetBlendFunc(unsigned int srcFactor, unsigned int dstFactor) override;
    void SetBlendFuncSeparate(unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha) override;

    void EnableDepthTest(bool enable) override;
    void SetDepthFunc(unsigned int func) override;
    void SetDepthMask(bool writeEnabled) override;

    void EnableCullFace(bool enable) override;
    void SetCullFace(unsigned int mode) override;

    void EnableScissorTest(bool enable) override;
    void SetScissor(int x, int y, std::size_t width, std::size_t height) override;

    [[nodiscard]] std::size_t GetDrawCallCount() const override;
    [[nodiscard]] std::size_t GetVertexCount() const override;
    void ResetStats() override;

    // Resource management
    [[nodiscard]] TextureHandle CreateTexture(const TextureDesc& desc) override;
    void UpdateTexture(TextureHandle handle, const void* data, std::size_t dataSize, uint32_t mipLevel = 0) override;
    void DestroyTexture(TextureHandle handle) override;
    
    [[nodiscard]] BufferHandle CreateBuffer(const BufferDesc& desc) override;
    void UpdateBuffer(BufferHandle handle, const void* data, std::size_t size, std::size_t offset) override;
    void DestroyBuffer(BufferHandle handle) override;
    
    [[nodiscard]] FramebufferHandle CreateFramebuffer(const FramebufferDesc& desc) override;
    void DestroyFramebuffer(FramebufferHandle handle) override;
    void BindFramebuffer(FramebufferHandle handle) override;
    
    void BindTextureToSlot(TextureHandle handle, unsigned int slot) override;
    void UnbindTextureSlot(unsigned int slot) override;

private:
    // High-level scene rendering moved to OpenGLSceneRenderer.
    bool m_Initialized = false;
    std::size_t m_DrawCallsThisFrame = 0;
    std::size_t m_VerticesThisFrame = 0;

    // OpenGL state cache (eliminates glGet* calls - 500x faster)
    OpenGLStateCache m_StateCache;
    
    // Texture metadata cache (eliminates glGetTexLevelParameteriv - 6-9x faster)
    struct TextureMetadata {
        uint32_t width;
        uint32_t height;
        unsigned int internalFormat;
        unsigned int format;
        unsigned int type;
    };
    std::unordered_map<TextureHandle, TextureMetadata> m_TextureMetadata;
};

} // namespace Graphics
} // namespace SAGE
