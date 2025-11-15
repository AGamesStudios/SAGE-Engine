#include "RenderTarget.h"
#include "Core/Logger.h"
#include "Graphics/Core/Resources/Material.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"

namespace SAGE {

// ===== RenderTarget Implementation =====

RenderTarget::RenderTarget(const Specification& spec)
    : m_Specification(spec) {
    
    // Override format if HDR is requested
    if (m_Specification.useHDR) {
        m_Specification.colorFormat = Graphics::TextureFormat::RGBA16F;
    }
    
    Invalidate();
}

RenderTarget::~RenderTarget() {
    Release();
}

void RenderTarget::Invalidate() {
    Release();
    
    auto* backend = Renderer::GetRenderBackend();
    if (!backend) {
        SAGE_ERROR("RenderBackend is null, cannot create RenderTarget");
        return;
    }
    
    // Create color texture
    Graphics::TextureDesc colorDesc;
    colorDesc.width = m_Specification.width;
    colorDesc.height = m_Specification.height;
    colorDesc.format = m_Specification.colorFormat;
    colorDesc.minFilter = Graphics::TextureFilter::Linear;
    colorDesc.magFilter = Graphics::TextureFilter::Linear;
    colorDesc.wrapU = Graphics::TextureWrap::ClampToEdge;
    colorDesc.wrapV = Graphics::TextureWrap::ClampToEdge;
    colorDesc.samples = m_Specification.samples;
    colorDesc.isRenderTarget = true;
    colorDesc.generateMipmaps = false;
    
    m_ColorTexture = backend->CreateTexture(colorDesc);
    
    // Create depth texture if needed
    if (m_Specification.useDepth) {
        Graphics::TextureDesc depthDesc;
        depthDesc.width = m_Specification.width;
        depthDesc.height = m_Specification.height;
        
        if (m_Specification.useStencil) {
            depthDesc.format = Graphics::TextureFormat::Depth24Stencil8;
        } else {
            depthDesc.format = Graphics::TextureFormat::Depth24;
        }
        
        depthDesc.minFilter = Graphics::TextureFilter::Linear;
        depthDesc.magFilter = Graphics::TextureFilter::Linear;
        depthDesc.wrapU = Graphics::TextureWrap::ClampToEdge;
        depthDesc.wrapV = Graphics::TextureWrap::ClampToEdge;
        depthDesc.samples = m_Specification.samples;
        depthDesc.isRenderTarget = true;
        depthDesc.generateMipmaps = false;
        
        m_DepthTexture = backend->CreateTexture(depthDesc);
    }
    
    // Create framebuffer
    Graphics::FramebufferDesc fbDesc;
    fbDesc.width = m_Specification.width;
    fbDesc.height = m_Specification.height;
    fbDesc.samples = m_Specification.samples;
    
    fbDesc.attachments[0].type = Graphics::FramebufferAttachment::Color0;
    fbDesc.attachments[0].format = m_Specification.colorFormat;
    fbDesc.attachments[0].existingTexture = m_ColorTexture;
    fbDesc.attachmentCount = 1;
    
    if (m_Specification.useDepth) {
        fbDesc.attachments[1].type = m_Specification.useStencil ? 
            Graphics::FramebufferAttachment::DepthStencil : Graphics::FramebufferAttachment::Depth;
        fbDesc.attachments[1].format = m_Specification.useStencil ?
            Graphics::TextureFormat::Depth24Stencil8 : Graphics::TextureFormat::Depth24;
        fbDesc.attachments[1].existingTexture = m_DepthTexture;
        fbDesc.attachmentCount = 2;
    }
    
    m_Framebuffer = backend->CreateFramebuffer(fbDesc);
    
    SAGE_INFO("RenderTarget created: {}x{}", m_Specification.width, m_Specification.height);
}

void RenderTarget::Release() {
    auto* backend = Renderer::GetRenderBackend();
    if (!backend) return;
    
    if (m_Framebuffer) {
        backend->DestroyFramebuffer(m_Framebuffer);
        m_Framebuffer = 0;
    }
    if (m_ColorTexture) {
        backend->DestroyTexture(m_ColorTexture);
        m_ColorTexture = 0;
    }
    if (m_DepthTexture) {
        backend->DestroyTexture(m_DepthTexture);
        m_DepthTexture = 0;
    }
}

void RenderTarget::Bind() {
    // Binding will be handled by backend during rendering
    // For now, keeping this method for compatibility
    // TODO: Remove once all rendering uses backend directly
}

void RenderTarget::Unbind() {
    // Unbinding will be handled by backend during rendering
    // For now, keeping this method for compatibility
    // TODO: Remove once all rendering uses backend directly
}

void RenderTarget::Resize(unsigned int width, unsigned int height) {
    if (width == m_Specification.width && height == m_Specification.height) {
        return;
    }
    
    m_Specification.width = width;
    m_Specification.height = height;
    
    Invalidate();
    
    // Invalidate texture wrapper
    m_TextureWrapper.reset();
}

void RenderTarget::Clear(const Color& color) {
    // TODO: Implement Clear through backend
    // For now, this is called during Bind() in the rendering loop
    // Backend should handle framebuffer clear operations
}

Ref<Texture> RenderTarget::AsTexture() {
    if (!m_TextureWrapper) {
        // Create a wrapper Texture that references our texture handle
        m_TextureWrapper = Ref<Texture>(new Texture());
        m_TextureWrapper->SetWidth(m_Specification.width);
        m_TextureWrapper->SetHeight(m_Specification.height);
        // Convert handle to GLuint for legacy Texture class
        // TODO: Refactor Texture class to use TextureHandle directly
        m_TextureWrapper->SetGLTexture(static_cast<unsigned int>(m_ColorTexture));
    }
    return m_TextureWrapper;
}

// ===== CameraRenderSystem Implementation =====

CameraRenderSystem::CameraRenderSystem() {
}

CameraRenderSystem::~CameraRenderSystem() {
    Clear();
}

RenderTarget* CameraRenderSystem::CreateRenderTarget(const std::string& name, 
                                                      const RenderTarget::Specification& spec) {
    auto it = m_RenderTargets.find(name);
    if (it != m_RenderTargets.end()) {
        SAGE_WARNING("RenderTarget '{}' already exists, replacing", name);
        m_RenderTargets.erase(it);
    }
    
    auto renderTarget = std::make_unique<RenderTarget>(spec);
    auto* ptr = renderTarget.get();
    m_RenderTargets[name] = std::move(renderTarget);
    
    SAGE_INFO("Created RenderTarget '{}'", name);
    return ptr;
}

RenderTarget* CameraRenderSystem::GetRenderTarget(const std::string& name) {
    auto it = m_RenderTargets.find(name);
    if (it == m_RenderTargets.end()) {
        SAGE_ERROR("RenderTarget '{}' not found", name);
        return nullptr;
    }
    return it->second.get();
}

void CameraRenderSystem::RemoveRenderTarget(const std::string& name) {
    auto it = m_RenderTargets.find(name);
    if (it != m_RenderTargets.end()) {
        m_RenderTargets.erase(it);
        SAGE_INFO("Removed RenderTarget '{}'", name);
    }
}

void CameraRenderSystem::RenderToTarget(const std::string& targetName, 
                                        Camera2D& camera, 
                                        RenderCallback renderCallback) {
    auto* target = GetRenderTarget(targetName);
    if (!target) {
        SAGE_ERROR("Cannot render to target '{}': not found", targetName);
        return;
    }
    
    // TODO: Implement proper framebuffer/viewport save/restore through backend
    // For now, Bind() is called which doesn't actually bind anything
    
    // Bind render target (no-op for now)
    target->Bind();
    target->Clear();
    
    // Update camera viewport to match target size
    camera.SetViewportSize(static_cast<float>(target->GetWidth()), 
                          static_cast<float>(target->GetHeight()));
    
    // Render scene
    if (renderCallback) {
        renderCallback();
    }
    
    // Unbind (no-op for now)
    target->Unbind();
}

void CameraRenderSystem::RenderToTarget(const std::string& targetName, 
                                        RenderCallback renderCallback) {
    auto* target = GetRenderTarget(targetName);
    if (!target) {
        SAGE_ERROR("Cannot render to target '{}': not found", targetName);
        return;
    }
    
    // TODO: Implement proper framebuffer/viewport save/restore through backend
    
    // Bind render target (no-op for now)
    target->Bind();
    target->Clear();
    
    // Render
    if (renderCallback) {
        renderCallback();
    }
    
    // Unbind (no-op for now)
    target->Unbind();
}

Ref<Texture> CameraRenderSystem::GetTargetTexture(const std::string& name) {
    auto* target = GetRenderTarget(name);
    if (!target) {
        return nullptr;
    }
    return target->AsTexture();
}

void CameraRenderSystem::ResizeAll(unsigned int width, unsigned int height) {
    for (auto& [name, target] : m_RenderTargets) {
        target->Resize(width, height);
    }
    SAGE_INFO("Resized all RenderTargets to {}x{}", width, height);
}

void CameraRenderSystem::Clear() {
    m_RenderTargets.clear();
    SAGE_INFO("Cleared all RenderTargets");
}

} // namespace SAGE
