#include "PostProcessManager.h"
#include "Core/Logger.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Graphics/Backend/Implementations/OpenGL/OpenGLTypeConversions.h"
#include "Graphics/API/Renderer.h"
#include <cstring>

namespace SAGE {

// === Framebuffer Implementation ===

Framebuffer::~Framebuffer() {
    Destroy();
}

void Framebuffer::Create(const FramebufferSpec& spec) {
    m_Spec = spec;
    
    auto* backend = Renderer::GetRenderBackend();
    if (!backend) {
        SAGE_ERROR("RenderBackend is null, cannot create framebuffer");
        return;
    }
    
    // Create color texture
    Graphics::TextureDesc colorDesc;
    colorDesc.width = spec.width;
    colorDesc.height = spec.height;
    colorDesc.format = spec.format;
    colorDesc.minFilter = Graphics::TextureFilter::Linear;
    colorDesc.magFilter = Graphics::TextureFilter::Linear;
    colorDesc.wrapU = Graphics::TextureWrap::ClampToEdge;
    colorDesc.wrapV = Graphics::TextureWrap::ClampToEdge;
    colorDesc.generateMipmaps = false;
    
    m_ColorTexture = backend->CreateTexture(colorDesc);
    
    // Create depth texture if needed
    if (spec.useDepth) {
        Graphics::TextureDesc depthDesc;
        depthDesc.width = spec.width;
        depthDesc.height = spec.height;
        depthDesc.format = Graphics::TextureFormat::Depth24;
        depthDesc.minFilter = Graphics::TextureFilter::Linear;
        depthDesc.magFilter = Graphics::TextureFilter::Linear;
        depthDesc.wrapU = Graphics::TextureWrap::ClampToEdge;
        depthDesc.wrapV = Graphics::TextureWrap::ClampToEdge;
        depthDesc.generateMipmaps = false;
        
        m_DepthTexture = backend->CreateTexture(depthDesc);
    }
    
    // Create framebuffer - using simplified approach for now
    Graphics::FramebufferDesc fbDesc;
    fbDesc.width = spec.width;
    fbDesc.height = spec.height;
    fbDesc.attachments[0].type = Graphics::FramebufferAttachment::Color0;
    fbDesc.attachments[0].format = spec.format;
    fbDesc.attachments[0].existingTexture = m_ColorTexture;
    fbDesc.attachmentCount = 1;
    
    if (spec.useDepth) {
        fbDesc.attachments[1].type = Graphics::FramebufferAttachment::Depth;
        fbDesc.attachments[1].format = Graphics::TextureFormat::Depth24;
        fbDesc.attachments[1].existingTexture = m_DepthTexture;
        fbDesc.attachmentCount = 2;
    }
    
    m_FBO = backend->CreateFramebuffer(fbDesc);
    
    SAGE_INFO("Framebuffer created: {}x{}, FBO={}, ColorTex={}", 
              spec.width, spec.height, m_FBO, m_ColorTexture);
}

void Framebuffer::Destroy() {
    auto* backend = Renderer::GetRenderBackend();
    if (!backend) return;
    
    if (m_FBO) {
        backend->DestroyFramebuffer(m_FBO);
        m_FBO = 0;
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

void Framebuffer::Resize(int width, int height) {
    if (width == m_Spec.width && height == m_Spec.height) return;
    
    // Recreate with new size
    Destroy();
    m_Spec.width = width;
    m_Spec.height = height;
    Create(m_Spec);
}

void Framebuffer::Bind() const {
    // Binding is handled by IRenderBackend during rendering
    // This method kept for compatibility but does nothing
    // TODO: Remove once all code uses RenderBackend directly
}

void Framebuffer::Unbind() const {
    // Unbinding is handled by IRenderBackend
    // This method kept for compatibility but does nothing
    // TODO: Remove once all code uses RenderBackend directly
}

// === PostProcessManager Implementation ===

PostProcessManager::PostProcessManager() {
}

PostProcessManager::~PostProcessManager() {
    Shutdown();
}

void PostProcessManager::Init(int width, int height) {
    m_Width = width;
    m_Height = height;
    
    CreateQuad();
    CreateShaders();
    
    // Create framebuffers
    FramebufferSpec spec;
    spec.width = width;
    spec.height = height;
    spec.format = Graphics::TextureFormat::RGBA16F;
    spec.useDepth = true;
    
    m_SceneFramebuffer.Create(spec);
    
    spec.useDepth = false;
    m_BrightFramebuffer.Create(spec);
    m_PingPongFramebuffers[0].Create(spec);
    m_PingPongFramebuffers[1].Create(spec);
    m_TempFramebuffer.Create(spec);
    
    SAGE_INFO("PostProcessManager initialized: {}x{}", width, height);
}

void PostProcessManager::Shutdown() {
    m_SceneFramebuffer.Destroy();
    m_BrightFramebuffer.Destroy();
    m_PingPongFramebuffers[0].Destroy();
    m_PingPongFramebuffers[1].Destroy();
    m_TempFramebuffer.Destroy();
    
    if (m_QuadVAO) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        glDeleteBuffers(1, &m_QuadVBO);
        m_QuadVAO = 0;
        m_QuadVBO = 0;
    }
    
    m_BrightPassShader.reset();
    m_BlurShader.reset();
    m_CombineShader.reset();
    m_ToneMappingShader.reset();
    m_VignetteShader.reset();
    m_ChromaticAberrationShader.reset();
    m_CopyShader.reset();
}

void PostProcessManager::Resize(int width, int height) {
    if (width == m_Width && height == m_Height) return;
    
    m_Width = width;
    m_Height = height;
    
    m_SceneFramebuffer.Resize(width, height);
    m_BrightFramebuffer.Resize(width, height);
    m_PingPongFramebuffers[0].Resize(width, height);
    m_PingPongFramebuffers[1].Resize(width, height);
    m_TempFramebuffer.Resize(width, height);
    
    SAGE_INFO("PostProcessManager resized: {}x{}", width, height);
}

void PostProcessManager::CreateQuad() {
    float vertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

void PostProcessManager::CreateShaders() {
    // Vertex shader (shared)
    const char* vertexSrc = R"(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";
    
    // Bright pass shader
    const char* brightPassSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform float u_Threshold;

void main() {
    vec3 color = texture(u_Texture, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float factor = smoothstep(u_Threshold - 0.1, u_Threshold + 0.3, brightness);
    FragColor = vec4(color * factor, 1.0);
}
)";
    
    // Gaussian blur shader
    const char* blurSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform vec2 u_Direction;
uniform float u_BlurSize;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_Texture, 0));
    vec3 result = vec3(0.0);
    
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    
    result += texture(u_Texture, TexCoords).rgb * weights[0];
    
    for(int i = 1; i < 5; i++) {
        vec2 offset = u_Direction * texelSize * float(i) * u_BlurSize;
        result += texture(u_Texture, TexCoords + offset).rgb * weights[i];
        result += texture(u_Texture, TexCoords - offset).rgb * weights[i];
    }
    
    FragColor = vec4(result, 1.0);
}
)";
    
    // Combine shader (bloom)
    const char* combineSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Scene;
uniform sampler2D u_Bloom;
uniform float u_BloomStrength;

void main() {
    vec3 scene = texture(u_Scene, TexCoords).rgb;
    vec3 bloom = texture(u_Bloom, TexCoords).rgb;
    
    vec3 result = scene + bloom * u_BloomStrength * 0.8;
    float bloomLuminance = dot(bloom, vec3(0.2126, 0.7152, 0.0722));
    result += bloom * bloomLuminance * 0.3;
    
    FragColor = vec4(result, 1.0);
}
)";
    
    // Tone mapping shader (ACES)
    const char* toneMappingSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform bool u_UseACES;

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 Reinhard(vec3 x) {
    return x / (1.0 + x);
}

void main() {
    vec3 color = texture(u_Texture, TexCoords).rgb;
    
    if (u_UseACES) {
        color = ACESFilm(color);
    } else {
        color = Reinhard(color);
    }
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
)";
    
    // Vignette shader
    const char* vignetteSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform float u_Intensity;
uniform float u_Smoothness;

void main() {
    vec3 color = texture(u_Texture, TexCoords).rgb;
    
    vec2 uv = TexCoords;
    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * 15.0;
    vig = pow(vig, u_Smoothness);
    vig = mix(1.0 - u_Intensity, 1.0, vig);
    
    color *= vig;
    
    FragColor = vec4(color, 1.0);
}
)";
    
    // Chromatic aberration shader
    const char* chromaticAberrationSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;
uniform float u_Strength;

void main() {
    vec2 offset = (TexCoords - 0.5) * u_Strength;
    
    float r = texture(u_Texture, TexCoords + offset).r;
    float g = texture(u_Texture, TexCoords).g;
    float b = texture(u_Texture, TexCoords - offset).b;
    
    FragColor = vec4(r, g, b, 1.0);
}
)";
    
    // Simple copy shader
    const char* copySrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D u_Texture;

void main() {
    FragColor = texture(u_Texture, TexCoords);
}
)";
    
    // Create shaders
    m_BrightPassShader = std::make_unique<Shader>(vertexSrc, brightPassSrc);
    
    m_BlurShader = std::make_unique<Shader>(vertexSrc, blurSrc);
    
    m_CombineShader = std::make_unique<Shader>(vertexSrc, combineSrc);
    
    m_ToneMappingShader = std::make_unique<Shader>(vertexSrc, toneMappingSrc);
    
    m_VignetteShader = std::make_unique<Shader>(vertexSrc, vignetteSrc);
    
    m_ChromaticAberrationShader = std::make_unique<Shader>(vertexSrc, chromaticAberrationSrc);
    
    m_CopyShader = std::make_unique<Shader>(vertexSrc, copySrc);
}

void PostProcessManager::BeginScene() {
    m_SceneFramebuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessManager::EndScene() {
    m_SceneFramebuffer.Unbind();
    
    GLuint currentTexture = m_SceneFramebuffer.GetColorTexture();
    
    // Apply bloom if enabled
    if (m_BloomEnabled) {
        ApplyBloom(m_SceneFramebuffer, m_TempFramebuffer);
        currentTexture = m_TempFramebuffer.GetColorTexture();
    }
    
    // Apply chromatic aberration if enabled
    if (m_ChromaticAberrationEnabled) {
        m_TempFramebuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_ChromaticAberrationShader->Bind();
        m_ChromaticAberrationShader->SetInt("u_Texture", 0);
        m_ChromaticAberrationShader->SetFloat("u_Strength", m_ChromaticParams.strength);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        currentTexture = m_TempFramebuffer.GetColorTexture();
    }
    
    // Apply vignette if enabled
    if (m_VignetteEnabled) {
        m_TempFramebuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_VignetteShader->Bind();
        m_VignetteShader->SetInt("u_Texture", 0);
        m_VignetteShader->SetFloat("u_Intensity", m_VignetteParams.intensity);
        m_VignetteShader->SetFloat("u_Smoothness", m_VignetteParams.smoothness);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        currentTexture = m_TempFramebuffer.GetColorTexture();
    }
    
    // Apply tone mapping (always last)
    if (m_ToneMappingEnabled) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_Width, m_Height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_ToneMappingShader->Bind();
        m_ToneMappingShader->SetInt("u_Texture", 0);
        m_ToneMappingShader->SetBool("u_UseACES", true);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    } else {
        // Just copy to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_Width, m_Height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_CopyShader->Bind();
        m_CopyShader->SetInt("u_Texture", 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void PostProcessManager::ApplyBloom(Framebuffer& input, Framebuffer& output) {
    // Pass 1: Extract bright pixels
    m_BrightFramebuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_BrightPassShader->Bind();
    m_BrightPassShader->SetInt("u_Texture", 0);
    m_BrightPassShader->SetFloat("u_Threshold", m_BloomParams.threshold);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input.GetColorTexture());
    
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Pass 2: Blur bright pixels (ping-pong)
    bool horizontal = true;
    m_BlurShader->Bind();
    m_BlurShader->SetInt("u_Texture", 0);
    m_BlurShader->SetFloat("u_BlurSize", m_BloomParams.blurSize);
    
    for (int i = 0; i < m_BloomParams.blurPasses; i++) {
        m_PingPongFramebuffers[horizontal].Bind();
        m_BlurShader->SetFloat2("u_Direction", 
            horizontal ? Float2{1.0f, 0.0f} : Float2{0.0f, 1.0f});
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 
            i == 0 ? m_BrightFramebuffer.GetColorTexture() : 
                     m_PingPongFramebuffers[!horizontal].GetColorTexture());
        
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        horizontal = !horizontal;
    }
    
    // Pass 3: Combine scene + bloom
    output.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_CombineShader->Bind();
    m_CombineShader->SetInt("u_Scene", 0);
    m_CombineShader->SetInt("u_Bloom", 1);
    m_CombineShader->SetFloat("u_BloomStrength", m_BloomParams.strength);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input.GetColorTexture());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_PingPongFramebuffers[!horizontal].GetColorTexture());
    
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PostProcessManager::SetEffectEnabled(const std::string& name, bool enabled) {
    if (name == "bloom") m_BloomEnabled = enabled;
    else if (name == "vignette") m_VignetteEnabled = enabled;
    else if (name == "chromatic_aberration") m_ChromaticAberrationEnabled = enabled;
    else if (name == "tone_mapping") m_ToneMappingEnabled = enabled;
}

bool PostProcessManager::IsEffectEnabled(const std::string& name) const {
    if (name == "bloom") return m_BloomEnabled;
    if (name == "vignette") return m_VignetteEnabled;
    if (name == "chromatic_aberration") return m_ChromaticAberrationEnabled;
    if (name == "tone_mapping") return m_ToneMappingEnabled;
    return false;
}

GLuint PostProcessManager::GetFinalTexture() const {
    return m_TempFramebuffer.GetColorTexture();
}

} // namespace SAGE
