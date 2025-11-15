#pragma once

#include "../Core/Event.h"
#include "Input/GamepadButtons.h"

#include <string>
#include <sstream>

namespace SAGE {

/// @brief Базовый класс для всех геймпад событий
class GamepadEvent : public Event {
public:
    /// @brief Получить ID геймпада (0-15)
    int GetGamepadID() const { return m_GamepadID; }

    int GetCategories() const override {
        return EventCategory::Input | EventCategory::Gamepad;
    }

protected:
    explicit GamepadEvent(int gamepadID) : m_GamepadID(gamepadID) {}

    int m_GamepadID;
};

/// @brief Событие: кнопка геймпада нажата
class GamepadButtonPressedEvent : public GamepadEvent {
public:
    GamepadButtonPressedEvent(int gamepadID, GamepadButton button)
        : GamepadEvent(gamepadID), m_Button(button) {}

    /// @brief Получить код кнопки
    GamepadButton GetButton() const { return m_Button; }

    EventType GetType() const override { return EventType::GamepadButtonPressed; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadButtonPressedEvent: Gamepad[" << m_GamepadID << "] "
            << "Button " << static_cast<int>(m_Button);
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_Button = GamepadButton::A;
    }

private:
    GamepadButton m_Button;
};

/// @brief Событие: кнопка геймпада отпущена
class GamepadButtonReleasedEvent : public GamepadEvent {
public:
    GamepadButtonReleasedEvent(int gamepadID, GamepadButton button)
        : GamepadEvent(gamepadID), m_Button(button) {}

    GamepadButton GetButton() const { return m_Button; }

    EventType GetType() const override { return EventType::GamepadButtonReleased; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadButtonReleasedEvent: Gamepad[" << m_GamepadID << "] "
            << "Button " << static_cast<int>(m_Button);
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_Button = GamepadButton::A;
    }

private:
    GamepadButton m_Button;
};

/// @brief Событие: ось геймпада изменилась
class GamepadAxisEvent : public GamepadEvent {
public:
    GamepadAxisEvent(int gamepadID, GamepadAxis axis, float value, float deadzone = 0.0f)
        : GamepadEvent(gamepadID), m_Axis(axis), m_Value(value), m_Deadzone(deadzone) {
        // Применяем deadzone
        if (std::abs(m_Value) < m_Deadzone) {
            m_Value = 0.0f;
        }
    }

    /// @brief Получить тип оси
    GamepadAxis GetAxis() const { return m_Axis; }

    /// @brief Получить значение оси [-1.0, 1.0]
    float GetValue() const { return m_Value; }

    /// @brief Получить deadzone
    float GetDeadzone() const { return m_Deadzone; }

    /// @brief Проверить, находится ли значение в мертвой зоне
    bool IsInDeadzone() const { return m_Value == 0.0f && std::abs(m_Value) < m_Deadzone; }

    EventType GetType() const override { return EventType::GamepadAxis; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadAxisEvent: Gamepad[" << m_GamepadID << "] "
            << "Axis " << static_cast<int>(m_Axis)
            << " = " << m_Value;
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_Axis = GamepadAxis::LeftX;
        m_Value = 0.0f;
        m_Deadzone = 0.0f;
    }

private:
    GamepadAxis m_Axis;
    float m_Value;
    float m_Deadzone;
};

/// @brief Событие: триггер геймпада изменился
class GamepadTriggerEvent : public GamepadEvent {
public:
    GamepadTriggerEvent(int gamepadID, GamepadTrigger trigger, float value)
        : GamepadEvent(gamepadID), m_Trigger(trigger), m_Value(value) {}

    /// @brief Получить тип триггера
    GamepadTrigger GetTrigger() const { return m_Trigger; }

    /// @brief Получить значение триггера [0.0, 1.0]
    float GetValue() const { return m_Value; }

    /// @brief Проверить, полностью ли нажат триггер
    bool IsFullyPressed() const { return m_Value >= 0.99f; }

    EventType GetType() const override { return EventType::GamepadTrigger; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadTriggerEvent: Gamepad[" << m_GamepadID << "] "
            << "Trigger " << static_cast<int>(m_Trigger)
            << " = " << m_Value;
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_Trigger = GamepadTrigger::Left;
        m_Value = 0.0f;
    }

private:
    GamepadTrigger m_Trigger;
    float m_Value;
};

/// @brief Событие: геймпад подключен
class GamepadConnectedEvent : public GamepadEvent {
public:
    GamepadConnectedEvent(int gamepadID, const char* name = nullptr)
        : GamepadEvent(gamepadID), m_Name(name ? name : "Unknown Gamepad") {}

    /// @brief Получить имя геймпада
    const std::string& GetName() const { return m_Name; }

    EventType GetType() const override { return EventType::GamepadConnected; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadConnectedEvent: Gamepad[" << m_GamepadID << "] "
            << "\"" << m_Name << "\"";
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_Name = "Unknown Gamepad";
    }

private:
    std::string m_Name;
};

/// @brief Событие: геймпад отключен
class GamepadDisconnectedEvent : public GamepadEvent {
public:
    explicit GamepadDisconnectedEvent(int gamepadID)
        : GamepadEvent(gamepadID) {}

    EventType GetType() const override { return EventType::GamepadDisconnected; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadDisconnectedEvent: Gamepad[" << m_GamepadID << "]";
        return oss.str();
    }
};

/// @brief Событие: вибрация геймпада
class GamepadVibrationEvent : public GamepadEvent {
public:
    GamepadVibrationEvent(int gamepadID, float leftMotor, float rightMotor, float duration)
        : GamepadEvent(gamepadID), 
          m_LeftMotor(leftMotor), 
          m_RightMotor(rightMotor),
          m_Duration(duration) {}

    /// @brief Получить силу левого мотора [0.0, 1.0]
    float GetLeftMotor() const { return m_LeftMotor; }

    /// @brief Получить силу правого мотора [0.0, 1.0]
    float GetRightMotor() const { return m_RightMotor; }

    /// @brief Получить длительность вибрации (секунды)
    float GetDuration() const { return m_Duration; }

    EventType GetType() const override { return EventType::GamepadVibration; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "GamepadVibrationEvent: Gamepad[" << m_GamepadID << "] "
            << "Left: " << m_LeftMotor << ", Right: " << m_RightMotor
            << ", Duration: " << m_Duration << "s";
        return oss.str();
    }

    void Reset() override {
        GamepadEvent::Reset();
        m_LeftMotor = 0.0f;
        m_RightMotor = 0.0f;
        m_Duration = 0.0f;
    }

private:
    float m_LeftMotor;
    float m_RightMotor;
    float m_Duration;
};

} // namespace SAGE
