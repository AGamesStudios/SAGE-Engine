#pragma once

#include <functional>
#include <ostream>
#include <string>

namespace SAGE {

    enum class EventType {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowMove,
        WindowMinimize,
        WindowMaximize,
        WindowContentScale,
        WindowFileDrop,
        AppTick,
        AppUpdate,
        AppRender,
        GamepadConnected,
        GamepadDisconnected,
        CursorModeChanged,
    PhysicsCollision,
    CollisionBegin,
    CollisionEnd,
    CollisionPreSolve,
    CollisionPostSolve,
    TriggerEnter,
    TriggerExit,
    PhysicsStep,
    PhysicsTransformUpdated,
        CameraMoved,
        CameraZoomed,
        CameraRotated,
        Custom
    };

    enum EventCategory {
        EventCategoryNone = 0,
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4,
        EventCategoryGamepad = 1 << 5,
        EventCategoryCursor = 1 << 6,
        EventCategoryPhysics = 1 << 7,
        EventCategoryCamera = 1 << 8,
        Gameplay = 1 << 9
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
    virtual EventType GetEventType() const override { return GetStaticType(); }\
    virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    class Event {
        friend std::ostream& operator<<(std::ostream& os, const Event& e);
    public:
        virtual ~Event() = default;

        bool Handled = false;

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool IsInCategory(EventCategory category) const {
            return (GetCategoryFlags() & category) != 0;
        }

        // Coalescing support: override to enable deduplication in queue
        virtual bool CanCoalesce() const { return false; }
        virtual std::size_t GetCoalescingKey() const { return 0; }
    };

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event& event)
            : m_Event(event) {}

        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType()) {
                m_Event.Handled = func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

} // namespace SAGE
