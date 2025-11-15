#pragma once

#include <cstdint>

namespace SAGE::Graphics {

/// @brief Opaque handle for GPU textures (backend-agnostic)
using TextureHandle = uint64_t;
constexpr TextureHandle InvalidTextureHandle = 0;

/// @brief Opaque handle for framebuffers (backend-agnostic)
using FramebufferHandle = uint64_t;
constexpr FramebufferHandle InvalidFramebufferHandle = 0;

/// @brief Opaque handle for shader programs (backend-agnostic)
using ShaderHandle = uint64_t;
constexpr ShaderHandle InvalidShaderHandle = 0;

/// @brief Opaque handle for vertex buffers (backend-agnostic)
using BufferHandle = uint64_t;
constexpr BufferHandle InvalidBufferHandle = 0;

/// @brief Backend-neutral texture format
enum class TextureFormat {
    Unknown = 0,
    
    // Standard formats
    RGBA8,
    RGB8,
    Red8,
    
    // HDR formats
    RGBA16F,
    RGBA32F,
    
    // Compressed formats
    BC1,        // DXT1
    BC3,        // DXT5
    BC5,        // 2-channel compressed
    ASTC_4x4,   // Mobile
    ETC2_RGBA8, // Mobile
    
    // Depth/Stencil
    Depth24,
    Depth32F,
    Depth24Stencil8,
    Depth32FStencil8
};

/// @brief Backend-neutral framebuffer attachment type
enum class FramebufferAttachment {
    Color0 = 0,
    Color1,
    Color2,
    Color3,
    Depth,
    Stencil,
    DepthStencil
};

/// @brief Backend-neutral texture filter mode
enum class TextureFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

/// @brief Backend-neutral texture wrap mode
enum class TextureWrap {
    Repeat,
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat
};

/// @brief Backend-neutral buffer usage hint
enum class BufferUsage {
    Static,  // Written once, read many times
    Dynamic, // Updated frequently
    Stream   // Written once per frame
};

/// @brief Backend-neutral buffer type
enum class BufferType {
    Vertex,
    Index,
    Uniform,
    Storage
};

/// @brief Backend-neutral primitive topology
enum class PrimitiveTopology {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};

/// @brief Backend-neutral index format
enum class IndexFormat {
    UInt16,
    UInt32
};

/// @brief Texture creation descriptor
struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;        // For 3D textures
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;  // For texture arrays
    uint32_t samples = 1;      // For MSAA
    
    TextureFormat format = TextureFormat::RGBA8;
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureWrap wrapU = TextureWrap::ClampToEdge;
    TextureWrap wrapV = TextureWrap::ClampToEdge;
    TextureWrap wrapW = TextureWrap::ClampToEdge;
    
    bool generateMipmaps = false;
    bool isRenderTarget = false;
    
    const void* initialData = nullptr;
    size_t initialDataSize = 0;
};

/// @brief Framebuffer creation descriptor
struct FramebufferDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t samples = 1; // For MSAA
    
    struct Attachment {
        FramebufferAttachment type = FramebufferAttachment::Color0;
        TextureFormat format = TextureFormat::RGBA8;
        TextureHandle existingTexture = InvalidTextureHandle; // Optional: use existing texture
    };
    
    static constexpr size_t MaxAttachments = 8;
    Attachment attachments[MaxAttachments];
    size_t attachmentCount = 0;
};

/// @brief Buffer creation descriptor
struct BufferDesc {
    size_t size = 0;
    BufferType type = BufferType::Vertex;
    BufferUsage usage = BufferUsage::Static;
    const void* initialData = nullptr;
};

} // namespace SAGE::Graphics
