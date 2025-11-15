#include "Widget.h"
#include <algorithm>

namespace SAGE {
namespace UI {

bool Widget::Contains(const Vector2& point) const {
    return point.x >= m_Position.x && point.x <= m_Position.x + m_Size.x &&
           point.y >= m_Position.y && point.y <= m_Position.y + m_Size.y;
}

void Widget::DispatchEvent(UIEvent& event) {
    if (!m_Visible || event.IsPropagationStopped()) return;
    
    event.SetTarget(this);
    
    // ========== CAPTURING PHASE ==========
    // Event goes from root to target (parent → child)
    // Note: Currently we start from this widget, not from root
    // Full capturing phase implementation would require:
    // 1. Widget hierarchy tree traversal from root
    // 2. Ancestor chain collection
    // 3. Reverse iteration for capturing
    // For most use cases, current bubbling implementation is sufficient
    
    // ========== TARGET PHASE ==========
    // Process event on this widget
    if (event.IsPropagationStopped()) return;
    
    // Dispatch to appropriate handler based on event type
    if (auto* mousePressed = dynamic_cast<MousePressedEvent*>(&event)) {
        OnMousePressed(*mousePressed);
        if (m_OnMousePressedCallback && !event.IsPropagationStopped()) {
            m_OnMousePressedCallback(*mousePressed);
        }
    }
    else if (auto* mouseReleased = dynamic_cast<MouseReleasedEvent*>(&event)) {
        OnMouseReleased(*mouseReleased);
        if (m_OnMouseReleasedCallback && !event.IsPropagationStopped()) {
            m_OnMouseReleasedCallback(*mouseReleased);
        }
    }
    else if (auto* mouseMoved = dynamic_cast<MouseMovedEvent*>(&event)) {
        OnMouseMoved(*mouseMoved);
    }
    else if (auto* mouseEnter = dynamic_cast<MouseEnterEvent*>(&event)) {
        OnMouseEnter(*mouseEnter);
        if (m_OnMouseEnterCallback && !event.IsPropagationStopped()) {
            m_OnMouseEnterCallback(*mouseEnter);
        }
    }
    else if (auto* mouseLeave = dynamic_cast<MouseLeaveEvent*>(&event)) {
        OnMouseLeave(*mouseLeave);
        if (m_OnMouseLeaveCallback && !event.IsPropagationStopped()) {
            m_OnMouseLeaveCallback(*mouseLeave);
        }
    }
    else if (auto* keyPressed = dynamic_cast<KeyPressedEvent*>(&event)) {
        OnKeyPressed(*keyPressed);
        if (m_OnKeyPressedCallback && !event.IsPropagationStopped()) {
            m_OnKeyPressedCallback(*keyPressed);
        }
    }
    else if (auto* keyReleased = dynamic_cast<KeyReleasedEvent*>(&event)) {
        OnKeyReleased(*keyReleased);
        if (m_OnKeyReleasedCallback && !event.IsPropagationStopped()) {
            m_OnKeyReleasedCallback(*keyReleased);
        }
    }
    
    // ========== BUBBLING PHASE ==========
    // Event bubbles up to parent (child → parent)
    if (!event.IsPropagationStopped() && m_Parent) {
        m_Parent->DispatchEvent(event);
    }
}

void Widget::AddChild(Widget* child) {
    if (!child || child == this) return;
    
    // Remove from old parent
    if (child->m_Parent) {
        child->m_Parent->RemoveChild(child);
    }
    
    // Add to this widget
    m_Children.push_back(child);
    child->m_Parent = this;
}

void Widget::RemoveChild(Widget* child) {
    if (!child) return;
    
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        (*it)->m_Parent = nullptr;
        m_Children.erase(it);
    }
}

} // namespace UI
} // namespace SAGE
