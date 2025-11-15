#include "Graphics/API/RenderSystemRegistry.h"

#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Core/Logger.h"

namespace SAGE::Graphics {

void RenderSystemRegistry::RegisterBackendFactory(BackendType type, BackendFactory factory) {
    m_BackendFactories[type] = std::move(factory);
}

bool RenderSystemRegistry::HasBackendFactory(BackendType type) const {
    return m_BackendFactories.find(type) != m_BackendFactories.end();
}

std::unique_ptr<::SAGE::IRenderBackend> RenderSystemRegistry::CreateBackend(const RenderSystemConfig& config) const {
    const auto it = m_BackendFactories.find(config.backendType);
    if (it == m_BackendFactories.end()) {
        SAGE_WARNING("RenderSystemRegistry: no backend factory registered for requested backend type");
        return nullptr;
    }

    if (!it->second) {
        SAGE_WARNING("RenderSystemRegistry: backend factory for requested backend type is null");
        return nullptr;
    }

    return it->second();
}

void RenderSystemRegistry::SetDevice(std::shared_ptr<IRenderDevice> device) {
    m_Device = std::move(device);
}

std::shared_ptr<IRenderDevice> RenderSystemRegistry::GetDevice() const {
    return m_Device;
}

void RenderSystemRegistry::SetContext(std::shared_ptr<IRenderContext> context) {
    m_Context = std::move(context);
}

std::shared_ptr<IRenderContext> RenderSystemRegistry::GetContext() const {
    return m_Context;
}

void RenderSystemRegistry::SetResourceManager(std::shared_ptr<IResourceManager> resources) {
    m_ResourceManager = std::move(resources);
}

std::shared_ptr<IResourceManager> RenderSystemRegistry::GetResourceManager() const {
    return m_ResourceManager;
}

void RenderSystemRegistry::SetSceneRenderer(std::shared_ptr<ISceneRenderer> sceneRenderer) {
    m_SceneRenderer = std::move(sceneRenderer);
}

std::shared_ptr<ISceneRenderer> RenderSystemRegistry::GetSceneRenderer() const {
    return m_SceneRenderer;
}

void RenderSystemRegistry::SetActiveBackend(::SAGE::IRenderBackend* backend) {
    m_ActiveBackend = backend;
}

::SAGE::IRenderBackend* RenderSystemRegistry::GetActiveBackend() const {
    return m_ActiveBackend;
}

void RenderSystemRegistry::SetActiveBackendShared(std::shared_ptr<::SAGE::IRenderBackend> backend) {
    m_BackendShared = std::move(backend);
    m_ActiveBackend = m_BackendShared.get();
}

} // namespace SAGE::Graphics
