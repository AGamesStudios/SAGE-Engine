#pragma once

#include "TrackedHandle.h"
#include "Graphics/GraphicsResourceManager.h"

#include <glad/glad.h>

namespace {

inline void LabelTrackedHandle(GLenum identifier, GLuint handle, const std::string& name)
{
    if (handle == 0 || name.empty()) {
        return;
    }

    if (glObjectLabel) {
        glObjectLabel(identifier, handle, static_cast<GLsizei>(name.size()), name.c_str());
    }
}

} // namespace

namespace SAGE {

template<ResourceKind Kind>
inline TrackedHandle<Kind>::TrackedHandle(const char* debugName)
    : m_Handle(0), m_DebugName(debugName ? debugName : "")
{
    Create(debugName);
}

template<ResourceKind Kind>
inline TrackedHandle<Kind>::TrackedHandle(const std::string& debugName)
    : m_Handle(0), m_DebugName(debugName)
{
    Create(debugName.c_str());
}

template<ResourceKind Kind>
inline TrackedHandle<Kind>::~TrackedHandle()
{
    Reset();
}

template<ResourceKind Kind>
inline TrackedHandle<Kind>::TrackedHandle(TrackedHandle&& other) noexcept
    : m_Handle(other.m_Handle), m_DebugName(std::move(other.m_DebugName))
{
    other.m_Handle = 0;
}

template<ResourceKind Kind>
inline TrackedHandle<Kind>& TrackedHandle<Kind>::operator=(TrackedHandle&& other) noexcept
{
    if (this != &other) {
        Reset();
        m_Handle = other.m_Handle;
        m_DebugName = std::move(other.m_DebugName);
        other.m_Handle = 0;
    }
    return *this;
}

template<ResourceKind Kind>
inline void TrackedHandle<Kind>::Create(const char* debugName)
{
    Reset();
    m_OwnsHandle = true;
    if (debugName) {
        m_DebugName = debugName;
    }
    CreateGLHandle();
    GraphicsResourceManager::Increment(Kind);
}

template<ResourceKind Kind>
inline void TrackedHandle<Kind>::Create(const std::string& debugName)
{
    Create(debugName.c_str());
}

template<ResourceKind Kind>
inline void TrackedHandle<Kind>::Adopt(unsigned int handle, const char* debugName, bool takeOwnership)
{
    Reset();
    m_Handle = handle;
    m_OwnsHandle = takeOwnership;
    m_DebugName = debugName ? debugName : "";

    if (m_Handle != 0) {
        GraphicsResourceManager::Increment(Kind);
        if (debugName && *debugName) {
            if constexpr (Kind == ResourceKind::Texture) {
                LabelTrackedHandle(GL_TEXTURE, m_Handle, m_DebugName);
            } else if constexpr (Kind == ResourceKind::Buffer) {
                LabelTrackedHandle(GL_BUFFER, m_Handle, m_DebugName);
            } else if constexpr (Kind == ResourceKind::VertexArray) {
                LabelTrackedHandle(GL_VERTEX_ARRAY, m_Handle, m_DebugName);
            } else if constexpr (Kind == ResourceKind::Framebuffer) {
                LabelTrackedHandle(GL_FRAMEBUFFER, m_Handle, m_DebugName);
            } else if constexpr (Kind == ResourceKind::Renderbuffer) {
                LabelTrackedHandle(GL_RENDERBUFFER, m_Handle, m_DebugName);
            } else if constexpr (Kind == ResourceKind::ShaderProgram) {
                LabelTrackedHandle(GL_PROGRAM, m_Handle, m_DebugName);
            }
        }
    }
}

template<ResourceKind Kind>
inline unsigned int TrackedHandle<Kind>::Release()
{
    unsigned int handle = m_Handle;
    if (m_Handle != 0) {
        GraphicsResourceManager::Decrement(Kind);
        m_Handle = 0;
        m_DebugName.clear();
        m_OwnsHandle = true;
    }
    return handle;
}

template<ResourceKind Kind>
inline void TrackedHandle<Kind>::Reset()
{
    if (m_Handle != 0) {
        if (m_OwnsHandle) {
            DestroyGLHandle();
        }
        GraphicsResourceManager::Decrement(Kind);
        m_Handle = 0;
        m_DebugName.clear();
        m_OwnsHandle = true;
    }
}

// GL-specific creation/destruction
template<ResourceKind Kind>
inline void TrackedHandle<Kind>::CreateGLHandle()
{
    if constexpr (Kind == ResourceKind::Texture) {
        glGenTextures(1, &m_Handle);
        LabelTrackedHandle(GL_TEXTURE, m_Handle, m_DebugName);
    } else if constexpr (Kind == ResourceKind::Buffer) {
        glGenBuffers(1, &m_Handle);
        LabelTrackedHandle(GL_BUFFER, m_Handle, m_DebugName);
    } else if constexpr (Kind == ResourceKind::VertexArray) {
        glGenVertexArrays(1, &m_Handle);
        LabelTrackedHandle(GL_VERTEX_ARRAY, m_Handle, m_DebugName);
    } else if constexpr (Kind == ResourceKind::Framebuffer) {
        glGenFramebuffers(1, &m_Handle);
        LabelTrackedHandle(GL_FRAMEBUFFER, m_Handle, m_DebugName);
    } else if constexpr (Kind == ResourceKind::Renderbuffer) {
        glGenRenderbuffers(1, &m_Handle);
        LabelTrackedHandle(GL_RENDERBUFFER, m_Handle, m_DebugName);
    } else if constexpr (Kind == ResourceKind::ShaderProgram) {
        m_Handle = glCreateProgram();
        LabelTrackedHandle(GL_PROGRAM, m_Handle, m_DebugName);
    }
}

template<ResourceKind Kind>
inline void TrackedHandle<Kind>::DestroyGLHandle()
{
    if (m_Handle == 0) return;
    
    if constexpr (Kind == ResourceKind::Texture) {
        glDeleteTextures(1, &m_Handle);
    } else if constexpr (Kind == ResourceKind::Buffer) {
        glDeleteBuffers(1, &m_Handle);
    } else if constexpr (Kind == ResourceKind::VertexArray) {
        glDeleteVertexArrays(1, &m_Handle);
    } else if constexpr (Kind == ResourceKind::Framebuffer) {
        glDeleteFramebuffers(1, &m_Handle);
    } else if constexpr (Kind == ResourceKind::Renderbuffer) {
        glDeleteRenderbuffers(1, &m_Handle);
    } else if constexpr (Kind == ResourceKind::ShaderProgram) {
        glDeleteProgram(m_Handle);
    }
}

} // namespace SAGE
