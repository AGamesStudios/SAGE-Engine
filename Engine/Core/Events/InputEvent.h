#pragma once

#include "../Event.h"

#include <sstream>
#include <string>

namespace SAGE {

    // Режим курсора (определяется здесь чтобы избежать зависимости от несуществующих файлов)
    enum class CursorMode {
        Normal,
        Locked,
        Hidden,
        Confined
    };

    // Тип устройства геймпада (определяется здесь чтобы избежать зависимости от несуществующих файлов)
    enum class GamepadDeviceType {
        Unknown,
        Xbox,
        PlayStation,
        Nintendo,
        Generic
    };

    inline const char* ToString(GamepadDeviceType type) {
        switch (type) {
        case GamepadDeviceType::Xbox: return "Xbox";
        case GamepadDeviceType::PlayStation: return "PlayStation";
        case GamepadDeviceType::Nintendo: return "Nintendo";
        case GamepadDeviceType::Generic: return "Generic";
        default: return "Unknown";
        }
    }

    class GamepadConnectionEvent : public Event {
    public:
        int GetGamepadId() const { return m_Id; }
        const std::string& GetDeviceName() const { return m_Name; }
        const std::string& GetGuid() const { return m_Guid; }
        GamepadDeviceType GetDeviceType() const { return m_Type; }

        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryGamepad)

    protected:
        GamepadConnectionEvent(int id, std::string name, std::string guid, GamepadDeviceType type)
            : m_Id(id)
            , m_Name(std::move(name))
            , m_Guid(std::move(guid))
            , m_Type(type) {}

        std::string ToBaseString(const char* eventName) const {
            std::stringstream ss;
            ss << eventName << " [id=" << m_Id
               << ", name=" << m_Name
               << ", type=" << SAGE::ToString(m_Type)
               << ", guid=" << m_Guid
               << "]";
            return ss.str();
        }

    private:
        int m_Id;
        std::string m_Name;
        std::string m_Guid;
        GamepadDeviceType m_Type;
    };

    class GamepadConnectedEvent : public GamepadConnectionEvent {
    public:
        GamepadConnectedEvent(int id, std::string name, std::string guid, GamepadDeviceType type)
            : GamepadConnectionEvent(id, std::move(name), std::move(guid), type) {}

        EVENT_CLASS_TYPE(GamepadConnected)

        std::string ToString() const override {
            return ToBaseString("GamepadConnectedEvent");
        }
    };

    class GamepadDisconnectedEvent : public GamepadConnectionEvent {
    public:
        GamepadDisconnectedEvent(int id, std::string name, std::string guid, GamepadDeviceType type)
            : GamepadConnectionEvent(id, std::move(name), std::move(guid), type) {}

        EVENT_CLASS_TYPE(GamepadDisconnected)

        std::string ToString() const override {
            return ToBaseString("GamepadDisconnectedEvent");
        }
    };

    class CursorModeChangedEvent : public Event {
    public:
        CursorModeChangedEvent(CursorMode previousMode, CursorMode newMode)
            : m_Previous(previousMode)
            , m_Current(newMode) {}

        CursorMode GetPreviousMode() const { return m_Previous; }
        CursorMode GetCurrentMode() const { return m_Current; }

        EVENT_CLASS_TYPE(CursorModeChanged)
        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryCursor)

        std::string ToString() const override {
            const auto toString = [](CursorMode mode) -> const char* {
                switch (mode) {
                case CursorMode::Normal: return "Normal";
                case CursorMode::Locked: return "Locked";
                case CursorMode::Hidden: return "Hidden";
                case CursorMode::Confined: return "Confined";
                default: return "Unknown";
                }
            };

            std::stringstream ss;
            ss << "CursorModeChangedEvent [from=" << toString(m_Previous)
               << ", to=" << toString(m_Current) << "]";
            return ss.str();
        }

    private:
        CursorMode m_Previous;
        CursorMode m_Current;
    };

} // namespace SAGE
