#include "ViewportManager.h"
#include "Core/Logger.h"

namespace SAGE {

void ViewportManager::SetViewport(int x, int y, int width, int height) {
    // Validate dimensions
    if (width <= 0 || height <= 0) {
        SAGE_WARNING("ViewportManager::SetViewport called with invalid dimensions: {}x{}", width, height);
        return;
    }

    m_Viewport.x = static_cast<float>(x);
    m_Viewport.y = static_cast<float>(y);
    m_Viewport.width = static_cast<float>(width);
    m_Viewport.height = static_cast<float>(height);

    NotifyCallbacks();
}

void ViewportManager::OnWindowResize(int width, int height) {
    SetViewport(0, 0, width, height);
}

size_t ViewportManager::RegisterCallback(ViewportChangedCallback callback) {
    size_t id = m_NextCallbackId++;
    m_Callbacks.emplace_back(id, std::move(callback));
    return id;
}

void ViewportManager::UnregisterCallback(size_t callbackId) {
    auto it = std::remove_if(m_Callbacks.begin(), m_Callbacks.end(),
        [callbackId](const auto& pair) { return pair.first == callbackId; });
    
    if (it != m_Callbacks.end()) {
        m_Callbacks.erase(it, m_Callbacks.end());
    }
}

void ViewportManager::ClearCallbacks() {
    m_Callbacks.clear();
}

void ViewportManager::NotifyCallbacks() {
    for (const auto& [id, callback] : m_Callbacks) {
        if (callback) {
            callback(m_Viewport);
        }
    }
}

} // namespace SAGE
