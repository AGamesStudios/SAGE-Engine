#include "Graphics/Backend/Implementations/OpenGL/OpenGLAdapters.h"

#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Graphics/ShaderManager.h"
#include "Core/Logger.h"
#include "Core/ServiceLocator.h"

#include <glad/glad.h>

#include <array>
#include <utility>

namespace SAGE::Graphics {

namespace {

using ::SAGE::GraphicsResourceManager;
using ::SAGE::Shader;
using ::SAGE::ShaderManager;
using ::SAGE::IShaderManager;
using ::SAGE::ServiceLocator;

IShaderManager& GetShaderManager() {
    static ShaderManager s_Fallback;
    if (ServiceLocator::HasGlobalInstance()) {
        auto& services = ServiceLocator::GetGlobalInstance();
        if (services.HasShaderManager()) {
            return services.GetShaderManager();
        }
    }
    return s_Fallback;
}


template <typename HandleT>
HandleT NextHandle(HandleT& counter) {
    HandleT next = counter++;
    if (next == 0) {
        next = counter++;
    }
    return next;
}

GLenum ToGLTopology(PrimitiveTopology topology) {
    switch (topology) {
    case PrimitiveTopology::Lines:
        return GL_LINES;
    case PrimitiveTopology::Points:
        return GL_POINTS;
    case PrimitiveTopology::Triangles:
    default:
        return GL_TRIANGLES;
    }
}

GLenum ToGLBlendFactor(BlendFactor factor) {
    switch (factor) {
    case BlendFactor::Zero: return GL_ZERO;
    case BlendFactor::One: return GL_ONE;
    case BlendFactor::SourceColor: return GL_SRC_COLOR;
    case BlendFactor::InverseSourceColor: return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DestinationColor: return GL_DST_COLOR;
    case BlendFactor::InverseDestinationColor: return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::SourceAlpha: return GL_SRC_ALPHA;
    case BlendFactor::InverseSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DestinationAlpha: return GL_DST_ALPHA;
    case BlendFactor::InverseDestinationAlpha: return GL_ONE_MINUS_DST_ALPHA;
    default: return GL_ONE;
    }
}

GLenum ToGLBlendEquation(BlendOperation op) {
    switch (op) {
    case BlendOperation::Add: return GL_FUNC_ADD;
    case BlendOperation::Subtract: return GL_FUNC_SUBTRACT;
    case BlendOperation::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOperation::Min: return GL_MIN;
    case BlendOperation::Max: return GL_MAX;
    default: return GL_FUNC_ADD;
    }
}

GLenum ToGLDepthFunc(DepthCompare compare) {
    switch (compare) {
    case DepthCompare::Less: return GL_LESS;
    case DepthCompare::LessEqual: return GL_LEQUAL;
    case DepthCompare::Equal: return GL_EQUAL;
    case DepthCompare::Greater: return GL_GREATER;
    case DepthCompare::GreaterEqual: return GL_GEQUAL;
    case DepthCompare::Always: return GL_ALWAYS;
    case DepthCompare::Never: return GL_NEVER;
    default: return GL_LEQUAL;
    }
}

} // namespace

OpenGLDeviceAdapter::OpenGLDeviceAdapter() = default;

void OpenGLDeviceAdapter::Initialize() {
    if (m_Initialized) {
        return;
    }
    GraphicsResourceManager::Init();
    GetShaderManager().Init();
    m_Initialized = true;
}

void OpenGLDeviceAdapter::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    for (auto& [handle, record] : m_Textures) {
        record.handle.Reset();
    }
    m_Textures.clear();

    m_Shaders.clear();

    m_Initialized = false;
}

bool OpenGLDeviceAdapter::IsInitialized() const {
    return m_Initialized;
}

TextureHandle OpenGLDeviceAdapter::CreateTexture(const TextureDesc& desc,
                                                 const void* data,
                                                 std::size_t /*dataSize*/) {
    if (!m_Initialized || desc.width == 0 || desc.height == 0) {
        return 0;
    }

    GraphicsResourceManager::TrackedTextureHandle glHandle;
    glHandle.Create("DeviceAdapter_Texture");

    glBindTexture(GL_TEXTURE_2D, glHandle.Get());
    const GLenum internalFormat = GL_RGBA8;
    const GLenum dataFormat = GL_RGBA;
    const GLenum dataType = GL_UNSIGNED_BYTE;

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 static_cast<GLsizei>(desc.width),
                 static_cast<GLsizei>(desc.height),
                 0,
                 dataFormat,
                 dataType,
                 data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (desc.generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    const TextureHandle handle = NextHandle(m_NextTextureHandle);
    m_Textures.emplace(handle, TextureRecord{ std::move(glHandle), desc });
    return handle;
}

void OpenGLDeviceAdapter::DestroyTexture(TextureHandle handle) {
    if (auto it = m_Textures.find(handle); it != m_Textures.end()) {
        it->second.handle.Reset();
        m_Textures.erase(it);
    }
}

ShaderHandle OpenGLDeviceAdapter::CompileShader(const ShaderCompileRequest& request) {
    if (!m_Initialized || request.vertexSource.empty() || request.fragmentSource.empty()) {
        return 0;
    }

    auto shader = CreateRef<Shader>(std::string(request.vertexSource), std::string(request.fragmentSource));
    if (!shader) {
        SAGE_ERROR("OpenGLDeviceAdapter: failed to compile shader '{}'", request.debugName);
        return 0;
    }

    const ShaderHandle handle = NextHandle(m_NextShaderHandle);
    m_Shaders.emplace(handle, shader);
    return handle;
}

void OpenGLDeviceAdapter::DestroyShader(ShaderHandle handle) {
    m_Shaders.erase(handle);
}

void OpenGLDeviceAdapter::DrawPrimitives(const DrawPrimitiveArgs& args) {
    const GLenum mode = ToGLTopology(args.topology);
    if (args.instanceCount > 1) {
        glDrawArraysInstanced(mode,
                              0,
                              static_cast<GLsizei>(args.vertexCount),
                              static_cast<GLsizei>(args.instanceCount));
    } else {
        glDrawArrays(mode, 0, static_cast<GLsizei>(args.vertexCount));
    }
}

void OpenGLContextAdapter::SetViewport(const Viewport& viewport) {
    glViewport(viewport.x,
               viewport.y,
               viewport.width,
               viewport.height);
}

void OpenGLContextAdapter::SetScissor(const ScissorRect& scissor) {
    if (scissor.width <= 0 || scissor.height <= 0) {
        glDisable(GL_SCISSOR_TEST);
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

void OpenGLContextAdapter::SetBlendState(const BlendStateDesc& state) {
    if (state.enabled) {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(ToGLBlendFactor(state.sourceColor),
                            ToGLBlendFactor(state.destinationColor),
                            ToGLBlendFactor(state.sourceAlpha),
                            ToGLBlendFactor(state.destinationAlpha));
        glBlendEquationSeparate(ToGLBlendEquation(state.colorOp),
                                ToGLBlendEquation(state.alphaOp));
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGLContextAdapter::SetDepthState(const DepthStateDesc& state) {
    if (state.testEnabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(ToGLDepthFunc(state.compare));
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(state.writeEnabled ? GL_TRUE : GL_FALSE);

    if (state.biasConstant != 0.0f || state.biasSlope != 0.0f) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(state.biasSlope, state.biasConstant);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void OpenGLContextAdapter::SetRenderTarget(RenderTargetHandle handle) {
    if (handle == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        SAGE_WARNING("OpenGLContextAdapter::SetRenderTarget received unsupported handle {}", handle);
    }
}

void OpenGLResourceManagerAdapter::Initialize(IRenderDevice& device) {
    m_Device = &device;
    m_Initialized = true;
}

void OpenGLResourceManagerAdapter::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    if (m_Device) {
        for (auto& [handle, id] : m_TextureHandles) {
            m_Device->DestroyTexture(handle);
        }
        for (auto& [handle, id] : m_ShaderHandles) {
            m_Device->DestroyShader(handle);
        }
    }

    m_TextureIds.clear();
    m_ShaderIds.clear();
    m_MaterialIds.clear();
    m_TextureHandles.clear();
    m_ShaderHandles.clear();
    m_Materials.clear();
    m_Device = nullptr;
    m_Initialized = false;
}

TextureHandle OpenGLResourceManagerAdapter::LoadTexture(std::string_view id,
                                                        const TextureDesc& desc,
                                                        const TextureDataView& data) {
    if (!m_Device || id.empty()) {
        return 0;
    }

    if (auto it = m_TextureIds.find(std::string(id)); it != m_TextureIds.end()) {
        return it->second;
    }

    const TextureHandle handle = m_Device->CreateTexture(desc, data.data, data.size);
    if (handle == 0) {
        return 0;
    }

    std::string idStr{id};
    m_TextureIds[idStr] = handle;
    m_TextureHandles[handle] = idStr;
    return handle;
}

ShaderHandle OpenGLResourceManagerAdapter::LoadShader(std::string_view id,
                                                      const ShaderSource& source) {
    if (!m_Device || id.empty()) {
        return 0;
    }

    if (auto it = m_ShaderIds.find(std::string(id)); it != m_ShaderIds.end()) {
        return it->second;
    }

    ShaderCompileRequest request{};
    request.vertexSource = source.vertex;
    request.fragmentSource = source.fragment;
    request.debugName = source.debugName.empty() ? id : source.debugName;

    const ShaderHandle handle = m_Device->CompileShader(request);
    if (handle == 0) {
        return 0;
    }

    std::string idStr{id};
    m_ShaderIds[idStr] = handle;
    m_ShaderHandles[handle] = idStr;
    return handle;
}

MaterialHandle OpenGLResourceManagerAdapter::CreateMaterial(std::string_view id,
                                                            const MaterialDesc& desc) {
    if (!m_Device || id.empty()) {
        return 0;
    }

    if (auto it = m_MaterialIds.find(std::string(id)); it != m_MaterialIds.end()) {
        return it->second;
    }

    const MaterialHandle handle = NextHandle(m_NextMaterialHandle);
    m_MaterialIds[std::string(id)] = handle;
    m_Materials.emplace(handle, MaterialRecord{ desc, std::string(id) });
    return handle;
}

std::optional<TextureHandle> OpenGLResourceManagerAdapter::TryGetTexture(std::string_view id) const {
    if (auto it = m_TextureIds.find(std::string(id)); it != m_TextureIds.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ShaderHandle> OpenGLResourceManagerAdapter::TryGetShader(std::string_view id) const {
    if (auto it = m_ShaderIds.find(std::string(id)); it != m_ShaderIds.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<MaterialHandle> OpenGLResourceManagerAdapter::TryGetMaterial(std::string_view id) const {
    if (auto it = m_MaterialIds.find(std::string(id)); it != m_MaterialIds.end()) {
        return it->second;
    }
    return std::nullopt;
}

void OpenGLResourceManagerAdapter::DestroyTexture(TextureHandle handle) {
    if (!m_Device) {
        return;
    }
    if (auto it = m_TextureHandles.find(handle); it != m_TextureHandles.end()) {
        m_Device->DestroyTexture(handle);
        m_TextureIds.erase(it->second);
        m_TextureHandles.erase(it);
    }
}

void OpenGLResourceManagerAdapter::DestroyShader(ShaderHandle handle) {
    if (!m_Device) {
        return;
    }
    if (auto it = m_ShaderHandles.find(handle); it != m_ShaderHandles.end()) {
        m_Device->DestroyShader(handle);
        m_ShaderIds.erase(it->second);
        m_ShaderHandles.erase(it);
    }
}

void OpenGLResourceManagerAdapter::DestroyMaterial(MaterialHandle handle) {
    if (auto it = m_Materials.find(handle); it != m_Materials.end()) {
        m_MaterialIds.erase(it->second.id);
        m_Materials.erase(it);
    }
}

} // namespace SAGE::Graphics
