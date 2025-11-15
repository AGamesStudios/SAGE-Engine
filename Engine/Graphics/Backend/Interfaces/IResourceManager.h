#pragma once

#include "IRenderDevice.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace SAGE::Graphics {

struct TextureDataView {
    const void* data = nullptr;
    std::size_t size = 0;
};

struct ShaderSource {
    std::string_view vertex;
    std::string_view fragment;
    std::string_view debugName;
};

struct MaterialDesc {
    ShaderHandle shader = 0;
    TextureHandle diffuseTexture = 0;
    TextureHandle normalTexture = 0;
};

class IResourceManager {
public:
    virtual ~IResourceManager() = default;

    virtual void Initialize(IRenderDevice& device) = 0;
    virtual void Shutdown() = 0;

    [[nodiscard]] virtual TextureHandle LoadTexture(std::string_view id,
                                                    const TextureDesc& desc,
                                                    const TextureDataView& data) = 0;
    [[nodiscard]] virtual ShaderHandle LoadShader(std::string_view id,
                                                  const ShaderSource& source) = 0;
    [[nodiscard]] virtual MaterialHandle CreateMaterial(std::string_view id,
                                                        const MaterialDesc& desc) = 0;

    [[nodiscard]] virtual std::optional<TextureHandle> TryGetTexture(std::string_view id) const = 0;
    [[nodiscard]] virtual std::optional<ShaderHandle> TryGetShader(std::string_view id) const = 0;
    [[nodiscard]] virtual std::optional<MaterialHandle> TryGetMaterial(std::string_view id) const = 0;

    virtual void DestroyTexture(TextureHandle handle) = 0;
    virtual void DestroyShader(ShaderHandle handle) = 0;
    virtual void DestroyMaterial(MaterialHandle handle) = 0;
};

} // namespace SAGE::Graphics
