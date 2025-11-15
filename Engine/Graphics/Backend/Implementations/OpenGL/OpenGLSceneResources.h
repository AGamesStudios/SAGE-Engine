#pragma once

#include "Graphics/Backend/Interfaces/IBackendSceneResources.h"
#include <vector>
#include <cstdint>

namespace SAGE {
namespace Graphics {

/// OpenGL implementation of scene resources
/// Manages FBOs, textures, and VAOs for scene rendering and post-processing
class OpenGLSceneResources : public IBackendSceneResources {
public:
    OpenGLSceneResources() = default;
    ~OpenGLSceneResources() override;
    
    void Initialize(int width, int height) override;
    void Shutdown() override;
    bool IsInitialized() const override;
    void Resize(int width, int height) override;
    
    uint32_t GetSceneColorTexture() const override;
    uint32_t GetSceneFramebuffer() const override;
    uint32_t GetBlurTexture(int index) const override;
    uint32_t GetFullscreenQuadVAO() const override;

private:
    void CreateSceneFBO(int width, int height);
    void DestroySceneFBO();
    void CreateBlurTextures(int width, int height);
    void DestroyBlurTextures();
    void CreateFullscreenQuad();
    void DestroyFullscreenQuad();
    
    bool m_Initialized = false;
    int m_Width = 0;
    int m_Height = 0;
    
    // Scene render target
    uint32_t m_SceneFBO = 0;
    uint32_t m_SceneColorTexture = 0;
    uint32_t m_SceneDepthRBO = 0; // Renderbuffer for depth (if needed)
    
    // Blur intermediate textures (ping-pong)
    std::vector<uint32_t> m_BlurTextures;
    std::vector<uint32_t> m_BlurFBOs;
    
    // Fullscreen quad VAO/VBO for post-processing
    uint32_t m_FullscreenQuadVAO = 0;
    uint32_t m_FullscreenQuadVBO = 0;
};

} // namespace Graphics
} // namespace SAGE
