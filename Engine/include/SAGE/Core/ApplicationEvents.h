#pragma once

#include "SAGE/Core/Event.h"

namespace SAGE {

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}

    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))

private:
    unsigned int m_Width, m_Height;
};

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
};

class WindowFocusEvent : public Event {
public:
    WindowFocusEvent() = default;

    EVENT_CLASS_TYPE(WindowFocus)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
};

class WindowLostFocusEvent : public Event {
public:
    WindowLostFocusEvent() = default;

    EVENT_CLASS_TYPE(WindowLostFocus)
    EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
};

} // namespace SAGE
