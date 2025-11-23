#include "OpenGLRenderBackend.h"

#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Texture.h"
#include "SAGE/Log.h"
#include "SAGE/Time.h"

#include <glad/glad.h>

#include <cmath>

// Force rebuild
namespace SAGE {

namespace {

} // namespace

void OpenGLRenderBackend::Initialize(const RendererConfig& config) {
    if (m_Initialized) {
        return;
    }

    m_Config = config;

    SAGE_INFO("Initializing OpenGL renderer backend");
    SAGE_INFO("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    SAGE_INFO("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    SAGE_INFO("Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    SAGE_INFO("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_BlendSrc = GL_SRC_ALPHA;
    m_BlendDst = GL_ONE_MINUS_SRC_ALPHA;

    // Try loading shaders from files
    m_DefaultShader = Shader::CreateFromFiles("Engine/shaders/Sprite.vert", "Engine/shaders/Sprite.frag");
    
    if (!m_DefaultShader) {
        SAGE_ERROR("Failed to load default shaders from files. Ensure Engine/shaders/Sprite.vert and Sprite.frag exist.");
        // Fallback to hardcoded if file load fails (optional, but good for stability during dev)
        // For now, we strictly follow the request to remove hardcode, so we just error out or return.
    }

    CreateQuadBuffers();
    CreateDynamicBuffers();
    m_SpriteRenderer.Init();

    m_View = Matrix3::Identity();
    m_ViewProjection = m_Projection * m_View;

    m_Initialized = true;
}

void OpenGLRenderBackend::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    SAGE_INFO("Shutting down OpenGL renderer backend");

    if (m_QuadVAO != 0) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        glDeleteBuffers(1, &m_QuadVBO);
        glDeleteBuffers(1, &m_QuadEBO);
        m_QuadVAO = 0;
        m_QuadVBO = 0;
        m_QuadEBO = 0;
    }

    if (m_DynamicVAO != 0) {
        glDeleteVertexArrays(1, &m_DynamicVAO);
        glDeleteBuffers(1, &m_DynamicVBO);
        m_DynamicVAO = 0;
        m_DynamicVBO = 0;
    }

    m_DefaultShader.reset();
    m_SpriteRenderer.Shutdown();
    m_Initialized = false;
}

void OpenGLRenderBackend::BeginFrame() {
    m_Stats.Reset();
}

void OpenGLRenderBackend::EndFrame() {
    // Reserved for backend-specific sync if needed
}

void OpenGLRenderBackend::Clear(const Color& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderBackend::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderBackend::SetScissor(int x, int y, int width, int height) {
    // Legacy support: Clear stack and push new one
    while (!m_ScissorStack.empty()) m_ScissorStack.pop();
    PushScissor(x, y, width, height);
}

void OpenGLRenderBackend::DisableScissor() {
    // Legacy support: Clear stack
    while (!m_ScissorStack.empty()) m_ScissorStack.pop();
    glDisable(GL_SCISSOR_TEST);
}

void OpenGLRenderBackend::PushScissor(int x, int y, int width, int height) {
    ScissorRect newRect = {x, y, width, height};
    
    if (!m_ScissorStack.empty()) {
        // Intersect with current top
        const ScissorRect& top = m_ScissorStack.top();
        int x1 = std::max(top.x, newRect.x);
        int y1 = std::max(top.y, newRect.y);
        int x2 = std::min(top.x + top.width, newRect.x + newRect.width);
        int y2 = std::min(top.y + top.height, newRect.y + newRect.height);
        
        newRect.x = x1;
        newRect.y = y1;
        newRect.width = std::max(0, x2 - x1);
        newRect.height = std::max(0, y2 - y1);
    }
    
    m_ScissorStack.push(newRect);
    UpdateScissor();
}

void OpenGLRenderBackend::PopScissor() {
    if (!m_ScissorStack.empty()) {
        m_ScissorStack.pop();
        UpdateScissor();
    }
}

void OpenGLRenderBackend::UpdateScissor() {
    if (m_ScissorStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        glEnable(GL_SCISSOR_TEST);
        const ScissorRect& rect = m_ScissorStack.top();
        
        // Get current viewport to know window height
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        int windowHeight = viewport[3];
        
        // Convert top-left y to bottom-left y
        int glY = windowHeight - (rect.y + rect.height);
        
        glScissor(rect.x, glY, rect.width, rect.height);
    }
}

void OpenGLRenderBackend::SetRenderMode(RenderMode mode) {
    m_RenderMode = mode;
    glPolygonMode(GL_FRONT_AND_BACK, mode == RenderMode::Wireframe ? GL_LINE : GL_FILL);
}

RenderMode OpenGLRenderBackend::GetRenderMode() const {
    return m_RenderMode;
}

void OpenGLRenderBackend::EnableBlending(bool enabled) {
    m_BlendingEnabled = enabled;
    if (enabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGLRenderBackend::SetBlendFunc(uint32_t srcFactor, uint32_t dstFactor) {
    m_BlendSrc = srcFactor;
    m_BlendDst = dstFactor;
    glBlendFunc(srcFactor, dstFactor);
}

void OpenGLRenderBackend::DrawQuad(const Vector2& position, const Vector2& size, const Color& color) {
    if (!m_Initialized) {
        return;
    }
    if (!m_DefaultShader) {
        SAGE_ERROR("Default shader not initialized");
        return;
    }

    const Vector2 offset = position - size * 0.5f;
    const Matrix3 transform = Matrix3::Translation(offset) * Matrix3::Scale(size);

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetVec4("uColor", color.r, color.g, color.b, color.a);
    m_DefaultShader->SetInt("uUseTexture", 0);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawQuad(const Vector2& position, const Vector2& size, Texture* texture) {
    if (!m_Initialized) {
        return;
    }
    if (!m_DefaultShader || !texture) {
        SAGE_ERROR("Shader or texture not initialized");
        return;
    }

    const Vector2 offset = position - size * 0.5f;
    const Matrix3 transform = Matrix3::Translation(offset) * Matrix3::Scale(size);

    texture->Bind(0);
    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetInt("uTexture", 0);
    m_DefaultShader->SetInt("uUseTexture", 1);
    m_DefaultShader->SetVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawQuadTinted(const Vector2& position, const Vector2& size, const Color& color, Texture* texture) {
    if (!m_Initialized) {
        return;
    }
    if (!m_DefaultShader || !texture) {
        SAGE_ERROR("Shader or texture not initialized");
        return;
    }

    const Vector2 offset = position - size * 0.5f;
    const Matrix3 transform = Matrix3::Translation(offset) * Matrix3::Scale(size);

    texture->Bind(0);
    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetInt("uTexture", 0);
    m_DefaultShader->SetInt("uUseTexture", 1);
    m_DefaultShader->SetVec4("uColor", color.r, color.g, color.b, color.a);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawQuad(const Vector2& position, const Vector2& size, const Color& color, Shader* shader) {
    if (!m_Initialized) {
        return;
    }
    if (!shader) {
        SAGE_ERROR("Custom shader is null");
        return;
    }

    const Vector2 offset = position - size * 0.5f;
    const Matrix3 transform = Matrix3::Translation(offset) * Matrix3::Scale(size);

    shader->Bind();
    shader->SetMat3("uProjection", m_ViewProjection.m.data());
    shader->SetMat3("uTransform", transform.m.data());
    shader->SetVec4("uColor", color.r, color.g, color.b, color.a);
    shader->SetFloat("uTime", static_cast<float>(Time::Elapsed()));
    
    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawLine(const Vector2& start, const Vector2& end, const Color& color, float thickness) {
    if (!m_Initialized || !m_DefaultShader) {
        return;
    }
    const Vector2 delta = end - start;
    const float length = delta.Length();
    const float angle = std::atan2(delta.y, delta.x);

    const Matrix3 transform =
        Matrix3::Translation(start) *
        Matrix3::Rotation(angle) *
        Matrix3::Translation({0.0f, -thickness * 0.5f}) *
        Matrix3::Scale({length, thickness});

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetVec4("uColor", color.r, color.g, color.b, color.a);
    m_DefaultShader->SetInt("uUseTexture", 0);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawSprite(const Sprite& sprite) {
    if (!m_Initialized || !sprite.visible || !sprite.GetTexture() || !m_DefaultShader) {
        return;
    }

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", sprite.transform.GetMatrix().m.data());
    m_DefaultShader->SetInt("uUseTexture", 1);
    m_DefaultShader->SetVec4("uColor", sprite.tint.r, sprite.tint.g, sprite.tint.b, sprite.tint.a);
    m_DefaultShader->SetFloat("uTime", static_cast<float>(Time::Elapsed()));

    float u = sprite.textureRect.x;
    float v = sprite.textureRect.y;
    float w = sprite.textureRect.width != 0.0f ? sprite.textureRect.width : 1.0f;
    float h = sprite.textureRect.height != 0.0f ? sprite.textureRect.height : 1.0f;

    // Check if projection is Y-up (Bottom-Left origin)
    // In Ortho(left, right, bottom, top), m[1][1] = 2 / (top - bottom)
    // If top > bottom (Y up), m[1][1] is positive.
    if (m_ViewProjection.m[4] > 0.0f) {
        v = v + h;
        h = -h;
    }

    m_DefaultShader->SetVec4("uTexRect", u, v, w, h);

    sprite.GetTexture()->Bind(0);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::DrawSprite(const Sprite& sprite, const Camera2D& camera) {
    if (!m_Initialized || !sprite.visible || !sprite.GetTexture() || !m_DefaultShader) {
        return;
    }

    m_DefaultShader->Bind();
    const Matrix3 viewProj = camera.GetViewProjectionMatrix();
    m_DefaultShader->SetMat3("uProjection", viewProj.m.data());
    m_DefaultShader->SetMat3("uTransform", sprite.transform.GetMatrix().m.data());
    m_DefaultShader->SetInt("uUseTexture", 1);
    m_DefaultShader->SetVec4("uColor", sprite.tint.r, sprite.tint.g, sprite.tint.b, sprite.tint.a);
    m_DefaultShader->SetFloat("uTime", static_cast<float>(Time::Elapsed()));

    float u = sprite.textureRect.x;
    float v = sprite.textureRect.y;
    float w = sprite.textureRect.width != 0.0f ? sprite.textureRect.width : 1.0f;
    float h = sprite.textureRect.height != 0.0f ? sprite.textureRect.height : 1.0f;

    // Check if projection is Y-up (Bottom-Left origin)
    if (viewProj.m[4] > 0.0f) {
        v = v + h;
        h = -h;
    }

    m_DefaultShader->SetVec4("uTexRect", u, v, w, h);

    sprite.GetTexture()->Bind(0);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::BeginSpriteBatch(const Camera2D* camera) {
    if (!m_Initialized) {
        return;
    }
    const Matrix3 projection = camera ? camera->GetViewProjectionMatrix() : m_ViewProjection;
    m_SpriteRenderer.Begin(projection);
}

void OpenGLRenderBackend::SubmitSprite(const Sprite& sprite) {
    if (!m_Initialized) {
        return;
    }
    m_SpriteRenderer.Submit(sprite);
}

void OpenGLRenderBackend::FlushSpriteBatch() {
    if (!m_Initialized) {
        return;
    }
    const auto batchStats = m_SpriteRenderer.Flush();
    m_Stats.drawCalls += batchStats.drawCalls;
    m_Stats.vertices += batchStats.vertices;
    m_Stats.triangles += batchStats.triangles;
}

void OpenGLRenderBackend::DrawParticle(const Vector2& position, float size, const Color& color, float rotation) {
    if (!m_Initialized || !m_DefaultShader) {
        return;
    }

    const Matrix3 transform =
        Matrix3::Translation(position) *
        Matrix3::Rotation(rotation) *
        Matrix3::Scale({size, size}) *
        Matrix3::Translation({-0.5f, -0.5f});

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetInt("uUseTexture", 0);
    m_DefaultShader->SetVec4("uColor", color.r, color.g, color.b, color.a);
    m_DefaultShader->SetFloat("uTime", static_cast<float>(Time::Elapsed()));
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_QuadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

void OpenGLRenderBackend::SetProjectionMatrix(const Matrix3& projection) {
    m_Projection = projection;
    m_ViewProjection = m_Projection * m_View;
}

void OpenGLRenderBackend::SetViewMatrix(const Matrix3& view) {
    m_View = view;
    m_ViewProjection = m_Projection * m_View;
}

void OpenGLRenderBackend::SetCamera(const Camera2D& camera) {
    m_Projection = camera.GetProjectionMatrix();
    m_View = camera.GetViewMatrix();
    m_ViewProjection = camera.GetViewProjectionMatrix();
}

const Matrix3& OpenGLRenderBackend::GetProjectionMatrix() const {
    return m_Projection;
}

const Matrix3& OpenGLRenderBackend::GetViewMatrix() const {
    return m_View;
}

Matrix3 OpenGLRenderBackend::GetViewProjectionMatrix() const {
    return m_ViewProjection;
}

const RenderStats& OpenGLRenderBackend::GetStats() const {
    return m_Stats;
}

void OpenGLRenderBackend::ResetStats() {
    m_Stats.Reset();
}

void OpenGLRenderBackend::CreateQuadBuffers() {
    if (m_QuadVAO != 0) {
        return;
    }

    float vertices[] = {
        // Pos      // Tex    // Color (White)
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glGenBuffers(1, &m_QuadEBO);

    glBindVertexArray(m_QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));

    glBindVertexArray(0);
}

void OpenGLRenderBackend::CreateDynamicBuffers() {
    if (m_DynamicVAO != 0) {
        return;
    }

    glGenVertexArrays(1, &m_DynamicVAO);
    glGenBuffers(1, &m_DynamicVBO);

    glBindVertexArray(m_DynamicVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
    
    // Allocate 64KB for dynamic geometry
    glBufferData(GL_ARRAY_BUFFER, 64 * 1024, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));

    glBindVertexArray(0);
}

void OpenGLRenderBackend::DrawTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color& color) {
    if (!m_Initialized || !m_DefaultShader) {
        return;
    }

    // x, y, u, v, r, g, b, a
    float vertices[] = {
        p1.x, p1.y, 0.0f, 0.0f, color.r, color.g, color.b, color.a,
        p2.x, p2.y, 0.0f, 0.0f, color.r, color.g, color.b, color.a,
        p3.x, p3.y, 0.0f, 0.0f, color.r, color.g, color.b, color.a
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", Matrix3::Identity().m.data());
    m_DefaultShader->SetVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f); // Use vertex color
    m_DefaultShader->SetInt("uUseTexture", 0);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_DynamicVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    m_Stats.drawCalls++;
    m_Stats.vertices += 3;
    m_Stats.triangles += 1;
}

void OpenGLRenderBackend::DrawCircle(const Vector2& center, float radius, const Color& color) {
    if (!m_Initialized || !m_DefaultShader) {
        return;
    }

    const int segments = 32;
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 8); // Center + segments + close (8 floats per vertex)

    // Center vertex
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    vertices.push_back(0.5f); // UV center
    vertices.push_back(0.5f);
    vertices.push_back(color.r);
    vertices.push_back(color.g);
    vertices.push_back(color.b);
    vertices.push_back(color.a);

    const float step = 2.0f * 3.14159265359f / static_cast<float>(segments);
    for (int i = 0; i <= segments; ++i) {
        float angle = i * step;
        float x = center.x + std::cos(angle) * radius;
        float y = center.y + std::sin(angle) * radius;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f); // UV
        vertices.push_back(0.0f);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        vertices.push_back(color.a);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
    // Ensure buffer is large enough, or reallocate if needed. 
    // For simplicity, we assume the initial size (which we'll set in CreateDynamicBuffers) is enough for a circle.
    // A circle with 32 segments has ~34 vertices * 8 floats * 4 bytes = ~1088 bytes.
    // We'll allocate a decent chunk in CreateDynamicBuffers.
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", Matrix3::Identity().m.data());
    m_DefaultShader->SetVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f); // Use vertex color
    m_DefaultShader->SetInt("uUseTexture", 0);
    m_DefaultShader->SetVec4("uTexRect", 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(m_DynamicVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    m_Stats.drawCalls++;
    m_Stats.vertices += segments + 2;
    m_Stats.triangles += segments;
}

void OpenGLRenderBackend::DrawQuadGradient(const Vector2& position, const Vector2& size, const Color& c1, const Color& c2, const Color& c3, const Color& c4) {
    if (!m_Initialized || !m_DefaultShader) return;

    const Vector2 offset = position - size * 0.5f;
    const Matrix3 transform = Matrix3::Translation(offset) * Matrix3::Scale(size);

    m_DefaultShader->Bind();
    m_DefaultShader->SetMat3("uProjection", m_ViewProjection.m.data());
    m_DefaultShader->SetMat3("uTransform", transform.m.data());
    m_DefaultShader->SetVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f);
    m_DefaultShader->SetInt("uUseTexture", 0);

    // Dynamic vertices with gradient colors
    float vertices[] = {
        // Pos      // Tex    // Color
        0.0f, 0.0f, 0.0f, 0.0f, c1.r, c1.g, c1.b, c1.a, // Top-Left
        1.0f, 0.0f, 1.0f, 0.0f, c2.r, c2.g, c2.b, c2.a, // Top-Right
        1.0f, 1.0f, 1.0f, 1.0f, c3.r, c3.g, c3.b, c3.a, // Bottom-Right
        0.0f, 1.0f, 0.0f, 1.0f, c4.r, c4.g, c4.b, c4.a  // Bottom-Left
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_DynamicVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindVertexArray(m_DynamicVAO);
    // Use the static EBO (indices are the same)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    m_Stats.drawCalls++;
    m_Stats.vertices += 4;
    m_Stats.triangles += 2;
}

} // namespace SAGE
