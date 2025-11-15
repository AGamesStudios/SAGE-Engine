#pragma once

#include "Graphics/Interfaces/IRenderStateManager.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Resources/Material.h"
#include "DepthStateController.h"
#include "BlendStateController.h"
#include "StateStackManager.h"

namespace SAGE {
namespace StateManagement {

/// @brief Concrete implementation of IRenderStateManager
/// Central state management coordinator
/// Isolates all rendering state from low-level GL calls
/// Can be used both as instance (new way) and static (legacy compatibility)
class RenderStateManager : public IRenderStateManager {
public:
    RenderStateManager() = default;
    ~RenderStateManager() override = default;

    // ========== IRenderStateManager Interface ==========

    void Init() override;
    void Shutdown() override;

    // Blend state management
    void SetBlendMode(BlendMode mode) override;
    BlendMode GetBlendMode() override;
    void PushBlendMode(BlendMode mode) override;
    void PopBlendMode() override;

    // Depth state management
    void SetDepthState(const DepthSettings& settings) override;
    DepthSettings GetDepthState() override;
    void PushDepthState(const DepthSettings& settings) override;
    void PopDepthState() override;

    // Validation and diagnostics
    void Validate() override;
    void ApplyDirtyStates() override;

    // Controller access
    DepthStateController& Depth() override;
    BlendStateController& Blend() override;
    StateStackManager& Stack() override;

private:
    // Instance data
    bool m_Initialized = false;
    DepthStateController m_DepthController;
    BlendStateController m_BlendController;
    StateStackManager m_StackManager;
};

} // namespace StateManagement
} // namespace SAGE

