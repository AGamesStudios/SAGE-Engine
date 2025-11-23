#pragma once

#include "SAGE/Core/Event.h"
#include "SAGE/Input/Input.h"

namespace SAGE {

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput))

private:
    float m_MouseX, m_MouseY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput))

private:
    float m_XOffset, m_YOffset;
};

class MouseButtonEvent : public Event {
public:
    MouseButton GetMouseButton() const { return m_Button; }

    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput) | static_cast<int>(EventCategory::EventCategoryMouseButton))

protected:
    MouseButtonEvent(MouseButton button) : m_Button(button) {}
    MouseButton m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(MouseButton button) : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << static_cast<int>(m_Button);
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(MouseButton button) : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << static_cast<int>(m_Button);
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace SAGE
