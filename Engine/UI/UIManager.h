#pragma once

#include "Core/Core.h"
#include "Widget.h"
#include "UIEvent.h"
#include "Input/InputBridge.h"
#include "Memory/Ref.h"

#include <memory>
#include <vector>

struct GLFWwindow;  // Forward declaration

namespace SAGE {
namespace UI {

/// @brief UI Manager - manages all UI widgets and input events
class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    // Non-copyable, non-movable
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    /// @brief Get singleton instance
    static UIManager& Get();

    /// @brief Initialize UI manager
    /// @param inputBridge Input bridge for event handling
    /// @param window GLFW window for mouse position (optional)
    void Init(InputBridge* inputBridge, GLFWwindow* window = nullptr);

    /// @brief Shutdown UI manager
    void Shutdown();

    /// @brief Update all widgets
    void Update(float deltaTime);

    /// @brief Render all widgets
    void Render();

    /// @brief Add widget to manager
    template<typename T>
    std::shared_ptr<T> AddWidget(std::shared_ptr<T> widget) {
        m_Widgets.push_back(widget);
        m_WidgetsSorted = false;  // Need to re-sort
        return widget;
    }

    /// @brief Remove widget from manager
    void RemoveWidget(std::shared_ptr<Widget> widget);

    /// @brief Clear all widgets
    void Clear();

    /// @brief Get mouse position
    Vector2 GetMousePosition() const { return m_MousePosition; }

    /// @brief Update mouse position manually (optional - UIManager updates it automatically)
    void HandleMouseMove(const Vector2& position);

    /// @brief Set focused widget
    void SetFocusedWidget(Widget* widget);
    
    /// @brief Get focused widget
    Widget* GetFocusedWidget() const { return m_FocusedWidget; }
    
    /// @brief Perform hit test to find widget at position
    Widget* HitTest(const Vector2& position);
    
    /// @brief Handle Tab key for navigation
    void NavigateToNextWidget();
    void NavigateToPreviousWidget();

private:
    void HandleMouseClick(const Vector2& position);
    void SortWidgetsByZOrder();
    
    // Event handlers (called from InputBridge callbacks)
    void OnMouseButtonPressed(int button, const Vector2& position);
    void OnMouseButtonReleased(int button, const Vector2& position);
    void OnMouseMoved(const Vector2& position);
    void OnKeyPressed(int keyCode, int mods);
    void OnKeyReleased(int keyCode, int mods);

    std::vector<std::shared_ptr<Widget>> m_Widgets;
    InputBridge* m_InputBridge = nullptr;
    GLFWwindow* m_Window = nullptr;  // For automatic mouse position updates
    Vector2 m_MousePosition{0.0f, 0.0f};
    Widget* m_FocusedWidget = nullptr;  // Currently focused widget
    Widget* m_HoveredWidget = nullptr;  // Currently hovered widget
    bool m_Initialized = false;
    bool m_WidgetsSorted = false;  // Track if widgets need re-sorting
};

} // namespace UI
} // namespace SAGE
