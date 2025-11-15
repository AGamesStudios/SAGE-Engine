#pragma once

#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/Backend/Common/BackendType.h"
#include "Graphics/Backend/Interfaces/IRenderContext.h"
#include "Graphics/Backend/Interfaces/IRenderDevice.h"
#include "Graphics/Backend/Interfaces/IResourceManager.h"
#include "Graphics/Backend/Interfaces/ISceneRenderer.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"

#include <functional>
#include <map>
#include <memory>

namespace SAGE::Graphics {

class RenderSystemRegistry {
public:
    using BackendFactory = std::function<std::unique_ptr<::SAGE::IRenderBackend>()>;

    void RegisterBackendFactory(BackendType type, BackendFactory factory);
    [[nodiscard]] bool HasBackendFactory(BackendType type) const;
    [[nodiscard]] std::unique_ptr<IRenderBackend> CreateBackend(const RenderSystemConfig& config) const;

    void SetDevice(std::shared_ptr<IRenderDevice> device);
    [[nodiscard]] std::shared_ptr<IRenderDevice> GetDevice() const;

    void SetContext(std::shared_ptr<IRenderContext> context);
    [[nodiscard]] std::shared_ptr<IRenderContext> GetContext() const;

    void SetResourceManager(std::shared_ptr<IResourceManager> resources);
    [[nodiscard]] std::shared_ptr<IResourceManager> GetResourceManager() const;

    void SetSceneRenderer(std::shared_ptr<ISceneRenderer> sceneRenderer);
    [[nodiscard]] std::shared_ptr<ISceneRenderer> GetSceneRenderer() const;

    // Legacy raw-pointer activation (kept for compatibility)
    void SetActiveBackend(::SAGE::IRenderBackend* backend);
    [[nodiscard]] ::SAGE::IRenderBackend* GetActiveBackend() const;

    // Preferred: managed ownership of backend
    void SetActiveBackendShared(std::shared_ptr<::SAGE::IRenderBackend> backend);
    [[nodiscard]] std::shared_ptr<::SAGE::IRenderBackend> GetActiveBackendShared() const { return m_BackendShared; }

private:
    std::map<BackendType, BackendFactory> m_BackendFactories;
    std::shared_ptr<IRenderDevice> m_Device;
    std::shared_ptr<IRenderContext> m_Context;
    std::shared_ptr<IResourceManager> m_ResourceManager;
    std::shared_ptr<ISceneRenderer> m_SceneRenderer;
    ::SAGE::IRenderBackend* m_ActiveBackend = nullptr; // Non-owning for legacy access
    std::shared_ptr<::SAGE::IRenderBackend> m_BackendShared; // Owning backend reference
};

} // namespace SAGE::Graphics
