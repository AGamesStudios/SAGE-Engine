#include "RenderStateManager.h"
#include "Core/Logger.h"
#include <cassert>

namespace SAGE {
namespace StateManagement {

void RenderStateManager::Init() {
    if (m_Initialized) {
        SAGE_WARNING("[StateManagement] RenderStateManager::Init called multiple times on instance");
        return;
    }
    SAGE_INFO("[StateManagement] Initializing RenderStateManager instance");
    m_DepthController.Init();
    m_BlendController.Init();
    m_StackManager.Init();
    m_Initialized = true;
}

void RenderStateManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    SAGE_INFO("[StateManagement] Shutting down RenderStateManager instance");
    m_DepthController.Shutdown();
    m_BlendController.Shutdown();
    m_StackManager.Shutdown();
    m_Initialized = false;
}

void RenderStateManager::SetBlendMode(BlendMode mode) {
    m_BlendController.SetBlendMode(mode);
}

BlendMode RenderStateManager::GetBlendMode() {
    return m_BlendController.GetBlendMode();
}

void RenderStateManager::PushBlendMode(BlendMode mode) {
    m_BlendController.PushBlendMode(mode);
    m_StackManager.ResetOverrideCount();
}

void RenderStateManager::PopBlendMode() {
    m_BlendController.PopBlendMode();
    m_StackManager.ResetOverrideCount();
}

void RenderStateManager::SetDepthState(const DepthSettings& settings) {
    m_DepthController.SetDepthState(settings);
}

DepthSettings RenderStateManager::GetDepthState() {
    return m_DepthController.GetDepthState();
}

void RenderStateManager::PushDepthState(const DepthSettings& settings) {
    m_DepthController.PushDepthState(settings);
    m_StackManager.ResetOverrideCount();
}

void RenderStateManager::PopDepthState() {
    m_DepthController.PopDepthState();
    m_StackManager.ResetOverrideCount();
}

void RenderStateManager::Validate() {
    m_BlendController.Validate();
    m_DepthController.Validate();
    m_StackManager.Validate();
}

void RenderStateManager::ApplyDirtyStates() {
    if (m_BlendController.IsDirty()) {
        m_BlendController.ApplyToBackend();
    }
    if (m_DepthController.IsDirty()) {
        m_DepthController.ApplyToBackend();
    }
}

DepthStateController& RenderStateManager::Depth() { return m_DepthController; }
BlendStateController& RenderStateManager::Blend() { return m_BlendController; }
StateStackManager& RenderStateManager::Stack() { return m_StackManager; }

} // namespace StateManagement
} // namespace SAGE
