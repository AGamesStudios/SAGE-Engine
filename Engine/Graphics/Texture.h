#pragma once

#include <string>

namespace SAGE {

    class Texture {
    public:
        enum class Format {
            RGBA,
            Red
        };

        Texture(const std::string& path);
        Texture(unsigned int width, unsigned int height, Format format = Format::RGBA, const unsigned char* data = nullptr);
        ~Texture();
        
        void Bind(unsigned int slot = 0) const;
        void Unbind() const;
        
        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }
        unsigned int GetRendererID() const { return m_RendererID; }
        bool IsLoaded() const { return m_Loaded; }
    Format GetFormat() const { return m_Format; }
        
        void SetData(void* data, unsigned int size);
        
    private:
    void Allocate(unsigned int width, unsigned int height, Format format, const unsigned char* data);

        unsigned int m_RendererID;
        unsigned int m_Width, m_Height;
        unsigned int m_InternalFormat = 0;
        unsigned int m_DataFormat = 0;
        std::string m_Path;
        bool m_Loaded = false;
        Format m_Format = Format::RGBA;
    };

}
