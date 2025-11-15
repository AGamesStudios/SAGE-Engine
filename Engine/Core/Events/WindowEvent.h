#pragma once

#include "../Event.h"
#include <vector>

namespace SAGE {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height) {}

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }

        std::string ToString() const override {
            return "WindowResizeEvent: " + std::to_string(m_Width) + "x" + std::to_string(m_Height);
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        unsigned int m_Width;
        unsigned int m_Height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class WindowFocusEvent : public Event {
    public:
        WindowFocusEvent(bool focused)
            : m_Focused(focused) {}

        bool IsFocused() const { return m_Focused; }

        std::string ToString() const override {
            return m_Focused ? "WindowFocusEvent: Gained Focus" : "WindowFocusEvent: Lost Focus";
        }

        EVENT_CLASS_TYPE(WindowFocus)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        bool m_Focused;
    };

    class WindowMoveEvent : public Event {
    public:
        WindowMoveEvent(int x, int y)
            : m_PosX(x), m_PosY(y) {}

        int GetX() const { return m_PosX; }
        int GetY() const { return m_PosY; }

        std::string ToString() const override {
            return "WindowMoveEvent: " + std::to_string(m_PosX) + ", " + std::to_string(m_PosY);
        }

        EVENT_CLASS_TYPE(WindowMove)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        int m_PosX, m_PosY;
    };

    class WindowMinimizeEvent : public Event {
    public:
        WindowMinimizeEvent(bool minimized)
            : m_Minimized(minimized) {}

        bool IsMinimized() const { return m_Minimized; }

        std::string ToString() const override {
            return m_Minimized ? "WindowMinimizeEvent: Minimized" : "WindowMinimizeEvent: Restored";
        }

        EVENT_CLASS_TYPE(WindowMinimize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        bool m_Minimized;
    };

    class WindowMaximizeEvent : public Event {
    public:
        WindowMaximizeEvent(bool maximized)
            : m_Maximized(maximized) {}

        bool IsMaximized() const { return m_Maximized; }

        std::string ToString() const override {
            return m_Maximized ? "WindowMaximizeEvent: Maximized" : "WindowMaximizeEvent: Restored";
        }

        EVENT_CLASS_TYPE(WindowMaximize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        bool m_Maximized;
    };

    class WindowContentScaleEvent : public Event {
    public:
        WindowContentScaleEvent(float xScale, float yScale)
            : m_XScale(xScale), m_YScale(yScale) {}

        float GetXScale() const { return m_XScale; }
        float GetYScale() const { return m_YScale; }

        std::string ToString() const override {
            return "WindowContentScaleEvent: " + std::to_string(m_XScale) + "x, " + std::to_string(m_YScale) + "y";
        }

        EVENT_CLASS_TYPE(WindowContentScale)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        float m_XScale, m_YScale;
    };

    class WindowFileDropEvent : public Event {
    public:
        WindowFileDropEvent(const std::vector<std::string>& paths)
            : m_Paths(paths) {}

        const std::vector<std::string>& GetPaths() const { return m_Paths; }
        size_t GetCount() const { return m_Paths.size(); }

        std::string ToString() const override {
            return "WindowFileDropEvent: " + std::to_string(m_Paths.size()) + " file(s)";
        }

        EVENT_CLASS_TYPE(WindowFileDrop)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        std::vector<std::string> m_Paths;
    };

} // namespace SAGE
