#include "OpenGLSceneResources.h"
#include "Graphics/Backend/Implementations/OpenGL/GLDebug.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace SAGE {
namespace Graphics {

OpenGLSceneResources::~OpenGLSceneResources() {
    Shutdown();
}

void OpenGLSceneResources::Initialize(int width, int height) {
    if (m_Initialized) {
        SAGE_WARNING("OpenGLSceneResources already initialized; shutting down first");
        Shutdown();
    }
    
    m_Width = width;
    m_Height = height;
    
    CreateSceneFBO(width, height);
    CreateBlurTextures(width, height);
    CreateFullscreenQuad();
    
    m_Initialized = true;
    SAGE_INFO("OpenGLSceneResources initialized ({}x{})", width, height);
}

void OpenGLSceneResources::Shutdown() {
    if (!m_Initialized) return;
    
    DestroySceneFBO();
    DestroyBlurTextures();
    DestroyFullscreenQuad();
    
    m_Initialized = false;
    m_Width = 0;
    m_Height = 0;
    
    SAGE_INFO("OpenGLSceneResources shut down");
}

bool OpenGLSceneResources::IsInitialized() const {
    return m_Initialized;
}

void OpenGLSceneResources::Resize(int width, int height) {
    if (width == m_Width && height == m_Height) return;
    
    m_Width = width;
    m_Height = height;
    
    // Recreate FBOs and textures with new dimensions
    DestroySceneFBO();
    DestroyBlurTextures();
    
    CreateSceneFBO(width, height);
    CreateBlurTextures(width, height);
    
    SAGE_INFO("OpenGLSceneResources resized to {}x{}", width, height);
}

uint32_t OpenGLSceneResources::GetSceneColorTexture() const {
    return m_SceneColorTexture;
}

uint32_t OpenGLSceneResources::GetSceneFramebuffer() const {
    return m_SceneFBO;
}

uint32_t OpenGLSceneResources::GetBlurTexture(int index) const {
    if (index < 0 || index >= static_cast<int>(m_BlurTextures.size())) {
        return 0;
    }
    return m_BlurTextures[index];
}

uint32_t OpenGLSceneResources::GetFullscreenQuadVAO() const {
    return m_FullscreenQuadVAO;
}

void OpenGLSceneResources::CreateSceneFBO(int width, int height) {
    // Create framebuffer
    SAGE_GL_CHECK(glGenFramebuffers(1, &m_SceneFBO));
    SAGE_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO));
    
    // Create color texture
    SAGE_GL_CHECK(glGenTextures(1, &m_SceneColorTexture));
    SAGE_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_SceneColorTexture));
    SAGE_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    
    // Attach color texture to framebuffer
    SAGE_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SceneColorTexture, 0));
    
    // Create depth renderbuffer (optional, for depth testing in scene)
    SAGE_GL_CHECK(glGenRenderbuffers(1, &m_SceneDepthRBO));
    SAGE_GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_SceneDepthRBO));
    SAGE_GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
    SAGE_GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_SceneDepthRBO));
    
    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        SAGE_ERROR("OpenGLSceneResources: Scene FBO incomplete (status: 0x{:X})", status);
    }
    
    SAGE_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLSceneResources::DestroySceneFBO() {
    if (m_SceneFBO) {
        glDeleteFramebuffers(1, &m_SceneFBO);
        m_SceneFBO = 0;
    }
    if (m_SceneColorTexture) {
        glDeleteTextures(1, &m_SceneColorTexture);
        m_SceneColorTexture = 0;
    }
    if (m_SceneDepthRBO) {
        glDeleteRenderbuffers(1, &m_SceneDepthRBO);
        m_SceneDepthRBO = 0;
    }
}

void OpenGLSceneResources::CreateBlurTextures(int width, int height) {
    // Create 2 textures for ping-pong blur (horizontal/vertical passes)
    m_BlurTextures.resize(2);
    m_BlurFBOs.resize(2);
    
    for (int i = 0; i < 2; ++i) {
        // Create texture
        SAGE_GL_CHECK(glGenTextures(1, &m_BlurTextures[i]));
        SAGE_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_BlurTextures[i]));
        SAGE_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width / 2, height / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        SAGE_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
        // Create FBO for this blur texture
        SAGE_GL_CHECK(glGenFramebuffers(1, &m_BlurFBOs[i]));
        SAGE_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBOs[i]));
        SAGE_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BlurTextures[i], 0));
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            SAGE_ERROR("OpenGLSceneResources: Blur FBO {} incomplete (status: 0x{:X})", i, status);
        }
    }
    
    SAGE_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLSceneResources::DestroyBlurTextures() {
    if (!m_BlurTextures.empty()) {
        glDeleteTextures(static_cast<GLsizei>(m_BlurTextures.size()), m_BlurTextures.data());
        m_BlurTextures.clear();
    }
    if (!m_BlurFBOs.empty()) {
        glDeleteFramebuffers(static_cast<GLsizei>(m_BlurFBOs.size()), m_BlurFBOs.data());
        m_BlurFBOs.clear();
    }
}

void OpenGLSceneResources::CreateFullscreenQuad() {
    // Fullscreen quad vertices (position + texcoord)
    float quadVertices[] = {
        // pos (x,y)   texcoord (u,v)
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    
    SAGE_GL_CHECK(glGenVertexArrays(1, &m_FullscreenQuadVAO));
    SAGE_GL_CHECK(glGenBuffers(1, &m_FullscreenQuadVBO));
    
    SAGE_GL_CHECK(glBindVertexArray(m_FullscreenQuadVAO));
    SAGE_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenQuadVBO));
    SAGE_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW));
    
    // Position attribute (location 0)
    SAGE_GL_CHECK(glEnableVertexAttribArray(0));
    SAGE_GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
    
    // Texcoord attribute (location 1)
    SAGE_GL_CHECK(glEnableVertexAttribArray(1));
    SAGE_GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
    
    SAGE_GL_CHECK(glBindVertexArray(0));
}

void OpenGLSceneResources::DestroyFullscreenQuad() {
    if (m_FullscreenQuadVAO) {
        glDeleteVertexArrays(1, &m_FullscreenQuadVAO);
        m_FullscreenQuadVAO = 0;
    }
    if (m_FullscreenQuadVBO) {
        glDeleteBuffers(1, &m_FullscreenQuadVBO);
        m_FullscreenQuadVBO = 0;
    }
}

} // namespace Graphics
} // namespace SAGE
