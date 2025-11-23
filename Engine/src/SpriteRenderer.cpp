#include "SAGE/Graphics/SpriteRenderer.h"
#include "SAGE/Graphics/Shader.h"
#include "SAGE/Log.h"

#include <glad/glad.h>

#include <algorithm>

namespace SAGE {

namespace {
    // Advanced vertex shader with proper matrix transformations
    constexpr const char* BatchVertexShader = R"(
        #version 330 core
        
        // Input vertex attributes
        layout (location = 0) in vec2 aPos;        // Position in world space
        layout (location = 1) in vec2 aTexCoord;   // Texture coordinates (UV)
        layout (location = 2) in vec4 aColor;      // Vertex color/tint
        
        // Output to fragment shader
        out vec2 vTexCoord;
        out vec4 vColor;
        out vec2 vWorldPos;  // For advanced effects
        
        // Uniforms
        uniform mat3 uProjection;  // View-Projection matrix (3x3 for 2D)
        
        void main() {
            // Transform position from world space to NDC (Normalized Device Coordinates)
            // Using homogeneous coordinates for 2D: (x, y, 1)
            vec3 worldPosHomogeneous = vec3(aPos, 1.0);
            vec3 projectedPos = uProjection * worldPosHomogeneous;
            
            // Convert to clip space (OpenGL expects vec4)
            // Z=0 for 2D, W=1 for orthographic projection
            gl_Position = vec4(projectedPos.xy, 0.0, 1.0);
            
            // Pass through texture coordinates and color
            vTexCoord = aTexCoord;
            vColor = aColor;
            vWorldPos = aPos;
        }
    )";

    // Advanced fragment shader with gamma correction and optional effects
    constexpr const char* BatchFragmentShader = R"(
        #version 330 core
        
        // Input from vertex shader
        in vec2 vTexCoord;
        in vec4 vColor;
        in vec2 vWorldPos;
        
        // Output color
        out vec4 FragColor;
        
        // Uniforms
        uniform sampler2D uTexture;
        
        // Optional: gamma correction (set to 2.2 for sRGB)
        const float GAMMA = 2.2;
        const float INV_GAMMA = 1.0 / GAMMA;
        
        // Gamma correction functions
        vec3 toLinear(vec3 sRGB) {
            return pow(sRGB, vec3(GAMMA));
        }
        
        vec3 toSRGB(vec3 linear) {
            return pow(linear, vec3(INV_GAMMA));
        }
        
        void main() {
            // Sample texture
            vec4 texColor = texture(uTexture, vTexCoord);
            
            // Multiply by vertex color (tint)
            vec4 finalColor = texColor * vColor;
            
            // Gamma correction for proper color blending
            // (optional, enable if textures look washed out)
            // finalColor.rgb = toLinear(finalColor.rgb);
            // finalColor.rgb = toSRGB(finalColor.rgb);
            
            // Discard fully transparent pixels (optional optimization)
            if (finalColor.a < 0.01) {
                discard;
            }
            
            FragColor = finalColor;
        }
    )";

    inline void DestroyBuffer(GLuint& handle, void(*deleter)(GLsizei, const GLuint*)) {
        if (handle != 0) {
            deleter(1, &handle);
            handle = 0;
        }
    }
}

namespace {
    constexpr uint32_t MaxSprites = 10000;
    constexpr uint32_t MaxVertices = MaxSprites * 4;
    constexpr uint32_t MaxIndices = MaxSprites * 6;
}

void SpriteRenderer::Init() {
    if (m_Initialized) {
        return;
    }

    m_Shader = Shader::Create(BatchVertexShader, BatchFragmentShader);
    EnsureGPUResources();

    // Pre-fill index buffer
    std::vector<uint32_t> indices(MaxIndices);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < MaxIndices; i += 6) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SAGE_INFO("SpriteRenderer initialized");
    m_Initialized = true;
}

void SpriteRenderer::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    DestroyBuffer(m_VAO, glDeleteVertexArrays);
    DestroyBuffer(m_VBO, glDeleteBuffers);
    DestroyBuffer(m_EBO, glDeleteBuffers);
    m_Shader.reset();

    m_Commands.clear();
    m_VertexBuffer.clear();
    m_IndexBuffer.clear();

    m_Initialized = false;
}

void SpriteRenderer::Begin(const Matrix3& projection) {
    if (!m_Initialized) {
        Init();
    }

    m_Projection = projection;
    m_Commands.clear();
}

void SpriteRenderer::Submit(const Sprite& sprite) {
    if (!sprite.visible) {
        return;
    }

    auto texture = sprite.GetTexture();
    if (!texture || !texture->IsLoaded()) {
        return;
    }

    SpriteCommand cmd{};
    cmd.tint = sprite.tint;
    cmd.uv = sprite.textureRect;
    
    // Check if projection is Y-up (Bottom-Left origin)
    // In Ortho(left, right, bottom, top), m[1][1] = 2 / (top - bottom)
    // If top > bottom (Y up), m[1][1] is positive.
    // Since texture is loaded upside-down (stb default), it matches Y-down (UI) naturally.
    // For Y-up (Game), we need to flip UVs.
    if (m_Projection.m[4] > 0.0f) {
        cmd.uv.y = cmd.uv.y + cmd.uv.height;
        cmd.uv.height = -cmd.uv.height;
    }

    cmd.origin = sprite.transform.origin;
    cmd.texture = texture;
    cmd.layer = sprite.layer;
    cmd.flipX = sprite.flipX;
    cmd.flipY = sprite.flipY;

    const float texWidth = static_cast<float>(texture->GetWidth());
    const float texHeight = static_cast<float>(texture->GetHeight());
    const float uvWidth = sprite.textureRect.width != 0.0f ? sprite.textureRect.width : 1.0f;
    const float uvHeight = sprite.textureRect.height != 0.0f ? sprite.textureRect.height : 1.0f;

    Vector2 baseSize{texWidth * uvWidth, texHeight * uvHeight};
    cmd.size = {
        baseSize.x * sprite.transform.scale.x,
        baseSize.y * sprite.transform.scale.y
    };

    cmd.transform = Matrix3::Translation(sprite.transform.position) *
                    Matrix3::Rotation(sprite.transform.rotation);

    m_Commands.emplace_back(std::move(cmd));
}

SpriteRenderer::BatchStats SpriteRenderer::Flush() {
    BatchStats totals{};

    if (!m_Initialized || m_Commands.empty()) {
        return totals;
    }

    std::sort(m_Commands.begin(), m_Commands.end(), [](const SpriteCommand& a, const SpriteCommand& b) {
        if (a.layer == b.layer) {
            return a.texture.get() < b.texture.get();
        }
        return a.layer < b.layer;
    });

    EnsureGPUResources();

    m_Shader->Bind();
    m_Shader->SetMat3("uProjection", m_Projection.m.data());
    m_Shader->SetInt("uTexture", 0);

    glBindVertexArray(m_VAO);

    size_t batchStart = 0;
    while (batchStart < m_Commands.size()) {
        if (batchStart >= m_Commands.size()) {
            break;
        }
        const auto* currentTexture = m_Commands[batchStart].texture.get();
        const int currentLayer = m_Commands[batchStart].layer;

        size_t batchEnd = batchStart + 1;
        while (batchEnd < m_Commands.size()) {
            const auto& candidate = m_Commands[batchEnd];
            if (candidate.layer != currentLayer || candidate.texture.get() != currentTexture) {
                break;
            }
            ++batchEnd;
        }

        const size_t spriteCount = batchEnd - batchStart;
        if (spriteCount == 0) {
            batchStart = batchEnd;
            continue;
        }
        
        // Split batch if it exceeds MaxSprites
        size_t currentBatchSize = std::min(spriteCount, static_cast<size_t>(MaxSprites));
        // We might need a loop here if one texture has > MaxSprites, but for now let's assume it fits or we just draw chunks.
        // Actually, let's just cap it.
        
        m_VertexBuffer.clear();
        // m_IndexBuffer.clear(); // No longer needed
        m_VertexBuffer.reserve(currentBatchSize * 4);

        for (size_t i = batchStart; i < batchStart + currentBatchSize; ++i) {
            const auto& cmd = m_Commands[i];

            const float width = cmd.size.x;
            const float height = cmd.size.y;
            // ... (rest of vertex generation)
            Vector2 originOffset{cmd.origin.x * width, cmd.origin.y * height};
            Vector2 corners[4] = {
                {0.0f, 0.0f},
                {width, 0.0f},
                {width, height},
                {0.0f, height}
            };

            Vector2 positions[4];
            for (int corner = 0; corner < 4; ++corner) {
                Vector2 local = corners[corner] - originOffset;
                positions[corner] = cmd.transform.TransformPoint(local);
            }

            float u0 = cmd.uv.x;
            float v0 = cmd.uv.y;
            float u1 = cmd.uv.x + cmd.uv.width;
            float v1 = cmd.uv.y + cmd.uv.height;

            if (cmd.flipX) {
                std::swap(u0, u1);
            }
            if (cmd.flipY) {
                std::swap(v0, v1);
            }

            Vector2 texCoords[4] = {
                {u0, v0},
                {u1, v0},
                {u1, v1},
                {u0, v1}
            };

            for (int vert = 0; vert < 4; ++vert) {
                m_VertexBuffer.push_back({positions[vert], texCoords[vert], cmd.tint});
            }
        }

        if (m_VertexBuffer.empty()) {
            batchStart += currentBatchSize;
            continue;
        }

        // Check if we have space in the buffer
        if (m_BufferOffset + currentBatchSize * 4 > MaxVertices) {
            // Orphan the buffer
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);
            m_BufferOffset = 0;
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, m_BufferOffset * sizeof(SpriteVertex), static_cast<GLsizeiptr>(m_VertexBuffer.size() * sizeof(SpriteVertex)), m_VertexBuffer.data());

        m_Commands[batchStart].texture->Bind(0);
        
        // Use DrawElementsBaseVertex to draw from the correct offset
        glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(currentBatchSize * 6), GL_UNSIGNED_INT, nullptr, static_cast<GLint>(m_BufferOffset));

        m_BufferOffset += static_cast<uint32_t>(currentBatchSize * 4);

        totals.drawCalls++;
        totals.vertices += static_cast<uint32_t>(m_VertexBuffer.size());
        totals.triangles += static_cast<uint32_t>(currentBatchSize * 2);

        batchStart += currentBatchSize;
    }

    glBindVertexArray(0);
    m_Commands.clear();
    // Reset buffer offset at end of frame? No, keep it for next frame (ring buffer across frames)
    // But we need to ensure we don't overflow if we don't reset.
    // The check `m_BufferOffset + ... > MaxVertices` handles it.
    return totals;
}

void SpriteRenderer::EnsureGPUResources() {
    if (m_VAO != 0 && m_VBO != 0 && m_EBO != 0) {
        return;
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    // EBO data is set in Init() or if lost
    // But if EnsureGPUResources is called after context loss, we might need to refill EBO.
    // For now, we assume Init handles EBO data.

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, texCoord)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, color)));

    glBindVertexArray(0);
}

} // namespace SAGE
