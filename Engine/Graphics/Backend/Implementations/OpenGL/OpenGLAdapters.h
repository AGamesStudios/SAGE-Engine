#pragma once

#include "Graphics/Backend/Interfaces/IRenderDevice.h"
#include "Graphics/Backend/Interfaces/IRenderContext.h"
#include "Graphics/Backend/Interfaces/IResourceManager.h"
#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Memory/Ref.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SAGE::Graphics {

using ::SAGE::GraphicsResourceManager;
using ::SAGE::Shader;

class OpenGLDeviceAdapter final : public IRenderDevice {
public:
    OpenGLDeviceAdapter();

    void Initialize() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const override;

    [[nodiscard]] TextureHandle CreateTexture(const TextureDesc& desc,
                                              const void* data,
                                              std::size_t dataSize) override;
    void DestroyTexture(TextureHandle handle) override;

    [[nodiscard]] ShaderHandle CompileShader(const ShaderCompileRequest& request) override;
    void DestroyShader(ShaderHandle handle) override;

    void DrawPrimitives(const DrawPrimitiveArgs& args) override;

private:
    struct TextureRecord {
        GraphicsResourceManager::TrackedTextureHandle handle;
        TextureDesc desc;
    };

    bool m_Initialized = false;
    std::unordered_map<TextureHandle, TextureRecord> m_Textures;
    std::unordered_map<ShaderHandle, Ref<Shader>> m_Shaders;
    TextureHandle m_NextTextureHandle = 1;
    ShaderHandle m_NextShaderHandle = 1;
};

class OpenGLContextAdapter final : public IRenderContext {
public:
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const ScissorRect& scissor) override;

    void SetBlendState(const BlendStateDesc& state) override;
    void SetDepthState(const DepthStateDesc& state) override;

    void SetRenderTarget(RenderTargetHandle handle) override;
};

class OpenGLResourceManagerAdapter final : public IResourceManager {
public:
    void Initialize(IRenderDevice& device) override;
    void Shutdown() override;

    [[nodiscard]] TextureHandle LoadTexture(std::string_view id,
                                            const TextureDesc& desc,
                                            const TextureDataView& data) override;
    [[nodiscard]] ShaderHandle LoadShader(std::string_view id,
                                          const ShaderSource& source) override;
    [[nodiscard]] MaterialHandle CreateMaterial(std::string_view id,
                                                const MaterialDesc& desc) override;

    [[nodiscard]] std::optional<TextureHandle> TryGetTexture(std::string_view id) const override;
    [[nodiscard]] std::optional<ShaderHandle> TryGetShader(std::string_view id) const override;
    [[nodiscard]] std::optional<MaterialHandle> TryGetMaterial(std::string_view id) const override;

    void DestroyTexture(TextureHandle handle) override;
    void DestroyShader(ShaderHandle handle) override;
    void DestroyMaterial(MaterialHandle handle) override;

private:
    struct MaterialRecord {
        MaterialDesc desc;
        std::string id;
    };

    IRenderDevice* m_Device = nullptr;
    bool m_Initialized = false;

    std::unordered_map<std::string, TextureHandle> m_TextureIds;
    std::unordered_map<std::string, ShaderHandle> m_ShaderIds;
    std::unordered_map<std::string, MaterialHandle> m_MaterialIds;

    std::unordered_map<TextureHandle, std::string> m_TextureHandles;
    std::unordered_map<ShaderHandle, std::string> m_ShaderHandles;
    std::unordered_map<MaterialHandle, MaterialRecord> m_Materials;

    MaterialHandle m_NextMaterialHandle = 1;
};

} // namespace SAGE::Graphics
