#include "SAGE/Graphics/Texture.h"
#include "SAGE/Log.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <mutex>

namespace SAGE {

namespace {
    std::mutex s_StbImageMutex; // Protect global stb_image state

    GLenum FilterToGL(TextureFilter filter) {
        switch (filter) {
            case TextureFilter::Nearest: return GL_NEAREST;
            case TextureFilter::Linear:  return GL_LINEAR;
        }
        return GL_LINEAR;
    }

    GLenum WrapToGL(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat: return GL_REPEAT;
            case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
        }
        return GL_REPEAT;
    }
}

Texture::Texture(const std::string& path, const TextureSpec& spec)
    : m_Path(path), m_Spec(spec) {
    Load(path);
}

Texture::Texture(uint32_t width, uint32_t height, const void* data, const TextureSpec& spec)
    : m_Width(width), m_Height(height), m_Spec(spec) {
    m_Channels = 4; // Default to RGBA for data-created textures
    CreateFromData(data, spec);
}

Texture::~Texture() {
    Unload();
}

bool Texture::Load(const std::string& path) {
    if (path.empty()) {
        SAGE_ERROR("Texture::Load - Empty path provided");
        return false;
    }

    m_Path = path;
    
    int width, height, channels;
    unsigned char* data = nullptr;

    {
        std::lock_guard<std::mutex> lock(s_StbImageMutex);
        stbi_set_flip_vertically_on_load(m_Spec.flipVertically ? 1 : 0);
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    }

    if (!data) {
        SAGE_ERROR("Failed to load texture: {}", path);
        return false;
    }

    m_Width = static_cast<uint32_t>(width);
    m_Height = static_cast<uint32_t>(height);
    m_Channels = channels;

    CreateFromData(data, m_Spec);

    stbi_image_free(data);

    SAGE_INFO("Loaded texture: {} ({}x{}, {} channels)", path, m_Width, m_Height, m_Channels);
    return true;
}

void Texture::Unload() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
        m_Width = 0;
        m_Height = 0;
    }
}

void Texture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::shared_ptr<Texture> Texture::Create(const std::string& path, const TextureSpec& spec) {
    return std::make_shared<Texture>(path, spec);
}

std::shared_ptr<Texture> Texture::CreateWhiteTexture() {
    const uint32_t whitePixel = 0xFFFFFFFF;
    TextureSpec spec;
    spec.minFilter = TextureFilter::Nearest;
    spec.magFilter = TextureFilter::Nearest;
    spec.generateMipmaps = false;
    return std::make_shared<Texture>(1, 1, &whitePixel, spec);
}

void Texture::CreateFromData(const void* data, const TextureSpec& spec) {
    if (m_Width == 0 || m_Height == 0) {
        SAGE_ERROR("Texture::CreateFromData - Invalid dimensions: {}x{}", m_Width, m_Height);
        return;
    }

    if (!data) {
        SAGE_WARNING("Texture::CreateFromData - Null data pointer, creating empty texture");
    }

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    GLenum internalFormat = GL_RGBA;
    GLenum dataFormat = GL_RGBA;

    if (m_Channels == 3) {
        internalFormat = GL_RGB;
        dataFormat = GL_RGB;
    } else if (m_Channels == 4) {
        internalFormat = GL_RGBA;
        dataFormat = GL_RGBA;
    } else if (m_Channels == 1) {
        internalFormat = GL_RED;
        dataFormat = GL_RED;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                 static_cast<GLsizei>(m_Width), 
                 static_cast<GLsizei>(m_Height),
                 0, dataFormat, GL_UNSIGNED_BYTE, data);

    if (spec.generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        // Choose appropriate mipmap filter based on minFilter
        if (spec.minFilter == TextureFilter::Nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(spec.minFilter));
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(spec.magFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(spec.wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(spec.wrapT));

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFilter(TextureFilter min, TextureFilter mag) {
    m_Spec.minFilter = min;
    m_Spec.magFilter = mag;

    if (m_TextureID != 0) {
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        
        if (m_Spec.generateMipmaps) {
             if (min == TextureFilter::Nearest) {
                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
             } else {
                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
             }
        } else {
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(min));
        }
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(mag));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::SetWrap(TextureWrap s, TextureWrap t) {
    m_Spec.wrapS = s;
    m_Spec.wrapT = t;

    if (m_TextureID != 0) {
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(s));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(t));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

std::shared_ptr<Texture> Texture::CreateFromData(int width, int height, const void* data, const TextureSpec& spec) {
    auto texture = std::make_shared<Texture>();
    texture->m_Width = width;
    texture->m_Height = height;
    texture->m_Channels = 4; // RGBA by default
    texture->CreateFromData(data, spec);
    return texture;
}

} // namespace SAGE
