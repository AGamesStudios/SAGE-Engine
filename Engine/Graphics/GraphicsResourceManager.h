#pragma once

#include "Memory/Ref.h"
#include "Graphics/Core/Handles/TrackedHandle.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace SAGE {

class Texture;
class Shader;
class Font;

class GraphicsResourceManager {
public:
    static void Init();
    static void Shutdown();

    static void TrackTexture(const Ref<Texture>& texture);
    static void TrackShader(const Ref<Shader>& shader);
    static void TrackFont(const Ref<Font>& font);

    [[nodiscard]] static std::size_t ActiveTextureCount();
    [[nodiscard]] static std::size_t ActiveBufferCount();
    [[nodiscard]] static std::size_t ActiveVertexArrayCount();
    [[nodiscard]] static std::size_t ActiveFramebufferCount();
    [[nodiscard]] static std::size_t ActiveRenderbufferCount();
    [[nodiscard]] static std::size_t ActiveShaderProgramCount();

    [[nodiscard]] static std::size_t TotalTrackedHandleCount();
    [[nodiscard]] static bool ValidateNoLeaks();

    // For TrackedHandle template friends
    static void Increment(ResourceKind kind);
    static void Decrement(ResourceKind kind);

private:
    template<ResourceKind Kind>
    friend class TrackedHandle;

    static void EnsureInitialized();

public:
    // Type aliases for tracked handles
    using TrackedTextureHandle = TrackedHandle<ResourceKind::Texture>;
    using TrackedBufferHandle = TrackedHandle<ResourceKind::Buffer>;
    using TrackedVertexArrayHandle = TrackedHandle<ResourceKind::VertexArray>;
    using TrackedFramebufferHandle = TrackedHandle<ResourceKind::Framebuffer>;
    using TrackedRenderbufferHandle = TrackedHandle<ResourceKind::Renderbuffer>;
    using TrackedShaderProgramHandle = TrackedHandle<ResourceKind::ShaderProgram>;
};

} // namespace SAGE

