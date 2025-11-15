#pragma once

#include "Graphics/Backend/Interfaces/ISceneRenderer.h"
#include "Graphics/Backend/Interfaces/IRenderBackend.h"
#include "Memory/Ref.h"

#include <memory>

namespace SAGE {

class ShaderManager;
class GraphicsResourceManager;

namespace StateManagement {
    class RenderStateManager;
}

namespace Graphics {
    
namespace Batching {
    class BatchRenderer;
}

struct RenderSystemConfig;

/// Instance-based render context
/// Owns all rendering subsystems: state, batching, shaders, scene renderer
/// Replaces global static singletons with controlled lifetime
class RenderContext {
public:
    RenderContext();
    ~RenderContext();

    // Non-copyable, movable
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;
    RenderContext(RenderContext&&) noexcept;
    RenderContext& operator=(RenderContext&&) noexcept;

    // Lifecycle
    void Init(const RenderSystemConfig& config);
    void Shutdown();
    [[nodiscard]] bool IsInitialized() const;

    // Per-frame update
    void Update(float deltaTime);

    // Subsystem access
    [[nodiscard]] StateManagement::RenderStateManager& GetStateManager();
    [[nodiscard]] const StateManagement::RenderStateManager& GetStateManager() const;

    [[nodiscard]] Batching::BatchRenderer& GetBatchRenderer();
    [[nodiscard]] const Batching::BatchRenderer& GetBatchRenderer() const;

    [[nodiscard]] ShaderManager& GetShaderManager();
    [[nodiscard]] const ShaderManager& GetShaderManager() const;

    [[nodiscard]] ISceneRenderer& GetSceneRenderer();
    [[nodiscard]] const ISceneRenderer& GetSceneRenderer() const;

    // Access raw shared ownership of the scene renderer (needed for re-registration
    // or external validation without breaking encapsulation). Returns empty shared_ptr
    // if not initialized or scene renderer missing.
    [[nodiscard]] std::shared_ptr<ISceneRenderer> GetSceneRendererShared() const;

    [[nodiscard]] IRenderBackend& GetBackend();
    [[nodiscard]] const IRenderBackend& GetBackend() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // namespace Graphics
} // namespace SAGE
