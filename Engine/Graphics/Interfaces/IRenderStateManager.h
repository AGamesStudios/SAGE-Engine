#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Rendering/StateManagement/DepthStateController.h"
#include "Graphics/Rendering/StateManagement/BlendStateController.h"
#include "Graphics/Rendering/StateManagement/StateStackManager.h"

namespace SAGE {

/// @brief Interface for render state management
/// Provides abstraction over blend modes, depth testing, and state stacks
/// Enables dependency injection and mock-based testing
class IRenderStateManager {
public:
    virtual ~IRenderStateManager() = default;

    /// Initialize the state manager
    virtual void Init() = 0;

    /// Shutdown and cleanup
    virtual void Shutdown() = 0;

    // ========== Blend State Management ==========

    /// Set the current blend mode
    /// @param mode The blend mode to apply
    virtual void SetBlendMode(BlendMode mode) = 0;

    /// Get the current blend mode
    /// @return Current active blend mode
    [[nodiscard]] virtual BlendMode GetBlendMode() = 0;

    /// Push a new blend mode onto the stack
    /// @param mode The blend mode to push and activate
    virtual void PushBlendMode(BlendMode mode) = 0;

    /// Pop the top blend mode from the stack
    /// Restores the previous blend mode
    virtual void PopBlendMode() = 0;

    // ========== Depth State Management ==========

    /// Set the depth testing settings
    /// @param settings Depth state configuration
    virtual void SetDepthState(const DepthSettings& settings) = 0;

    /// Get the current depth state
    /// @return Current depth settings
    [[nodiscard]] virtual DepthSettings GetDepthState() = 0;

    /// Push new depth state onto the stack
    /// @param settings Depth settings to push and activate
    virtual void PushDepthState(const DepthSettings& settings) = 0;

    /// Pop the top depth state from the stack
    /// Restores the previous depth state
    virtual void PopDepthState() = 0;

    // ========== Validation and Diagnostics ==========

    /// Validate current state consistency
    /// Useful for debugging state issues
    virtual void Validate() = 0;

    /// Apply any pending state changes
    /// Some implementations may defer state changes for batching
    virtual void ApplyDirtyStates() = 0;

    // ========== Controller Access ==========
    // Advanced users may need direct access to controllers

    /// Get the depth state controller
    [[nodiscard]] virtual StateManagement::DepthStateController& Depth() = 0;

    /// Get the blend state controller
    [[nodiscard]] virtual StateManagement::BlendStateController& Blend() = 0;

    /// Get the state stack manager
    [[nodiscard]] virtual StateManagement::StateStackManager& Stack() = 0;
};

} // namespace SAGE
