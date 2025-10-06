#include "Texture.h"
#include "../Core/Logger.h"

#include <glad/glad.h>
#include <stb_image.h>

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

namespace SAGE {

    namespace {
        void SetDefaultParameters(Texture::Format format) {
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            if (format == Texture::Format::Red) {
                ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
                ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
                ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
                ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
            }
        }
    }

    Texture::Texture(const std::string& path)
        : m_Path(path) {
        stbi_set_flip_vertically_on_load(true);

        int width = 0, height = 0, channels = 0;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!data) {
            SAGE_ERROR("Не удалось загрузить текстуру: {}", path);
            Allocate(1, 1, Format::RGBA, nullptr);
            unsigned int color = 0xFFFFFFFFu;
            SetData(&color, sizeof(unsigned int));
            return;
        }

        Allocate(static_cast<unsigned int>(width), static_cast<unsigned int>(height), Format::RGBA, data);
        stbi_image_free(data);
        m_Loaded = true;
    }

    Texture::Texture(unsigned int width, unsigned int height, Format format, const unsigned char* data)
        : m_Path(""), m_Loaded(true) {
        Allocate(width, height, format, data);
    }

    Texture::~Texture() {
        if (m_RendererID)
            glDeleteTextures(1, &m_RendererID);
    }

    void Texture::Bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }

    void Texture::Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::SetData(void* data, unsigned int size) {
        if (!m_RendererID) {
            SAGE_ERROR("Texture::SetData вызван до создания текстуры");
            return;
        }

        unsigned int expected = m_Width * m_Height * (m_Format == Format::RGBA ? 4u : 1u);
        if (size != expected) {
            SAGE_ERROR("Размер данных ({}) не совпадает с размером текстуры ({}).", size, expected);
            return;
        }

        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        if (m_Format == Format::Red)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        ::glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

        if (m_Format == Format::Red)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    void Texture::Allocate(unsigned int width, unsigned int height, Format format, const unsigned char* data) {
        m_Width = width;
        m_Height = height;
        m_Format = format;

        if (!m_RendererID)
            ::glGenTextures(1, &m_RendererID);

        ::glBindTexture(GL_TEXTURE_2D, m_RendererID);

        if (format == Format::RGBA) {
            m_InternalFormat = GL_RGBA8;
            m_DataFormat = GL_RGBA;
        } else {
            m_InternalFormat = GL_R8;
            m_DataFormat = GL_RED;
        }

        if (format == Format::Red)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        ::glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);

        if (format == Format::Red)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        SetDefaultParameters(format);

        m_Loaded = (data != nullptr) || m_Path.empty();
    }

}
