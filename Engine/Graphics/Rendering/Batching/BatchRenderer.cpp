#include "BatchRenderer.h"

#include "Graphics/Core/Resources/Material.h"
#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/Backend/Implementations/OpenGL/Utils/GLErrorScope.h"
#include "Core/Logger.h"
#include "Core/UTF8Utils.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

#include <glad/glad.h>

namespace SAGE::Graphics::Batching {

struct InternalBatchKey {
    Ref<Material> material;
    Ref<Texture> texture;
    BlendMode blendMode = BlendMode::Alpha;
    DepthSettings depthState{};
    bool screenSpace = false;
    float layer = 0.0f;

    bool operator==(const InternalBatchKey& other) const {
        return material == other.material && texture == other.texture &&
               blendMode == other.blendMode && screenSpace == other.screenSpace &&
               depthState.testEnabled == other.depthState.testEnabled &&
               depthState.writeEnabled == other.depthState.writeEnabled &&
               depthState.function == other.depthState.function &&
               depthState.biasConstant == other.depthState.biasConstant &&
               depthState.biasSlope == other.depthState.biasSlope &&
               std::abs(layer - other.layer) <= 0.00001f;
    }
};

namespace {

float NormalizeLayer(float layer) {
    constexpr float kLayerDepthScale = 0.001f;
    float clamped = std::clamp(layer, -1000.0f, 1000.0f);
    return std::clamp(-clamped * kLayerDepthScale, -1.0f, 1.0f);
}

bool CheckForGLError(const char* context) {
#if defined(SAGE_GL_DEBUG)
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SAGE_ERROR("OpenGL error 0x{0:X} in {1}", static_cast<unsigned int>(error), context);
        return false;
    }
#endif
    (void)context;
    return true;
}

void BuildQuadVertices(const QuadCommand& command,
                       const FlushContext& /*context*/,
                       std::vector<QuadVertex>& outVertices) {
    // Emit vertices in model/screen space; GPU applies transforms via u_ViewProjection.
    const float layerDepth = NormalizeLayer(command.layer);
    const float z = -layerDepth; // negative Z is closer with default ortho settings

    Ref<Material> material = command.material;
    Color tint = material ? material->GetTint() : Color::White();
    Color finalColor(command.color.r * tint.r,
                     command.color.g * tint.g,
                     command.color.b * tint.b,
                     command.color.a * tint.a);

    float amplitude = command.effect.pulseAmplitude;
    float frequency = command.effect.pulseFrequency;
    if (material) {
        if (amplitude <= 0.0f) amplitude = material->GetPulseAmplitude();
        if (frequency <= 0.0f) frequency = material->GetPulseFrequency();
    }
    amplitude = std::clamp(amplitude, 0.0f, 1.0f);
    frequency = std::max(frequency, 0.0f);

    constexpr float kUvEpsilon = 1e-6f;
    if (std::abs(command.uvMax.x - command.uvMin.x) <= kUvEpsilon ||
        std::abs(command.uvMax.y - command.uvMin.y) <= kUvEpsilon) {
        // Degenerate UV rectangle – nothing to draw.
        return;
    }

    const float left = command.position.x;
    const float right = command.position.x + command.size.x;
    const float top = command.position.y;
    const float bottom = command.position.y + command.size.y;

    // Calculate rotation if needed
    const float centerX = command.position.x + command.size.x * 0.5f;
    const float centerY = command.position.y + command.size.y * 0.5f;
    constexpr float kDegreesToRadians = 3.14159265358979323846f / 180.0f;
    const float rotRad = command.rotation * kDegreesToRadians;
    const float cosR = std::cos(rotRad);
    const float sinR = std::sin(rotRad);
    const bool hasRotation = std::abs(command.rotation) > 0.001f;

    auto pushVertex = [&](float px, float py, float u, float v) {
        if (hasRotation) {
            const float dx = px - centerX;
            const float dy = py - centerY;
            px = centerX + dx * cosR - dy * sinR;
            py = centerY + dx * sinR + dy * cosR;
        }

        QuadVertex vertex{};
        vertex.position[0] = px;
        vertex.position[1] = py;
        vertex.position[2] = z;
        vertex.color[0] = finalColor.r;
        vertex.color[1] = finalColor.g;
        vertex.color[2] = finalColor.b;
        vertex.color[3] = finalColor.a;
        vertex.texCoord[0] = u;
        vertex.texCoord[1] = v;
        vertex.pulse[0] = amplitude;
        vertex.pulse[1] = frequency;
        outVertices.emplace_back(vertex);
    };

    // Bottom-left, bottom-right, top-right, top-left
    pushVertex(left,  bottom, command.uvMin.x, command.uvMax.y);
    pushVertex(right, bottom, command.uvMax.x, command.uvMax.y);
    pushVertex(right, top,    command.uvMax.x, command.uvMin.y);
    pushVertex(left,  top,    command.uvMin.x, command.uvMin.y);
}

bool FlushBatch(const InternalBatchKey& key,
                const std::vector<QuadVertex>& vertices,
                std::size_t quadCount,
                int textureMode,
                int hasTexture,
                size_t* drawCallCounter,
                size_t* vertexCounter,
                float totalTime,
                const FlushContext& context,
                const GraphicsResourceManager::TrackedVertexArrayHandle& vao,
                const GraphicsResourceManager::TrackedBufferHandle& vbo) {
    if (quadCount == 0 || !key.material) {
        if (!key.material && quadCount > 0) {
            SAGE_WARNING("BatchRenderer::FlushBatch skipping draw: material null (quadCount={}, texture={})", (unsigned)quadCount, key.texture && key.texture->IsLoaded() ? "yes" : "no");
        }
        return true;
    }

    auto shader = key.material->GetShader();
    if (!shader) {
        SAGE_WARNING("BatchRenderer::FlushBatch skipped draw because material \"{}\" has no shader", key.material->GetName());
        return true;
    }
    if (!shader->IsValid()) {
        SAGE_WARNING("BatchRenderer::FlushBatch skipped draw because shader for material \"{}\" is invalid", key.material->GetName());
        return true;
    }
    shader->Bind();

    const Matrix4 identity = Matrix4::Identity();

    const Matrix4* projectionMatrix = nullptr;
    const Matrix4* viewMatrix = nullptr;
    if (key.screenSpace) {
        // Screen-space quads still honour per-quad rotation locally; view stays identity.
        projectionMatrix = context.screenProjection ? context.screenProjection : &identity;
        viewMatrix = &identity;
    }
    else {
        projectionMatrix = context.projection ? context.projection : &identity;
        viewMatrix = context.view ? context.view : &identity;
    }

    shader->SetMat4IfExists("u_Projection", projectionMatrix->Data());
    shader->SetMat4IfExists("u_View", viewMatrix->Data());
    shader->SetFloatIfExists("u_Time", totalTime);
    shader->SetIntIfExists("u_HasTexture", hasTexture);
    shader->SetIntIfExists("u_TextureMode", textureMode);

    const int textureSlot = context.textureSlotBase;
    shader->SetIntIfExists("u_Texture", textureSlot);

    if (hasTexture == 1 && key.texture && key.texture->IsLoaded()) {
        key.texture->Bind(static_cast<unsigned int>(textureSlot));
    }
    else {
        // FIXED: Bind white 1x1 texture instead of unbinding (undefined behavior)
        // Note: This requires white texture to be created in OpenGLSceneRenderer::Init
        // For now, bind slot 0 to prevent undefined behavior
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, 0);
        // TODO: Use shared default white texture once it's available in renderer context
    }

    {
        GLErrorScope errorScope("BatchRenderer::FlushBatch");
        
        glBindVertexArray(vao.Get());
        glBindBuffer(GL_ARRAY_BUFFER, vbo.Get());

        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        static_cast<GLsizeiptr>(vertices.size() * sizeof(QuadVertex)),
                        vertices.data());

        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(quadCount * BatchRenderer::kIndicesPerQuad),
                       GL_UNSIGNED_INT,
                       nullptr);
    }

    const bool drawOk = CheckForGLError("BatchRenderer::FlushBatch glDrawElements");

    if (drawOk) {
        if (drawCallCounter) {
            (*drawCallCounter)++;
        }
        if (vertexCounter) {
            (*vertexCounter) += quadCount * BatchRenderer::kVerticesPerQuad;
        }
    }

    glBindVertexArray(0);
    return drawOk;
}

} // namespace

void BatchRenderer::Initialize(std::size_t maxQuads, bool allowDynamicResize) {
    if (m_Initialized) {
        return;
    }

    m_AllowResize = allowDynamicResize;
    m_MaxQuads = std::max<std::size_t>(1, maxQuads);

    m_CommandBuffer.Initialize(m_MaxQuads);
    BuildIndexCache(m_MaxQuads);

    m_QuadVAO.Create("BatchRenderer::Quad VAO");
    glBindVertexArray(m_QuadVAO.Get());

    m_QuadVBO.Create("BatchRenderer::Quad VBO");
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO.Get());
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(MaxVertices() * sizeof(QuadVertex)),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    m_QuadEBO.Create("BatchRenderer::Quad EBO");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO.Get());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(MaxIndices() * sizeof(uint32_t)),
                 m_IndexCache.data(),
                 GL_STATIC_DRAW);

    constexpr GLsizei stride = static_cast<GLsizei>(sizeof(QuadVertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, color)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, texCoord)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(QuadVertex, pulse)));

    glBindVertexArray(0);

    m_LastFlushSuccessful = true;
    m_LastFlushDurationMs = 0.0f;
    m_Initialized = true;
}

void BatchRenderer::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    m_QuadEBO.Reset();
    m_QuadVBO.Reset();
    m_QuadVAO.Reset();
    m_CommandBuffer.Clear();
    m_CommandBuffer.SetMaxQuads(0);
    m_IndexCache.clear();
    m_MaxQuads = 0;
    m_AllowResize = false;
    m_Initialized = false;
}

void BatchRenderer::BeginFrame() {
    m_CommandBuffer.Clear();
    m_LastFlushSuccessful = true;
    m_LastFlushDurationMs = 0.0f;
}

bool BatchRenderer::QueueQuad(const QuadCommand& command, const FlushDelegate& flushDelegate) {
    if (m_CommandBuffer.PushQuad(command)) {
        return true;
    }

    if (EnsureCapacityFor(m_CommandBuffer.Size() + 1) && m_CommandBuffer.PushQuad(command)) {
        return true;
    }

    if (!flushDelegate || !flushDelegate()) {
        return false;
    }

    EnsureCapacityFor(m_CommandBuffer.Size() + 1);
    return m_CommandBuffer.PushQuad(command);
}

std::size_t BatchRenderer::QueueText(const TextCommand& command, const FlushDelegate& flushDelegate) {
    if (command.text.empty() || !command.font || !command.font->IsLoaded()) {
        return 0; // nothing queued
    }

    const Ref<Texture> atlas = command.font->GetAtlasTexture();
    if (!atlas || !atlas->IsLoaded()) {
        return 0; // nothing queued
    }

    std::string_view textView(command.text);
    size_t utf8Offset = 0;
    uint32_t codepoint = 0;

    float cursorX = command.position.x;
    float baseline = command.position.y + command.font->GetAscent() * command.scale;
    if (command.screenSpace) {
        baseline = std::round(baseline);
    }
    uint32_t previousCodepoint = 0;
    bool hasPrevious = false;
    std::size_t glyphQuadsQueued = 0;

    while (Core::UTF8Utils::NextCodePoint(textView, utf8Offset, codepoint)) {
        if (codepoint == static_cast<uint32_t>('\n')) {
            cursorX = command.position.x;
            baseline += command.font->GetLineHeight() * command.scale;
            if (command.screenSpace) {
                baseline = std::round(baseline);
            }
            hasPrevious = false;
            previousCodepoint = 0;
            continue;
        }

        if (hasPrevious) {
            cursorX += command.font->GetKerning(previousCodepoint, codepoint) * command.scale;
        }

        const Glyph& glyph = command.font->GetGlyph(codepoint);
        const float x0 = cursorX + glyph.bearing.x * command.scale;
        const float y0 = baseline + glyph.bearing.y * command.scale;
        const float x1 = cursorX + glyph.extent.x * command.scale;
        const float y1 = baseline + glyph.extent.y * command.scale;

        Float2 glyphPosition(x0, y0);
        Float2 glyphSize(x1 - x0, y1 - y0);
        if (command.screenSpace) {
            glyphPosition.x = std::round(glyphPosition.x);
            glyphPosition.y = std::round(glyphPosition.y);
            glyphSize.x = std::round(glyphPosition.x + glyphSize.x) - glyphPosition.x;
            glyphSize.y = std::round(glyphPosition.y + glyphSize.y) - glyphPosition.y;
        }

        if (glyphSize.x > 0.0f && glyphSize.y > 0.0f) {
            QuadCommand glyphCommand;
            glyphCommand.position = glyphPosition;
            glyphCommand.size = glyphSize;
            glyphCommand.uvMin = glyph.uvMin;
            glyphCommand.uvMax = glyph.uvMax;
            glyphCommand.color = command.color;
            glyphCommand.texture = atlas;
            glyphCommand.material = command.material;
            glyphCommand.effect = command.effect;
            glyphCommand.layer = command.layer;
            glyphCommand.screenSpace = command.screenSpace;
            glyphCommand.blendMode = command.blendMode;
            glyphCommand.depthState = command.depthState;
            glyphCommand.materialId = command.materialId;

            if (!QueueQuad(glyphCommand, flushDelegate)) {
                // failure; return what we queued so far (non-zero) or 0 if none; caller can treat < full as error if needed
                return glyphQuadsQueued;
            }
            ++glyphQuadsQueued;
        }

        cursorX += glyph.advance * command.scale;
        previousCodepoint = codepoint;
        hasPrevious = true;
    }

    return glyphQuadsQueued;
}

bool BatchRenderer::Flush(const FlushContext& context) {
    m_LastFlushSuccessful = FlushInternal(context);
    if (m_LastFlushSuccessful) {
        m_CommandBuffer.Clear();
    }
    return m_LastFlushSuccessful;
}

bool BatchRenderer::HasPendingCommands() const {
    return !m_CommandBuffer.Empty();
}

std::size_t BatchRenderer::GetPendingCommandCount() const {
    return m_CommandBuffer.Size();
}

bool BatchRenderer::FlushInternal(const FlushContext& context) {
    if (m_CommandBuffer.Empty()) {
        m_LastFlushDurationMs = 0.0f;
        return true;
    }

    if (m_MaxQuads == 0) {
        SAGE_ERROR("BatchRenderer::FlushInternal called with zero capacity");
        return false;
    }

    const auto flushStart = std::chrono::high_resolution_clock::now();

    auto& commands = m_CommandBuffer.GetQuadsMutable();
    std::stable_sort(commands.begin(), commands.end(), [](const QuadCommand& a, const QuadCommand& b) {
        if (std::abs(a.layer - b.layer) > 0.00001f) {
            return a.layer < b.layer;
        }
        std::uint32_t aMat = a.material ? a.material->GetId() : 0;
        std::uint32_t bMat = b.material ? b.material->GetId() : 0;
        if (aMat != bMat) {
            return aMat < bMat;
        }
        if (a.blendMode != b.blendMode) {
            return static_cast<int>(a.blendMode) < static_cast<int>(b.blendMode);
        }
        if (a.depthState.testEnabled != b.depthState.testEnabled) {
            return static_cast<int>(a.depthState.testEnabled) < static_cast<int>(b.depthState.testEnabled);
        }
        if (a.depthState.writeEnabled != b.depthState.writeEnabled) {
            return static_cast<int>(a.depthState.writeEnabled) < static_cast<int>(b.depthState.writeEnabled);
        }
        if (a.depthState.function != b.depthState.function) {
            return static_cast<int>(a.depthState.function) < static_cast<int>(b.depthState.function);
        }
        if (a.depthState.biasConstant != b.depthState.biasConstant) {
            return a.depthState.biasConstant < b.depthState.biasConstant;
        }
        if (a.depthState.biasSlope != b.depthState.biasSlope) {
            return a.depthState.biasSlope < b.depthState.biasSlope;
        }
        if (a.screenSpace != b.screenSpace) {
            return !a.screenSpace && b.screenSpace;
        }
        const Texture* aTex = a.texture ? a.texture.get() : nullptr;
        const Texture* bTex = b.texture ? b.texture.get() : nullptr;
        if (aTex != bTex) {
            const unsigned int aId = aTex ? aTex->GetRendererID() : 0;
            const unsigned int bId = bTex ? bTex->GetRendererID() : 0;
            if (aId != bId) {
                return aId < bId;
            }
        }
        return aTex < bTex;
    });

    std::vector<QuadVertex> vertexBuffer;
    if (MaxVertices() > 0) {
        vertexBuffer.reserve(std::min(commands.size() * kVerticesPerQuad, MaxVertices()));
    }

    InternalBatchKey currentKey{};
    bool hasKey = false;
    std::size_t quadCount = 0;
    int textureMode = 0;
    int hasTexture = 0;
    bool flushSucceeded = true;

    auto flushBatch = [&]() {
        if (!hasKey || quadCount == 0) {
            vertexBuffer.clear();
            quadCount = 0;
            return;
        }
        if (!FlushBatch(currentKey,
                        vertexBuffer,
                        quadCount,
                        textureMode,
                        hasTexture,
                        context.drawCallCounter,
                        context.vertexCounter,
                        context.totalTime,
                        context,
                        m_QuadVAO,
                        m_QuadVBO)) {
            flushSucceeded = false;
        }
        vertexBuffer.clear();
        quadCount = 0;
    };

    for (const QuadCommand& command : commands) {
        if (!flushSucceeded) {
            break;
        }

        InternalBatchKey key{
            command.material,
            command.texture,
            command.blendMode,
            command.depthState,
            command.screenSpace,
            command.layer
        };

        if (!hasKey || !(key == currentKey) || quadCount >= m_MaxQuads) {
            flushBatch();
            if (!flushSucceeded) {
                break;
            }
            currentKey = key;
            hasKey = true;
            if (command.texture && command.texture->IsLoaded()) {
                hasTexture = 1;
                textureMode = command.texture->GetFormat() == Texture::Format::Red8 ? 1 : 0;
            }
            else {
                hasTexture = 0;
                textureMode = 0;
            }
        }

        BuildQuadVertices(command,
                          context,
                          vertexBuffer);
        ++quadCount;
    }

    if (flushSucceeded) {
        flushBatch();
    }

    const auto flushEnd = std::chrono::high_resolution_clock::now();
    m_LastFlushDurationMs = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(flushEnd - flushStart).count();

    if (!flushSucceeded) {
        const char* materialName = (hasKey && currentKey.material) ? currentKey.material->GetName().c_str() : "<null>";
        const int blendValue = hasKey ? static_cast<int>(currentKey.blendMode) : 0;
        const bool depthTest = hasKey ? currentKey.depthState.testEnabled : false;
        const bool depthWrite = hasKey ? currentKey.depthState.writeEnabled : false;
        SAGE_ERROR("BatchRenderer::Flush aborted after processing {0} commands (material={1}, blend={2}, depthTest={3}, depthWrite={4})",
                   commands.size(),
                   materialName,
                   blendValue,
                   depthTest,
                   depthWrite);
    }

    return flushSucceeded;
}

void BatchRenderer::BuildIndexCache(std::size_t quadCount) {
    m_IndexCache.resize(quadCount * kIndicesPerQuad);
    for (std::size_t i = 0; i < quadCount; ++i) {
        const std::size_t indexOffset = i * kIndicesPerQuad;
        const std::size_t vertexOffset = i * kVerticesPerQuad;
        m_IndexCache[indexOffset + 0] = static_cast<uint32_t>(vertexOffset + 0);
        m_IndexCache[indexOffset + 1] = static_cast<uint32_t>(vertexOffset + 1);
        m_IndexCache[indexOffset + 2] = static_cast<uint32_t>(vertexOffset + 2);
        m_IndexCache[indexOffset + 3] = static_cast<uint32_t>(vertexOffset + 2);
        m_IndexCache[indexOffset + 4] = static_cast<uint32_t>(vertexOffset + 3);
        m_IndexCache[indexOffset + 5] = static_cast<uint32_t>(vertexOffset + 0);
    }
}

bool BatchRenderer::ReallocateBuffers(std::size_t newMaxQuads) {
    if (newMaxQuads == 0) {
        return false;
    }

    BuildIndexCache(newMaxQuads);
    m_CommandBuffer.SetMaxQuads(newMaxQuads);

    if (m_QuadVAO) {
        glBindVertexArray(m_QuadVAO.Get());
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO.Get());
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(newMaxQuads * kVerticesPerQuad * sizeof(QuadVertex)),
                     nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO.Get());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(newMaxQuads * kIndicesPerQuad * sizeof(uint32_t)),
                     m_IndexCache.data(),
                     GL_STATIC_DRAW);

        glBindVertexArray(0);

        if (!CheckForGLError("BatchRenderer::ReallocateBuffers")) {
            return false;
        }
    }

    m_MaxQuads = newMaxQuads;
    return true;
}

bool BatchRenderer::EnsureCapacityFor(std::size_t requiredQuads) {
    if (requiredQuads <= m_MaxQuads) {
        return true;
    }

    if (!m_AllowResize) {
        return false;
    }

    // Growth strategy with hysteresis: grow in 50% increments minimum, align to 1000 quad boundaries
    std::size_t newMax = m_MaxQuads + std::max<std::size_t>(m_MaxQuads / 2, 1000);
    newMax = std::max(newMax, requiredQuads);
    
    // Align to 1000 quad boundary for cleaner allocations
    newMax = ((newMax + 999) / 1000) * 1000;

    if (!ReallocateBuffers(newMax)) {
        return false;
    }

    SAGE_INFO("BatchRenderer capacity increased: {} -> {} quads (+{:.1f}%)", 
              m_MaxQuads, newMax, 100.0f * (newMax - m_MaxQuads) / m_MaxQuads);
    return true;
}

} // namespace SAGE::Graphics::Batching
