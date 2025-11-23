#pragma once

#include "SAGE/Core/ResourceManager.h"
#include <cstdint>
#include <string>
#include <memory>

namespace SAGE {

enum class TextureFilter {
    Nearest,
    Linear
};

enum class TextureWrap {
    Repeat,
    Clamp
};

struct TextureSpec {
    TextureFilter minFilter = TextureFilter::Nearest;
    TextureFilter magFilter = TextureFilter::Nearest;
    TextureWrap wrapS = TextureWrap::Clamp;
    TextureWrap wrapT = TextureWrap::Clamp;
    bool generateMipmaps = false;
    bool flipVertically = false; // Use standard image coordinates (Top-Left origin)
};

class Texture : public IResource {
public:
    Texture() = default;
    Texture(const std::string& path, const TextureSpec& spec = {});
    Texture(uint32_t width, uint32_t height, const void* data, const TextureSpec& spec = {});
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetID() const { return m_TextureID; }

    // IResource interface
    bool Load(const std::string& path) override;
    void Unload() override;
    bool IsLoaded() const override { return m_TextureID != 0; }
    const std::string& GetPath() const override { return m_Path; }
    const TextureSpec& GetSpec() const { return m_Spec; }

    void SetFilter(TextureFilter min, TextureFilter mag);
    void SetWrap(TextureWrap s, TextureWrap t);
    void SetSpec(const TextureSpec& spec) { m_Spec = spec; }

    static std::shared_ptr<Texture> Create(const std::string& path, const TextureSpec& spec = {});
    static std::shared_ptr<Texture> CreateWhiteTexture();
    static std::shared_ptr<Texture> CreateFromData(int width, int height, const void* data, const TextureSpec& spec = {});

private:
    void CreateFromData(const void* data, const TextureSpec& spec);

    uint32_t m_TextureID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    int m_Channels = 0;
    std::string m_Path;
    TextureSpec m_Spec;
};

} // namespace SAGE

