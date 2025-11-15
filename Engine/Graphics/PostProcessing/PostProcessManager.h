#pragma once

#include "Graphics/Core/Types/GraphicsTypes.h"
#include <memory>
#include <vector>
#include <string>
#include "Graphics/Core/Resources/Shader.h"

namespace SAGE {

struct FramebufferSpec {
    int width = 1280;
    int height = 720;
    Graphics::TextureFormat format = Graphics::TextureFormat::RGBA16F;
    bool useDepth = false;
};

class Framebuffer {
public:
    Framebuffer() = default;
    ~Framebuffer();
    
    void Create(const FramebufferSpec& spec);
    void Destroy();
    void Resize(int width, int height);
    
    void Bind() const;
    void Unbind() const;
    
    // Backend-agnostic getters
    Graphics::TextureHandle GetColorTexture() const { return m_ColorTexture; }
    Graphics::TextureHandle GetDepthTexture() const { return m_DepthTexture; }
    Graphics::FramebufferHandle GetFBO() const { return m_FBO; }
    Graphics::FramebufferHandle GetFramebufferHandle() const { return m_FBO; }
    Graphics::TextureHandle GetColorTextureHandle() const { return m_ColorTexture; }
    
    int GetWidth() const { return m_Spec.width; }
    int GetHeight() const { return m_Spec.height; }
    
private:
    Graphics::FramebufferHandle m_FBO = 0;
    Graphics::TextureHandle m_ColorTexture = 0;
    Graphics::TextureHandle m_DepthTexture = 0;
    FramebufferSpec m_Spec;
};

struct PostProcessEffect {
    std::string name;
    Shader* shader = nullptr;
    bool enabled = true;
    float intensity = 1.0f;
    
    virtual void Apply(Framebuffer& input, Framebuffer& output) {}
    virtual void UpdateUniforms() {}
};

class PostProcessManager {
public:
    PostProcessManager();
    ~PostProcessManager();
    
    void Init(int width, int height);
    void Shutdown();
    void Resize(int width, int height);
    
    // Add effects
    void AddBloomEffect(float threshold = 0.6f, float strength = 1.0f, int blurPasses = 12);
    void AddVignetteEffect(float intensity = 0.5f, float smoothness = 0.5f);
    void AddChromaticAberration(float strength = 0.002f);
    void AddToneMapping(bool useACES = true);
    
    // Effect control
    void SetEffectEnabled(const std::string& name, bool enabled);
    void SetEffectIntensity(const std::string& name, float intensity);
    bool IsEffectEnabled(const std::string& name) const;
    
    // Rendering
    void BeginScene();
    void EndScene();
    GLuint GetFinalTexture() const;
    
    // Scene framebuffer access
    Framebuffer& GetSceneFramebuffer() { return m_SceneFramebuffer; }
    
    // Parameters
    struct BloomParams {
        float threshold = 0.6f;
        float strength = 1.0f;
        float blurSize = 3.0f;
        int blurPasses = 12;
    };
    
    struct VignetteParams {
        float intensity = 0.5f;
        float smoothness = 0.5f;
    };
    
    struct ChromaticAberrationParams {
        float strength = 0.002f;
    };
    
    BloomParams& GetBloomParams() { return m_BloomParams; }
    VignetteParams& GetVignetteParams() { return m_VignetteParams; }
    ChromaticAberrationParams& GetChromaticAberrationParams() { return m_ChromaticParams; }
    
private:
    void CreateQuad();
    void CreateShaders();
    void ApplyBloom(Framebuffer& input, Framebuffer& output);
    void ApplyToneMapping(Framebuffer& input, Framebuffer& output);
    void ApplyVignette(Framebuffer& input, Framebuffer& output);
    void ApplyChromaticAberration(Framebuffer& input, Framebuffer& output);
    
    int m_Width = 1280;
    int m_Height = 720;
    
    // Framebuffers
    Framebuffer m_SceneFramebuffer;
    Framebuffer m_BrightFramebuffer;
    Framebuffer m_PingPongFramebuffers[2];
    Framebuffer m_TempFramebuffer;
    
    // Shaders
    std::unique_ptr<Shader> m_BrightPassShader;
    std::unique_ptr<Shader> m_BlurShader;
    std::unique_ptr<Shader> m_CombineShader;
    std::unique_ptr<Shader> m_ToneMappingShader;
    std::unique_ptr<Shader> m_VignetteShader;
    std::unique_ptr<Shader> m_ChromaticAberrationShader;
    std::unique_ptr<Shader> m_CopyShader;
    
    // Quad for fullscreen rendering
    GLuint m_QuadVAO = 0;
    GLuint m_QuadVBO = 0;
    
    // Effect parameters
    BloomParams m_BloomParams;
    VignetteParams m_VignetteParams;
    ChromaticAberrationParams m_ChromaticParams;
    
    // Effect states
    bool m_BloomEnabled = true;
    bool m_VignetteEnabled = false;
    bool m_ChromaticAberrationEnabled = false;
    bool m_ToneMappingEnabled = true;
};

} // namespace SAGE
