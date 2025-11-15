#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"
#include "Graphics/Core/Types/RendererTypes.h"
#include "UIEvent.h"

#include <functional>
#include <string>
#include <vector>

namespace SAGE {
namespace UI {

using Vector2 = ::SAGE::Vector2;

/// @brief Base widget class for UI elements
class Widget {
public:
    Widget() = default;
    virtual ~Widget() = default;

    /// @brief Update widget logic
    virtual void Update(float deltaTime) { (void)deltaTime; }

    /// @brief Render widget
    virtual void Render() = 0;

    /// @brief Check if point is inside widget bounds
    virtual bool Contains(const Vector2& point) const;

    /// @brief Handle mouse click (DEPRECATED - use event callbacks instead)
    virtual void OnClick(const Vector2& mousePos) { (void)mousePos; }

    /// @brief Handle mouse hover (DEPRECATED - use OnMouseEnter/OnMouseLeave instead)
    virtual void OnHover(const Vector2& mousePos) { (void)mousePos; }
    
    // ========== Event System (NEW) ==========
    
    /// @brief Dispatch event to this widget and its children (with propagation)
    virtual void DispatchEvent(UIEvent& event);
    
    /// @brief Event handlers (override in derived classes)
    virtual void OnMousePressed(MousePressedEvent& event) { (void)event; }
    virtual void OnMouseReleased(MouseReleasedEvent& event) { (void)event; }
    virtual void OnMouseMoved(MouseMovedEvent& event) { (void)event; }
    virtual void OnMouseEnter(MouseEnterEvent& event) { (void)event; m_Hovered = true; }
    virtual void OnMouseLeave(MouseLeaveEvent& event) { (void)event; m_Hovered = false; }
    virtual void OnKeyPressed(KeyPressedEvent& event) { (void)event; }
    virtual void OnKeyReleased(KeyReleasedEvent& event) { (void)event; }
    
    // Event callbacks (for external code)
    void SetOnMousePressed(MouseButtonCallback callback) { m_OnMousePressedCallback = callback; }
    void SetOnMouseReleased(MouseButtonCallback callback) { m_OnMouseReleasedCallback = callback; }
    void SetOnMouseEnter(MouseEnterCallback callback) { m_OnMouseEnterCallback = callback; }
    void SetOnMouseLeave(MouseLeaveCallback callback) { m_OnMouseLeaveCallback = callback; }
    void SetOnKeyPressed(KeyCallback callback) { m_OnKeyPressedCallback = callback; }
    void SetOnKeyReleased(KeyCallback callback) { m_OnKeyReleasedCallback = callback; }

    // Position and size
    void SetPosition(const Vector2& pos) { m_Position = pos; }
    Vector2 GetPosition() const { return m_Position; }

    void SetSize(const Vector2& size) { m_Size = size; }
    Vector2 GetSize() const { return m_Size; }

    // Visibility
    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

    // Enable/disable
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }

    // Z-Order for layering
    void SetZOrder(int zOrder) { m_ZOrder = zOrder; }
    int GetZOrder() const { return m_ZOrder; }

    // Parent-Child hierarchy
    void SetParent(Widget* parent) { m_Parent = parent; }
    Widget* GetParent() const { return m_Parent; }
    void AddChild(Widget* child);
    void RemoveChild(Widget* child);
    const std::vector<Widget*>& GetChildren() const { return m_Children; }

    // Focus system
    virtual void OnFocus() { m_Focused = true; }
    virtual void OnBlur() { m_Focused = false; }
    bool IsFocused() const { return m_Focused; }
    
    // Hover state
    bool IsHovered() const { return m_Hovered; }

protected:
    Vector2 m_Position{0.0f, 0.0f};
    Vector2 m_Size{100.0f, 50.0f};
    bool m_Visible = true;
    bool m_Enabled = true;
    bool m_Hovered = false;
    bool m_Focused = false;
    int m_ZOrder = 0;
    
    // Hierarchy
    Widget* m_Parent = nullptr;
    std::vector<Widget*> m_Children;
    
    // Event callbacks
    MouseButtonCallback m_OnMousePressedCallback;
    MouseButtonCallback m_OnMouseReleasedCallback;
    MouseEnterCallback m_OnMouseEnterCallback;
    MouseLeaveCallback m_OnMouseLeaveCallback;
    KeyCallback m_OnKeyPressedCallback;
    KeyCallback m_OnKeyReleasedCallback;
};

} // namespace UI
} // namespace SAGE
