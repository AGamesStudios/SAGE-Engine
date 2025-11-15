#include "ResourceManagerAdapter.h"

#include <Core/ResourceManager.h>
#include <Core/Logger.h>
#include <Graphics/Core/Resources/Texture.h>
#include <Graphics/Core/Resources/Shader.h>

namespace SAGE {
namespace Internal {

ResourceManagerAdapter::ResourceManagerAdapter() {
    m_ShaderManager.Init();
}

ResourceManagerAdapter::~ResourceManagerAdapter() {
    if (m_ShaderManager.IsInitialized()) {
        m_ShaderManager.Shutdown();
    }
}

TextureHandle ResourceManagerAdapter::LoadTexture(const std::string& path) {
    auto texture = ResourceManager::Get().Load<Texture>(path);
    if (!texture) {
        SAGE_WARN("ResourceManagerAdapter: failed to load texture '{}'", path);
        return NullTexture;
    }

    TextureHandle handle = m_NextTextureHandle++;
    m_Textures[handle] = TextureRecord{texture};
    return handle;
}

void ResourceManagerAdapter::UnloadTexture(TextureHandle handle) {
    auto it = m_Textures.find(handle);
    if (it == m_Textures.end()) {
        return;
    }

    if (it->second.texture) {
        ResourceManager::Get().Unload(it->second.texture->GetPath());
    }
    m_Textures.erase(it);
}

bool ResourceManagerAdapter::IsTextureValid(TextureHandle handle) const {
    auto it = m_Textures.find(handle);
    if (it == m_Textures.end()) {
        return false;
    }
    return it->second.texture && it->second.texture->IsLoaded();
}

std::string ResourceManagerAdapter::MakeShaderName(const std::string& vertexPath, const std::string& fragmentPath) {
    return vertexPath + "|" + fragmentPath;
}

ShaderHandle ResourceManagerAdapter::LoadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string shaderName = MakeShaderName(vertexPath, fragmentPath);
    auto shader = m_ShaderManager.LoadFromFile(shaderName, vertexPath, fragmentPath);
    if (!shader) {
        SAGE_WARN("ResourceManagerAdapter: failed to load shader (v: '{}', f: '{}')", vertexPath, fragmentPath);
        return NullShader;
    }

    ShaderHandle handle = m_NextShaderHandle++;
    m_Shaders[handle] = ShaderRecord{shader, shaderName};
    return handle;
}

void ResourceManagerAdapter::UnloadShader(ShaderHandle handle) {
    auto it = m_Shaders.find(handle);
    if (it == m_Shaders.end()) {
        return;
    }

    if (!it->second.name.empty()) {
        m_ShaderManager.Remove(it->second.name);
    }
    m_Shaders.erase(it);
}

bool ResourceManagerAdapter::IsShaderValid(ShaderHandle handle) const {
    auto it = m_Shaders.find(handle);
    if (it == m_Shaders.end()) {
        return false;
    }
    return it->second.shader && it->second.shader->IsValid();
}

} // namespace Internal
} // namespace SAGE
