#pragma once

#include "Graphics/Core/Types/GraphicsTypes.h"
#include <glad/glad.h>

namespace SAGE::Graphics {

/// @brief Convert backend-neutral TextureFormat to OpenGL internal format
inline GLenum ToGLInternalFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:        return GL_RGBA8;
        case TextureFormat::RGB8:         return GL_RGB8;
        case TextureFormat::Red8:         return GL_R8;
        case TextureFormat::RGBA16F:      return GL_RGBA16F;
        case TextureFormat::RGBA32F:      return GL_RGBA32F;
        case TextureFormat::Depth24:      return GL_DEPTH_COMPONENT24;
        case TextureFormat::Depth32F:     return GL_DEPTH_COMPONENT32F;
        case TextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        case TextureFormat::Depth32FStencil8: return GL_DEPTH32F_STENCIL8;
        
        // Compressed formats
        case TextureFormat::BC1:          return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case TextureFormat::BC3:          return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case TextureFormat::BC5:          return GL_COMPRESSED_RG_RGTC2;
        case TextureFormat::ASTC_4x4:     return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        case TextureFormat::ETC2_RGBA8:   return GL_COMPRESSED_RGBA8_ETC2_EAC;
        
        default:                          return GL_RGBA8;
    }
}

/// @brief Convert backend-neutral TextureFormat to OpenGL format
inline GLenum ToGLFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return GL_RGBA;
            
        case TextureFormat::RGB8:
            return GL_RGB;
            
        case TextureFormat::Red8:
            return GL_RED;
            
        case TextureFormat::Depth24:
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT;
            
        case TextureFormat::Depth24Stencil8:
        case TextureFormat::Depth32FStencil8:
            return GL_DEPTH_STENCIL;
            
        // Compressed formats don't have a separate format parameter
        default:
            return GL_RGBA;
    }
}

/// @brief Convert backend-neutral TextureFormat to OpenGL type
inline GLenum ToGLType(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:
        case TextureFormat::RGB8:
        case TextureFormat::Red8:
            return GL_UNSIGNED_BYTE;
            
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
        case TextureFormat::Depth32F:
        case TextureFormat::Depth32FStencil8:
            return GL_FLOAT;
            
        case TextureFormat::Depth24:
        case TextureFormat::Depth24Stencil8:
            return GL_UNSIGNED_INT_24_8;
            
        default:
            return GL_UNSIGNED_BYTE;
    }
}

/// @brief Check if format is a compressed format
inline bool IsCompressedFormat(TextureFormat format) {
    return format == TextureFormat::BC1 ||
           format == TextureFormat::BC3 ||
           format == TextureFormat::BC5 ||
           format == TextureFormat::ASTC_4x4 ||
           format == TextureFormat::ETC2_RGBA8;
}

/// @brief Convert backend-neutral TextureFilter to OpenGL filter mode
inline GLenum ToGLFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest:              return GL_NEAREST;
        case TextureFilter::Linear:               return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
        default:                                  return GL_LINEAR;
    }
}

/// @brief Convert backend-neutral TextureWrap to OpenGL wrap mode
inline GLenum ToGLWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:         return GL_REPEAT;
        case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        default:                          return GL_CLAMP_TO_EDGE;
    }
}

/// @brief Convert backend-neutral BufferUsage to OpenGL usage hint
inline GLenum ToGLUsage(BufferUsage usage) {
    switch (usage) {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        default:                   return GL_STATIC_DRAW;
    }
}

/// @brief Convert backend-neutral PrimitiveTopology to OpenGL primitive type
inline GLenum ToGLPrimitive(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::Points:        return GL_POINTS;
        case PrimitiveTopology::Lines:         return GL_LINES;
        case PrimitiveTopology::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveTopology::Triangles:     return GL_TRIANGLES;
        case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:   return GL_TRIANGLE_FAN;
        default:                               return GL_TRIANGLES;
    }
}

/// @brief Convert backend-neutral IndexFormat to OpenGL type
inline GLenum ToGLIndexType(IndexFormat format) {
    switch (format) {
        case IndexFormat::UInt16: return GL_UNSIGNED_SHORT;
        case IndexFormat::UInt32: return GL_UNSIGNED_INT;
        default:                  return GL_UNSIGNED_INT;
    }
}

} // namespace SAGE::Graphics
