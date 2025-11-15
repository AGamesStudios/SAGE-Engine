#pragma once

#include "CommandBuffer.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Graphics/Core/Camera2D.h"  // Include Camera2D class
#include "Math/Matrix4.h"

#include <functional>
#include <vector>

namespace SAGE::Graphics::Batching {

struct BatchKey;

struct QuadVertex {
    float position[3];
    float color[4];
    float texCoord[2];
    float pulse[2];
};

struct FlushContext {
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;
    float totalTime = 0.0f;
    const Camera2D* camera = nullptr;
    Vector2 cameraShakeOffset = Vector2::Zero();
    const Matrix4* projection = nullptr;
    const Matrix4* view = nullptr;
    const Matrix4* viewProjection = nullptr;
    const Matrix4* screenProjection = nullptr;
    size_t* drawCallCounter = nullptr;
    size_t* vertexCounter = nullptr;
    int textureSlotBase = 0;
};

class BatchRenderer {
public:
    using FlushDelegate = std::function<bool()>;

    static constexpr std::size_t kDefaultMaxQuads = 50000; // Increased from 20000

    void Initialize(std::size_t maxQuads = kDefaultMaxQuads, bool allowDynamicResize = false);
    void Shutdown();

    void BeginFrame();

    bool QueueQuad(const QuadCommand& command, const FlushDelegate& flushDelegate);
    // Queues text by expanding into glyph quads. Returns the number of glyph quads actually queued.
    // Returns 0 if nothing was queued (empty text, unloaded font/atlas, or failure after flush attempt).
    std::size_t QueueText(const TextCommand& command, const FlushDelegate& flushDelegate);

    bool Flush(const FlushContext& context);

    [[nodiscard]] bool HasPendingCommands() const;
    [[nodiscard]] std::size_t GetPendingCommandCount() const;
    [[nodiscard]] float GetLastFlushDurationMs() const { return m_LastFlushDurationMs; }
    [[nodiscard]] bool WasLastFlushSuccessful() const { return m_LastFlushSuccessful; }
    [[nodiscard]] std::size_t GetMaxQuads() const { return m_MaxQuads; }

    static constexpr std::size_t kVerticesPerQuad = 4;
    static constexpr std::size_t kIndicesPerQuad = 6;

private:

    CommandBuffer m_CommandBuffer;
    GraphicsResourceManager::TrackedVertexArrayHandle m_QuadVAO;
    GraphicsResourceManager::TrackedBufferHandle m_QuadVBO;
    GraphicsResourceManager::TrackedBufferHandle m_QuadEBO;
    std::vector<uint32_t> m_IndexCache;

    bool m_Initialized = false;
    bool m_LastFlushSuccessful = true;
    float m_LastFlushDurationMs = 0.0f;
    std::size_t m_MaxQuads = 0;
    bool m_AllowResize = false;

    [[nodiscard]] std::size_t MaxVertices() const { return m_MaxQuads * kVerticesPerQuad; }
    [[nodiscard]] std::size_t MaxIndices() const { return m_MaxQuads * kIndicesPerQuad; }
    void BuildIndexCache(std::size_t quadCount);
    bool ReallocateBuffers(std::size_t newMaxQuads);
    bool EnsureCapacityFor(std::size_t requiredQuads);

    bool FlushInternal(const FlushContext& context);
};

} // namespace SAGE::Graphics::Batching
