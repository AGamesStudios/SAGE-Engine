#pragma once

#include "RenderHandles.h"
#include "Graphics/Core/Types/GraphicsTypes.h"

#include <cstddef>
#include <string_view>

namespace SAGE::Graphics {

struct ShaderCompileRequest {
    std::string_view vertexSource;
    std::string_view fragmentSource;
    std::string_view debugName;
};

struct DrawPrimitiveArgs {
    PrimitiveTopology topology = PrimitiveTopology::Triangles;
    std::uint32_t vertexCount = 0;
    std::uint32_t instanceCount = 1;
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    [[nodiscard]] virtual bool IsInitialized() const = 0;

    [[nodiscard]] virtual TextureHandle CreateTexture(const TextureDesc& desc,
                                                      const void* data,
                                                      std::size_t dataSize) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;

    [[nodiscard]] virtual ShaderHandle CompileShader(const ShaderCompileRequest& request) = 0;
    virtual void DestroyShader(ShaderHandle handle) = 0;

    virtual void DrawPrimitives(const DrawPrimitiveArgs& args) = 0;
};

} // namespace SAGE::Graphics
