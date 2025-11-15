#pragma once

/**
 * @file InputEvents.h
 * @brief Включает все input события (удобный header)
 * 
 * Использование:
 *   #include "Events/Input/InputEvents.h"
 * 
 * Вместо:
 *   #include "Events/Input/KeyboardEvents.h"
 *   #include "Events/Input/MouseEvents.h"
 *   #include "Events/Input/GamepadEvents.h"
 */

#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "GamepadEvents.h"

namespace SAGE {

/// @brief Хелперы для работы с input событиями

/// @brief Проверить, является ли событие input событием
inline bool IsInputEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::Input) != 0;
}

/// @brief Проверить, является ли событие клавиатурным
inline bool IsKeyboardEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::Keyboard) != 0;
}

/// @brief Проверить, является ли событие мышиным
inline bool IsMouseEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::Mouse) != 0;
}

/// @brief Проверить, является ли событие геймпад
inline bool IsGamepadEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::Gamepad) != 0;
}

/// @brief Конвертировать Key в строку
inline const char* KeyToString(Key key) {
    switch (key) {
        case Key::Unknown: return "Unknown";
        case Key::Space: return "Space";
        case Key::Enter: return "Enter";
        case Key::Escape: return "Escape";
        case Key::Tab: return "Tab";
        case Key::Backspace: return "Backspace";
        case Key::A: return "A";
        case Key::B: return "B";
        case Key::C: return "C";
        case Key::D: return "D";
        case Key::E: return "E";
        case Key::F: return "F";
        case Key::G: return "G";
        case Key::H: return "H";
        case Key::I: return "I";
        case Key::J: return "J";
        case Key::K: return "K";
        case Key::L: return "L";
        case Key::M: return "M";
        case Key::N: return "N";
        case Key::O: return "O";
        case Key::P: return "P";
        case Key::Q: return "Q";
        case Key::R: return "R";
        case Key::S: return "S";
        case Key::T: return "T";
        case Key::U: return "U";
        case Key::V: return "V";
        case Key::W: return "W";
        case Key::X: return "X";
        case Key::Y: return "Y";
        case Key::Z: return "Z";
        case Key::LeftShift: return "LeftShift";
        case Key::RightShift: return "RightShift";
        case Key::LeftControl: return "LeftControl";
        case Key::RightControl: return "RightControl";
        case Key::LeftAlt: return "LeftAlt";
        case Key::RightAlt: return "RightAlt";
        default: return "Unknown";
    }
}

/// @brief Конвертировать MouseButton в строку
inline const char* MouseButtonToString(MouseButton button) {
    switch (button) {
        case MouseButton::Left: return "Left";
        case MouseButton::Right: return "Right";
        case MouseButton::Middle: return "Middle";
        case MouseButton::Button4: return "Button4";
        case MouseButton::Button5: return "Button5";
        default: return "Unknown";
    }
}

/// @brief Конвертировать GamepadButton в строку
inline const char* GamepadButtonToString(GamepadButton button) {
    switch (button) {
        case GamepadButton::A: return "A";
        case GamepadButton::B: return "B";
        case GamepadButton::X: return "X";
        case GamepadButton::Y: return "Y";
        case GamepadButton::LeftBumper: return "LeftBumper";
        case GamepadButton::RightBumper: return "RightBumper";
        case GamepadButton::Back: return "Back";
        case GamepadButton::Start: return "Start";
        case GamepadButton::Guide: return "Guide";
        case GamepadButton::LeftThumb: return "LeftThumb";
        case GamepadButton::RightThumb: return "RightThumb";
        case GamepadButton::DPadUp: return "DPadUp";
        case GamepadButton::DPadRight: return "DPadRight";
        case GamepadButton::DPadDown: return "DPadDown";
        case GamepadButton::DPadLeft: return "DPadLeft";
        default: return "Unknown";
    }
}

/// @brief Конвертировать GamepadAxis в строку
inline const char* GamepadAxisToString(GamepadAxis axis) {
    switch (axis) {
        case GamepadAxis::LeftX: return "LeftX";
        case GamepadAxis::LeftY: return "LeftY";
        case GamepadAxis::RightX: return "RightX";
        case GamepadAxis::RightY: return "RightY";
        default: return "Unknown";
    }
}

} // namespace SAGE
