#pragma once

#include <SAGE/IEngine.h>
#include <Graphics/ShaderManager.h>
#include <Memory/Ref.h>
#include <unordered_map>

namespace SAGE {
class Texture;
class Shader;

namespace Internal {

// Wraps the internal ResourceManager and ShaderManager into the public IResourceManager interface.
class ResourceManagerAdapter : public IResourceManager {
public:
    ResourceManagerAdapter();
    ~ResourceManagerAdapter() override;

    // IResourceManager interface
    TextureHandle LoadTexture(const std::string& path) override;
    void UnloadTexture(TextureHandle handle) override;
    bool IsTextureValid(TextureHandle handle) const override;

    ShaderHandle LoadShader(const std::string& vertexPath, const std::string& fragmentPath) override;
    void UnloadShader(ShaderHandle handle) override;
    bool IsShaderValid(ShaderHandle handle) const override;

private:
    struct TextureRecord {
        Ref<Texture> texture;
    };

    struct ShaderRecord {
        Ref<Shader> shader;
        std::string name;
    };

    TextureHandle m_NextTextureHandle = 1;
    ShaderHandle m_NextShaderHandle = 1;

    std::unordered_map<TextureHandle, TextureRecord> m_Textures;
    std::unordered_map<ShaderHandle, ShaderRecord> m_Shaders;

    ShaderManager m_ShaderManager;

    static std::string MakeShaderName(const std::string& vertexPath, const std::string& fragmentPath);
};

} // namespace Internal
} // namespace SAGE
