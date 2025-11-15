#include "Texture.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Graphics/Core/Utils/PNGLoader.h"
#include "Core/Logger.h"
#include "Graphics/Backend/Implementations/OpenGL/Utils/GLErrorScope.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef GL_LINEAR_MIPMAP_LINEAR
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#endif
#ifndef GL_NEAREST_MIPMAP_NEAREST
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#endif
#ifndef GL_NEAREST_MIPMAP_LINEAR
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#endif

typedef void (APIENTRYP PFN_GL_COMPRESSED_TEX_IMAGE_2D)(GLenum target,
                                                        GLint level,
                                                        GLenum internalFormat,
                                                        GLsizei width,
                                                        GLsizei height,
                                                        GLint border,
                                                        GLsizei imageSize,
                                                        const void* data);

typedef void (APIENTRYP PFN_GL_COMPRESSED_TEX_SUB_IMAGE_2D)(GLenum target,
                                                            GLint level,
                                                            GLint xoffset,
                                                            GLint yoffset,
                                                            GLsizei width,
                                                            GLsizei height,
                                                            GLenum format,
                                                            GLsizei imageSize,
                                                            const void* data);

typedef void (APIENTRYP PFN_GL_GENERATE_MIPMAP)(GLenum target);

static PFN_GL_COMPRESSED_TEX_IMAGE_2D s_CompressedTexImage2D = nullptr;
static PFN_GL_COMPRESSED_TEX_SUB_IMAGE_2D s_CompressedTexSubImage2D = nullptr;
static PFN_GL_GENERATE_MIPMAP s_GenerateMipmap = nullptr;

static void EnsureCompressionProcs() {
    if (!s_CompressedTexImage2D) {
        s_CompressedTexImage2D = reinterpret_cast<PFN_GL_COMPRESSED_TEX_IMAGE_2D>(glfwGetProcAddress("glCompressedTexImage2D"));
    }
    if (!s_CompressedTexSubImage2D) {
        s_CompressedTexSubImage2D = reinterpret_cast<PFN_GL_COMPRESSED_TEX_SUB_IMAGE_2D>(glfwGetProcAddress("glCompressedTexSubImage2D"));
    }
    if (!s_GenerateMipmap) {
        s_GenerateMipmap = reinterpret_cast<PFN_GL_GENERATE_MIPMAP>(glfwGetProcAddress("glGenerateMipmap"));
    }
}

#ifndef GL_UNPACK_ALIGNMENT
#define GL_UNPACK_ALIGNMENT 0x0CF5
#endif

#ifndef GL_TEXTURE_SWIZZLE_R
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#endif
#ifndef GL_TEXTURE_SWIZZLE_G
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#endif
#ifndef GL_TEXTURE_SWIZZLE_B
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#endif
#ifndef GL_TEXTURE_SWIZZLE_A
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#endif

#ifndef GL_RED
#define GL_RED 0x1903
#endif
#ifndef GL_R8
#define GL_R8 0x8229
#endif
#ifndef GL_RGB
#define GL_RGB 0x1907
#endif
#ifndef GL_RGB8
#define GL_RGB8 0x8051
#endif
#ifndef GL_RGBA
#define GL_RGBA 0x1908
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F 0x881A
#endif
#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif

namespace SAGE {

    namespace {

        struct FormatInfo {
            unsigned int InternalFormat = 0;
            unsigned int DataFormat = 0;
            unsigned int DataType = GL_UNSIGNED_BYTE;
            bool Compressed = false;
            bool Supported = true;
        };

        FormatInfo ResolveFormat(Texture::Format format) {
            FormatInfo info{};
            switch (format) {
            case Texture::Format::RGBA8:
                info.InternalFormat = GL_RGBA8;
                info.DataFormat = GL_RGBA;
                break;
            case Texture::Format::RGB8:
                info.InternalFormat = GL_RGB8;
                info.DataFormat = GL_RGB;
                break;
            case Texture::Format::Red8:
                info.InternalFormat = GL_R8;
                info.DataFormat = GL_RED;
                break;
            case Texture::Format::RGBA16F:
                info.InternalFormat = GL_RGBA16F;
                info.DataFormat = GL_RGBA;
                info.DataType = GL_HALF_FLOAT;
                break;
            case Texture::Format::BC1:
                info.InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                info.Compressed = true;
                break;
            case Texture::Format::BC3:
            case Texture::Format::BC5:
                info.InternalFormat = (format == Texture::Format::BC3)
                    ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
                    : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                info.Compressed = true;
                break;
            case Texture::Format::ASTC_4x4:
                info.InternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
                info.Compressed = true;
                break;
            case Texture::Format::ETC2_RGBA8:
                info.InternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
                info.Compressed = true;
                break;
            default:
                info.InternalFormat = GL_RGBA8;
                info.DataFormat = GL_RGBA;
                break;
            }
            return info;
        }

        std::size_t BytesPerPixel(Texture::Format format) {
            switch (format) {
            case Texture::Format::RGBA8:
                return 4;
            case Texture::Format::RGB8:
                return 3;
            case Texture::Format::Red8:
                return 1;
            case Texture::Format::RGBA16F:
                return 8;
            default:
                return 4;
            }
        }

    } // namespace

    bool Texture::SupportsCompression(Format format) {
        if (!IsCompressedFormat(format)) {
            return true;
        }
#ifdef __EMSCRIPTEN__
        return false;
#else
        return true;
#endif
    }

    bool Texture::IsCompressedFormat(Format format) {
        switch (format) {
        case Format::BC1:
        case Format::BC3:
        case Format::BC5:
        case Format::ASTC_4x4:
        case Format::ETC2_RGBA8:
            return true;
        default:
            return false;
        }
    }

    std::size_t Texture::BytesPerBlock(Format format) {
        switch (format) {
        case Format::BC1:
            return 8;
        case Format::BC3:
        case Format::BC5:
        case Format::ASTC_4x4:
        case Format::ETC2_RGBA8:
            return 16;
        default:
            return BytesPerPixel(format);
        }
    }

    unsigned int Texture::BlockWidth(Format format) {
        switch (format) {
        case Format::BC1:
        case Format::BC3:
        case Format::BC5:
        case Format::ASTC_4x4:
        case Format::ETC2_RGBA8:
            return 4;
        default:
            return 1;
        }
    }

    unsigned int Texture::BlockHeight(Format format) {
        return BlockWidth(format);
    }

    std::size_t Texture::CalculateDataFootprint(Format format,
                                                unsigned int width,
                                                unsigned int height,
                                                unsigned int mipLevels,
                                                bool compressed) {
        std::size_t totalBytes = 0;
        unsigned int w = width;
        unsigned int h = height;
        const unsigned int levels = std::max(1u, mipLevels);

        for (unsigned int level = 0; level < levels; ++level) {
            if (compressed) {
                const unsigned int bw = BlockWidth(format);
                const unsigned int bh = BlockHeight(format);
                const unsigned int blocksWide = std::max(1u, (w + bw - 1) / bw);
                const unsigned int blocksHigh = std::max(1u, (h + bh - 1) / bh);
                totalBytes += static_cast<std::size_t>(blocksWide) * blocksHigh * BytesPerBlock(format);
            } else {
                totalBytes += static_cast<std::size_t>(w) * h * BytesPerPixel(format);
            }

            w = std::max(1u, w / 2);
            h = std::max(1u, h / 2);
        }

        return totalBytes;
    }

    Texture::Texture(const std::string& path)
        : m_Path(path) {
        SAGE_INFO("[Texture] Loading '{}'", path);
        
        // Read file to memory
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            SAGE_ERROR("[Texture] Failed to open file '{}'", path);
            Allocate(1, 1, Format::RGBA8, nullptr, false);
            unsigned int color = 0xFFFFFFFFu;
            SetData(&color, sizeof(unsigned int));
            m_Loaded = true;
            m_State = ResourceState::Stub;
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            SAGE_ERROR("[Texture] Failed to read file '{}'", path);
            Allocate(1, 1, Format::RGBA8, nullptr, false);
            unsigned int color = 0xFFFFFFFFu;
            SetData(&color, sizeof(unsigned int));
            m_Loaded = true;
            m_State = ResourceState::Stub;
            return;
        }

#ifdef _WIN32
        // Use WIC decoder directly on Windows
        const auto decoded = Image::DecodeWithWIC(buffer.data(), buffer.size());
#else
        const auto decoded = Image::PNGImageDecoder::LoadFromFile(path);
#endif

        SAGE_INFO("[Texture] Decoded: valid={} width={} height={} pixelsSize={}", 
                  decoded.IsValid(), decoded.width, decoded.height, decoded.pixels.size());

        if (!decoded.IsValid()) {
            SAGE_ERROR("[Texture] Failed to decode texture '{}'", path);
            Allocate(1, 1, Format::RGBA8, nullptr, false);
            unsigned int color = 0xFFFFFFFFu;
            SetData(&color, sizeof(unsigned int));
            m_Loaded = true;
            m_State = ResourceState::Stub;
            return;
        }

        SAGE_INFO("[Texture] Calling Allocate...");
        Allocate(decoded.width,
                 decoded.height,
                 Format::RGBA8,
                 decoded.pixels.data(),
                 false,
                 false);

        SAGE_INFO("[Texture] Allocate complete, setting loaded state");
        m_Loaded = true;
        m_State = ResourceState::Loaded;
        SAGE_INFO("[Texture] Constructor complete for '{}'", path);
        SAGE_INFO("[Texture] Successfully loaded '{}' ({}x{})", m_Path, m_Width, m_Height);
    }

    Texture::Texture(unsigned int width,
                     unsigned int height,
                     Format format,
                     const unsigned char* data,
                     bool generateMipmaps)
        : m_Path(""), m_Loaded(true) {
        Allocate(width, height, format, data, generateMipmaps, IsCompressedFormat(format));
        m_State = ResourceState::Loaded;
    }

    Texture::Texture(const std::vector<MipLevelData>& mipChain,
                     Format format,
                     bool compressed)
        : m_Path(""), m_Loaded(true) {
        if (mipChain.empty()) {
            Allocate(1, 1, format, nullptr, false, compressed);
            m_State = ResourceState::Stub;
            return;
        }

        Allocate(mipChain.front().Width,
                 mipChain.front().Height,
                 format,
                 mipChain.front().Bytes.empty() ? nullptr : mipChain.front().Bytes.data(),
                 false,
                 compressed);

        if (mipChain.size() > 1) {
            UploadMipChain(mipChain, compressed);
        }
        m_State = ResourceState::Loaded;
    }

    Texture::Texture()
        : m_Path(""), m_Loaded(false), m_State(ResourceState::Unloaded) {
        // Empty texture for external wrapping (RenderTarget)
    }

    Texture::~Texture() {
        m_TextureHandle.Reset();
    }

    std::size_t Texture::GetGPUMemorySize() const {
        if (!m_Loaded || m_State == ResourceState::Stub || m_State == ResourceState::Unloaded) {
            return 0;
        }
        return CalculateDataFootprint(m_Format, m_Width, m_Height, m_MipLevels, m_IsCompressed);
    }

    bool Texture::Unload() noexcept {
        const bool wasLoaded = m_Loaded;
        if (m_TextureHandle) {
            m_TextureHandle.Reset();
        }
        m_Loaded = false;
        m_State = ResourceState::Unloaded;
        m_BindWarnEmitted = false;
        m_MipmapsGenerated = false;
        return wasLoaded;
    }

    bool Texture::Reload() {
        if (m_Path.empty()) {
            SAGE_WARNING("[Texture] Cannot reload texture with no file path");
            return false;
        }

        // Read file to memory
        std::ifstream file(m_Path, std::ios::binary | std::ios::ate);
        if (!file) {
            SAGE_ERROR("[Texture] Failed to open file for reload '{}'", m_Path);
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            SAGE_ERROR("[Texture] Failed to read file for reload '{}'", m_Path);
            return false;
        }

#ifdef _WIN32
        // Use WIC decoder directly on Windows
        const auto decoded = Image::DecodeWithWIC(buffer.data(), buffer.size());
#else
        const auto decoded = Image::PNGImageDecoder::LoadFromFile(m_Path);
#endif

        if (!decoded.IsValid()) {
            SAGE_ERROR("[Texture] Failed to reload texture '{}'", m_Path);
            return false;
        }

        // Re-allocate with new data
        Allocate(decoded.width,
                 decoded.height,
                 Format::RGBA8,
                 decoded.pixels.data(),
                 false,
                 false);

        m_Loaded = true;
        m_State = ResourceState::Loaded;
        m_MipmapsGenerated = false;

        SAGE_INFO("[Texture] Reloaded '{}'", m_Path);
        return true;
    }

    void Texture::Bind(unsigned int slot) const {
        if (!m_Loaded) {
            if (!m_BindWarnEmitted) {
                SAGE_WARNING("[Texture] Bind attempted on unloaded texture '{}'", m_Path);
                m_BindWarnEmitted = true;
            }
            return; // safe no-op
        }
        if (glad_glActiveTexture && glad_glBindTexture) {
            glActiveTexture(GL_TEXTURE0 + slot);
            {
                GLErrorScope scope("Texture::SetData-Bind");
                glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
            }
        }
    }

    void Texture::Unbind() const {
        if (glad_glBindTexture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void Texture::SetData(void* data, unsigned int size) {
        if (!m_TextureHandle) {
            SAGE_ERROR("[Texture] SetData called on uninitialized texture.");
            return;
        }

        const std::size_t expected = CalculateDataFootprint(m_Format, m_Width, m_Height, 1, m_IsCompressed);
        if (size != expected) {
            SAGE_ERROR("[Texture] SetData size mismatch ({} != {}).", size, expected);
            return;
        }

    if (glad_glBindTexture) {
        if (glad_glBindTexture) {
            glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
        }
    }

        if (m_IsCompressed) {
            EnsureCompressionProcs();
            if (!s_CompressedTexSubImage2D) {
                SAGE_ERROR("[Texture] Compressed texture updates are not supported on this platform.");
                return;
            }
            s_CompressedTexSubImage2D(GL_TEXTURE_2D,
                                      0,
                                      0,
                                      0,
                                      static_cast<GLsizei>(m_Width),
                                      static_cast<GLsizei>(m_Height),
                                      m_InternalFormat,
                                      static_cast<GLsizei>(size),
                                      data);
        } else {
            if (m_Format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }

            {
                GLErrorScope scope("Texture::SetData-SubImage");
                glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                0,
                                0,
                                static_cast<GLsizei>(m_Width),
                                static_cast<GLsizei>(m_Height),
                                m_DataFormat,
                                m_DataType,
                                data);
            }

            if (m_Format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }
    }

    void Texture::UploadMipChain(const std::vector<MipLevelData>& mipChain, bool compressed) {
        if (!m_TextureHandle || mipChain.empty()) {
            return;
        }

        {
            GLErrorScope scope("Texture::GenerateMipmaps-Bind");
            glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
        }
        const std::size_t levelCount = std::min<std::size_t>(mipChain.size(), static_cast<std::size_t>(32));
        for (std::size_t level = 0; level < levelCount; ++level) {
            UploadMipLevel(mipChain[level], static_cast<unsigned int>(level), compressed);
        }

        m_MipLevels = static_cast<unsigned int>(levelCount);
        ApplySamplerState();
    }

    void Texture::GenerateMipmaps() {
        if (!m_TextureHandle) {
            return;
        }

        {
            GLErrorScope scope("Texture::Allocate-Bind");
            glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
        }
        EnsureCompressionProcs();
        if (!s_GenerateMipmap) {
            SAGE_WARNING("[Texture] Unable to generate mipmaps on this platform.");
            return;
        }
        s_GenerateMipmap(GL_TEXTURE_2D);

        const unsigned int longestEdge = std::max(m_Width, m_Height);
        unsigned int size = longestEdge;
        m_MipLevels = 1;
        while (size > 1) {
            size = std::max(1u, size / 2);
            ++m_MipLevels;
        }

        ApplySamplerState();
    }

    void Texture::Allocate(unsigned int width,
                           unsigned int height,
                           Format format,
                           const unsigned char* data,
                           bool generateMipmaps,
                           bool compressed) {
        // Headless safeguard: if no valid GL context, skip GPU allocation.
        if (glfwGetCurrentContext() == nullptr) {
            m_Width = width;
            m_Height = height;
            m_Format = format;
            m_IsCompressed = compressed;
            m_GenerateMipmaps = false;
            m_MipLevels = 1;
            // Leave m_TextureHandle empty; treat as stub or unloaded depending on data.
            return;
        }
        // Validate compression flag consistency.
        const bool formatIsCompressed = IsCompressedFormat(format);
        if (compressed && !formatIsCompressed) {
            SAGE_WARNING("[Texture] 'compressed' flag set but format is not compressed. Forcing uncompressed RGBA8.");
            format = Format::RGBA8;
            compressed = false;
        }
        if (formatIsCompressed && !compressed) {
            // Allow silently enabling compressed path.
            compressed = true;
        }
        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_MipLevels = 1;
        m_IsCompressed = compressed;
        m_GenerateMipmaps = generateMipmaps && !compressed;

        if (!m_TextureHandle) {
            const std::string debugName = m_Path.empty() ? "Texture::Runtime" : m_Path;
            m_TextureHandle.Create(debugName);
        }

        glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
        ResolveFormats(format, compressed);

        const FormatInfo formatInfo = ResolveFormat(format);
        if (!formatInfo.Supported) {
            SAGE_WARNING("[Texture] Requested texture format not supported. Falling back to RGBA8.");
        }

        if (compressed) {
            EnsureCompressionProcs();
            if (!s_CompressedTexImage2D) {
                SAGE_ERROR("[Texture] Compressed textures are not supported on this platform.");
                return;
            }
            const std::size_t size = data
                ? CalculateDataFootprint(format, width, height, 1, true)
                : 0;
            if (s_CompressedTexImage2D) {
                s_CompressedTexImage2D(GL_TEXTURE_2D,
                                       0,
                                       m_InternalFormat,
                                       static_cast<GLsizei>(m_Width),
                                       static_cast<GLsizei>(m_Height),
                                       0,
                                       static_cast<GLsizei>(size),
                                       data);
            }
        } else {
            if (format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }

            {
                GLErrorScope scope("Texture::Allocate-Image2D");
                glTexImage2D(GL_TEXTURE_2D,
                             0,
                             static_cast<GLint>(m_InternalFormat),
                             static_cast<GLsizei>(m_Width),
                             static_cast<GLsizei>(m_Height),
                             0,
                             m_DataFormat,
                             m_DataType,
                             data);
            }

            if (format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }

        if (m_GenerateMipmaps && data) {
            EnsureCompressionProcs();
            if (!s_GenerateMipmap) {
                SAGE_WARNING("[Texture] Unable to generate mipmaps on this platform.");
                m_MipLevels = 1;
                ApplySamplerState();
                m_Loaded = (data != nullptr) || m_Path.empty();
                return;
            }
            s_GenerateMipmap(GL_TEXTURE_2D);
            const unsigned int longestEdge = std::max(m_Width, m_Height);
            unsigned int size = longestEdge;
            m_MipLevels = 1;
            while (size > 1) {
                size = std::max(1u, size / 2u);
                ++m_MipLevels;
            }
        } else {
            m_MipLevels = 1;
        }

        ApplySamplerState();
        m_Loaded = (data != nullptr) || m_Path.empty();
    }

    void Texture::UploadMipLevel(const MipLevelData& mip, unsigned int level, bool compressed) {
        if (compressed) {
            EnsureCompressionProcs();
            if (!s_CompressedTexImage2D) {
                SAGE_ERROR("[Texture] Compressed textures are not supported on this platform.");
                return;
            }
            s_CompressedTexImage2D(GL_TEXTURE_2D,
                                   static_cast<GLint>(level),
                                   m_InternalFormat,
                                   static_cast<GLsizei>(mip.Width),
                                   static_cast<GLsizei>(mip.Height),
                                   0,
                                   static_cast<GLsizei>(mip.Bytes.size()),
                                   mip.Bytes.empty() ? nullptr : mip.Bytes.data());
        } else {
            if (m_Format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }

            glTexImage2D(GL_TEXTURE_2D,
                         static_cast<GLint>(level),
                         static_cast<GLint>(m_InternalFormat),
                         static_cast<GLsizei>(mip.Width),
                         static_cast<GLsizei>(mip.Height),
                         0,
                         m_DataFormat,
                         m_DataType,
                         mip.Bytes.empty() ? nullptr : mip.Bytes.data());

            if (m_Format == Format::Red8) {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }
    }

    void Texture::ResolveFormats(Format format, bool compressed) {
        const FormatInfo info = ResolveFormat(format);
        m_InternalFormat = info.InternalFormat;
        m_DataFormat = info.DataFormat;
        m_DataType = info.DataType;
        m_IsCompressed = compressed || info.Compressed;
    }

    int Texture::ToGLMagFilter(FilterMode mode) {
        return mode == FilterMode::Nearest ? GL_NEAREST : GL_LINEAR;
    }

    int Texture::ToGLMinFilter(FilterMode mode, bool mipmapAware, unsigned int mipLevels) {
        const bool hasMipmaps = mipmapAware && mipLevels > 1;
        if (!hasMipmaps) {
            return mode == FilterMode::Nearest ? GL_NEAREST : GL_LINEAR;
        }
        return mode == FilterMode::Nearest ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
    }

    void Texture::ApplySamplerState() const {
        if (!m_TextureHandle || glfwGetCurrentContext() == nullptr) {
            return;
        }

        {
            GLErrorScope scope("Texture::ApplySamplerState-Bind");
            glBindTexture(GL_TEXTURE_2D, m_TextureHandle.Get());
        }

        const int glMinFilter = ToGLMinFilter(m_MinFilter, m_FilterUseMipmaps, m_MipLevels);
        const int glMagFilter = ToGLMagFilter(m_MagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (m_Format == Format::Red8) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        }
    }

    void Texture::SetFilter(FilterMode minFilter,
                             FilterMode magFilter,
                             bool enableMipmaps) {
        m_MinFilter = minFilter;
        m_MagFilter = magFilter;
        m_FilterUseMipmaps = enableMipmaps;

        if (!m_TextureHandle || glfwGetCurrentContext() == nullptr) {
            return;
        }

        ApplySamplerState();
    }

    void Texture::SetNativeTextureID(unsigned int textureID) {
        // Wrap an external OpenGL texture (e.g., from RenderTarget)
        // Note: We don't manage this texture's lifetime here
        m_TextureHandle.Adopt(textureID);
        m_Loaded = textureID != 0;
        m_State = textureID != 0 ? ResourceState::Loaded : ResourceState::Unloaded;
        m_MipmapsGenerated = false;
    }

} // namespace SAGE
