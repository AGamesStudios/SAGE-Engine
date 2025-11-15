#pragma once

#include "Math/Vector2.h"
#include <functional>

namespace SAGE {
namespace UI {

class Widget;

/// @brief Base class for UI events
class UIEvent {
public:
    UIEvent() = default;
    virtual ~UIEvent() = default;
    
    /// @brief Stop event propagation (bubbling/capturing)
    void StopPropagation() { m_PropagationStopped = true; }
    bool IsPropagationStopped() const { return m_PropagationStopped; }
    
    /// @brief Prevent default action
    void PreventDefault() { m_DefaultPrevented = true; }
    bool IsDefaultPrevented() const { return m_DefaultPrevented; }
    
    /// @brief Mark event as handled
    void SetHandled(bool handled = true) { m_Handled = handled; }
    bool IsHandled() const { return m_Handled; }
    
    /// @brief Get target widget
    Widget* GetTarget() const { return m_Target; }
    void SetTarget(Widget* target) { m_Target = target; }
    
protected:
    bool m_PropagationStopped = false;
    bool m_DefaultPrevented = false;
    bool m_Handled = false;
    Widget* m_Target = nullptr;
};

/// @brief Mouse button event
class MouseButtonEvent : public UIEvent {
public:
    enum class Button {
        Left = 0,
        Right = 1,
        Middle = 2
    };
    
    MouseButtonEvent(Button button, const Vector2& position)
        : m_Button(button), m_Position(position) {}
    
    Button GetButton() const { return m_Button; }
    const Vector2& GetPosition() const { return m_Position; }
    
private:
    Button m_Button;
    Vector2 m_Position;
};

/// @brief Mouse pressed event
class MousePressedEvent : public MouseButtonEvent {
public:
    using MouseButtonEvent::MouseButtonEvent;
    
    bool Handled = false;  // Event handled flag for widgets
};

/// @brief Mouse released event
class MouseReleasedEvent : public MouseButtonEvent {
public:
    using MouseButtonEvent::MouseButtonEvent;
    
    bool Handled = false;  // Event handled flag for widgets
};

/// @brief Mouse moved event
class MouseMovedEvent : public UIEvent {
public:
    MouseMovedEvent(const Vector2& position, const Vector2& delta)
        : m_Position(position), m_Delta(delta) {}
    
    const Vector2& GetPosition() const { return m_Position; }
    const Vector2& GetDelta() const { return m_Delta; }
    
private:
    Vector2 m_Position;
    Vector2 m_Delta;
};

/// @brief Mouse enter event (mouse entered widget bounds)
class MouseEnterEvent : public UIEvent {
public:
    MouseEnterEvent(const Vector2& position) : m_Position(position) {}
    const Vector2& GetPosition() const { return m_Position; }
    
private:
    Vector2 m_Position;
};

/// @brief Mouse leave event (mouse left widget bounds)
class MouseLeaveEvent : public UIEvent {
public:
    MouseLeaveEvent(const Vector2& position) : m_Position(position) {}
    const Vector2& GetPosition() const { return m_Position; }
    
private:
    Vector2 m_Position;
};

/// @brief Key event
class KeyEvent : public UIEvent {
public:
    KeyEvent(int keyCode, int mods)
        : m_KeyCode(keyCode), m_Mods(mods) {}
    
    int GetKeyCode() const { return m_KeyCode; }
    int GetMods() const { return m_Mods; }
    
private:
    int m_KeyCode;
    int m_Mods;
};

/// @brief Key pressed event
class KeyPressedEvent : public KeyEvent {
public:
    using KeyEvent::KeyEvent;
};

/// @brief Key released event
class KeyReleasedEvent : public KeyEvent {
public:
    using KeyEvent::KeyEvent;
};

/// @brief Focus event
class FocusEvent : public UIEvent {
public:
    FocusEvent() = default;
};

/// @brief Blur event
class BlurEvent : public UIEvent {
public:
    BlurEvent() = default;
};

// Event callback types
using MouseButtonCallback = std::function<void(MouseButtonEvent&)>;
using MouseMovedCallback = std::function<void(MouseMovedEvent&)>;
using MouseEnterCallback = std::function<void(MouseEnterEvent&)>;
using MouseLeaveCallback = std::function<void(MouseLeaveEvent&)>;
using KeyCallback = std::function<void(KeyEvent&)>;
using FocusCallback = std::function<void(FocusEvent&)>;
using BlurCallback = std::function<void(BlurEvent&)>;

} // namespace UI
} // namespace SAGE
