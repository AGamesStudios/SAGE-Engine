#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Core/IResource.h"
#include "Graphics/GraphicsResourceManager.h"

namespace SAGE {

    class Texture : public IResource {
    public:
        enum class Format {
            RGBA8,
            RGB8,
            Red8,
            RGBA16F,
            BC1,
            BC3,
            BC5,
            ASTC_4x4,
            ETC2_RGBA8
        };

        struct MipLevelData {
            unsigned int Width = 0;
            unsigned int Height = 0;
            std::vector<std::uint8_t> Bytes;
        };

        Texture(const std::string& path);
        Texture(unsigned int width,
                unsigned int height,
                Format format = Format::RGBA8,
                const unsigned char* data = nullptr,
                bool generateMipmaps = false);
        Texture(const std::vector<MipLevelData>& mipChain,
                Format format,
                bool compressed = false);
        // Constructor for external GL texture (from RenderTarget)
        Texture();  // Creates empty texture for external wrapping
        ~Texture() override;
        
        // RAII: запретить копирование, разрешить move
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        // IResource interface implementation
        [[nodiscard]] std::size_t GetGPUMemorySize() const override;
        [[nodiscard]] const std::string& GetPath() const override { return m_Path; }
        bool Unload() noexcept override; // Возвращает true при успехе
        bool Reload() override; // Возвращает true при успехе
    [[nodiscard]] bool IsLoaded() const override { return m_Loaded; }
    [[nodiscard]] ResourceState GetState() const override { return m_State; }

        void Bind(unsigned int slot = 0) const;
        void Unbind() const;

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }
        unsigned int GetRendererID() const { return m_TextureHandle.Get(); }
        unsigned int GetMipLevels() const { return m_MipLevels; }
        Format GetFormat() const { return m_Format; }
        bool IsCompressed() const { return m_IsCompressed; }

        // For RenderTarget support
        void SetWidth(unsigned int width) { m_Width = width; }
        void SetHeight(unsigned int height) { m_Height = height; }
        
        /// @brief Установить внешний нативный ID текстуры (абстракция от OpenGL/Vulkan/DX)
        void SetNativeTextureID(unsigned int textureID);
        
        /// @deprecated Используйте SetNativeTextureID()
        void SetGLTexture(unsigned int textureID) { SetNativeTextureID(textureID); }

        void SetData(void* data, unsigned int size);
        void UploadMipChain(const std::vector<MipLevelData>& mipChain, bool compressed);
        void GenerateMipmaps();
        
        /// @brief Проверить, сгенерированы ли миpmaps
        bool AreMipmapsGenerated() const { return m_MipmapsGenerated; }

        enum class FilterMode {
            Nearest,
            Linear
        };

        void SetFilter(FilterMode minFilter,
                       FilterMode magFilter,
                       bool enableMipmaps = true);
        void SetFilter(FilterMode filter, bool enableMipmaps = true) {
            SetFilter(filter, filter, enableMipmaps);
        }
        FilterMode GetMinFilter() const { return m_MinFilter; }
        FilterMode GetMagFilter() const { return m_MagFilter; }
        bool GetFilterUsesMipmaps() const { return m_FilterUseMipmaps; }

    // Mark as stub placeholder (1x1 pixel) without being treated as fully unloaded
    void MarkStub() { m_State = ResourceState::Stub; m_Loaded = true; }

        static bool SupportsCompression(Format format);
        static bool IsCompressedFormat(Format format);
        static std::size_t BytesPerBlock(Format format);
        static unsigned int BlockWidth(Format format);
        static unsigned int BlockHeight(Format format);
        static std::size_t CalculateDataFootprint(Format format,
                                                  unsigned int width,
                                                  unsigned int height,
                                                  unsigned int mipLevels,
                                                  bool compressed);

    private:
        void Allocate(unsigned int width,
                      unsigned int height,
                      Format format,
                      const unsigned char* data,
                      bool generateMipmaps,
                      bool compressed = false);

        void UploadMipLevel(const MipLevelData& mip, unsigned int level, bool compressed);
        void ResolveFormats(Format format, bool compressed);
        
        bool m_MipmapsGenerated = false; // Флаг для предотвращения повторной генерации
        void ApplySamplerState() const;
        static int ToGLMagFilter(FilterMode mode);
        static int ToGLMinFilter(FilterMode mode, bool mipmapAware, unsigned int mipLevels);

    GraphicsResourceManager::TrackedTextureHandle m_TextureHandle;
        unsigned int m_Width = 0;
        unsigned int m_Height = 0;
        unsigned int m_InternalFormat = 0;
        unsigned int m_DataFormat = 0;
        unsigned int m_DataType = 0;
        unsigned int m_MipLevels = 1;
        std::string m_Path;
    bool m_Loaded = false;
    ResourceState m_State = ResourceState::Unloaded;
    mutable bool m_BindWarnEmitted = false;
        bool m_IsCompressed = false;
        bool m_GenerateMipmaps = false;
        Format m_Format = Format::RGBA8;
        FilterMode m_MinFilter = FilterMode::Linear;
        FilterMode m_MagFilter = FilterMode::Linear;
        bool m_FilterUseMipmaps = true;
    };

} // namespace SAGE
