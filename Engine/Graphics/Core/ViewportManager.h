#pragma once

#include "Math/Vector2.h"
#include "Graphics/Core/Types/MathTypes.h"
#include <functional>
#include <vector>

namespace SAGE {

/// @brief Callback type for viewport change notifications
using ViewportChangedCallback = std::function<void(const Rect& viewport)>;

/// @brief Centralized viewport management system
/// Coordinates viewport changes between Window, Renderer, and Camera systems
class ViewportManager {
public:
    ViewportManager() = default;
    ~ViewportManager() = default;

    // Disable copy
    ViewportManager(const ViewportManager&) = delete;
    ViewportManager& operator=(const ViewportManager&) = delete;

    /// @brief Set viewport dimensions and notify all registered callbacks
    /// @param x Viewport X position (typically 0)
    /// @param y Viewport Y position (typically 0)
    /// @param width Viewport width in pixels
    /// @param height Viewport height in pixels
    void SetViewport(int x, int y, int width, int height);

    /// @brief Get current viewport bounds
    /// @return Rect containing viewport dimensions
    Rect GetViewport() const { return m_Viewport; }

    /// @brief Get viewport width in pixels
    int GetWidth() const { return static_cast<int>(m_Viewport.width); }

    /// @brief Get viewport height in pixels
    int GetHeight() const { return static_cast<int>(m_Viewport.height); }

    /// @brief Get viewport size as Vector2
    Vector2 GetSize() const { return Vector2(m_Viewport.width, m_Viewport.height); }

    /// @brief Get viewport aspect ratio (width / height)
    float GetAspectRatio() const {
        return m_Viewport.height > 0.0f ? m_Viewport.width / m_Viewport.height : 1.0f;
    }

    /// @brief Called when window is resized
    /// Updates viewport to match new window size
    /// @param width New window width
    /// @param height New window height
    void OnWindowResize(int width, int height);

    /// @brief Register callback for viewport changes
    /// @param callback Function to call when viewport changes
    /// @return Callback ID for later removal
    size_t RegisterCallback(ViewportChangedCallback callback);

    /// @brief Unregister viewport change callback
    /// @param callbackId ID returned from RegisterCallback
    void UnregisterCallback(size_t callbackId);

    /// @brief Clear all registered callbacks
    void ClearCallbacks();

private:
    Rect m_Viewport{ 0.0f, 0.0f, 1280.0f, 720.0f };  // Default viewport
    std::vector<std::pair<size_t, ViewportChangedCallback>> m_Callbacks;
    size_t m_NextCallbackId = 0;

    /// @brief Notify all registered callbacks of viewport change
    void NotifyCallbacks();
};

} // namespace SAGE
