#pragma once

#include <string>

namespace SAGE {

class GraphicsResourceManager;

/// Resource kind enumeration for tracking
enum class ResourceKind {
    Texture,
    Buffer,
    VertexArray,
    Framebuffer,
    Renderbuffer,
    ShaderProgram
};

/// Template-based tracked OpenGL handle
/// Automatically increments/decrements resource counters in GraphicsResourceManager
/// RAII-style ownership with move semantics
template<ResourceKind Kind>
class TrackedHandle {
public:
    TrackedHandle() = default;
    explicit TrackedHandle(const char* debugName);
    explicit TrackedHandle(const std::string& debugName);
    ~TrackedHandle();

    // Non-copyable
    TrackedHandle(const TrackedHandle&) = delete;
    TrackedHandle& operator=(const TrackedHandle&) = delete;

    // Movable
    TrackedHandle(TrackedHandle&& other) noexcept;
    TrackedHandle& operator=(TrackedHandle&& other) noexcept;

    // Create new GL handle
    void Create(const char* debugName = nullptr);
    void Create(const std::string& debugName);

    // Adopt an existing GL handle without generating a new one
    // Optionally take ownership so the handle gets destroyed on Reset
    void Adopt(unsigned int handle, const char* debugName = nullptr, bool takeOwnership = false);

    // Release without destroying GL object (transfer ownership)
    [[nodiscard]] unsigned int Release();

    // Destroy GL handle and reset
    void Reset();

    // Accessors
    [[nodiscard]] unsigned int Get() const { return m_Handle; }
    [[nodiscard]] const std::string& GetDebugName() const { return m_DebugName; }
    
    explicit operator bool() const { return m_Handle != 0; }

private:
    unsigned int m_Handle = 0;
    std::string m_DebugName;
    bool m_OwnsHandle = true;

    void CreateGLHandle();
    void DestroyGLHandle();
    
    friend class GraphicsResourceManager;
};

// Type aliases for convenience
using TrackedTextureHandle = TrackedHandle<ResourceKind::Texture>;
using TrackedBufferHandle = TrackedHandle<ResourceKind::Buffer>;
using TrackedVertexArrayHandle = TrackedHandle<ResourceKind::VertexArray>;
using TrackedFramebufferHandle = TrackedHandle<ResourceKind::Framebuffer>;
using TrackedRenderbufferHandle = TrackedHandle<ResourceKind::Renderbuffer>;
using TrackedShaderProgramHandle = TrackedHandle<ResourceKind::ShaderProgram>;

} // namespace SAGE

// Include implementation
#include "TrackedHandle.inl"
