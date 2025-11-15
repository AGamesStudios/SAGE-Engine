#pragma once

#include "../Core/Event.h"
#include "Input/KeyCodes.h"

#include <string>
#include <sstream>

namespace SAGE {

// Alias для совместимости
using KeyCode = Key;

/// @brief Базовый класс для всех клавиатурных событий
class KeyboardEvent : public Event {
public:
    /// @brief Получить код клавиши
    KeyCode GetKeyCode() const { return m_KeyCode; }

    /// @brief Получить модификаторы (Shift, Ctrl, Alt)
    int GetModifiers() const { return m_Modifiers; }

    /// @brief Проверить наличие модификатора
    bool HasModifier(int modifier) const { return (m_Modifiers & modifier) != 0; }

    /// @brief Категория события
    int GetCategories() const override {
        return EventCategory::Input | EventCategory::Keyboard;
    }

    void Reset() override {
        Event::Reset();
        m_KeyCode = KeyCode::Unknown;
        m_Modifiers = 0;
    }

protected:
    KeyboardEvent(KeyCode keyCode, int modifiers)
        : m_KeyCode(keyCode), m_Modifiers(modifiers) {}

    KeyCode m_KeyCode;
    int m_Modifiers;
};

/// @brief Событие: клавиша нажата
class KeyPressedEvent : public KeyboardEvent {
public:
    KeyPressedEvent(KeyCode keyCode, int modifiers = 0, int repeatCount = 0)
        : KeyboardEvent(keyCode, modifiers), m_RepeatCount(repeatCount) {}

    /// @brief Получить количество повторов (для зажатых клавиш)
    int GetRepeatCount() const { return m_RepeatCount; }

    /// @brief Проверить, является ли событие повтором
    bool IsRepeat() const { return m_RepeatCount > 0; }

    EventType GetType() const override { return EventType::KeyPressed; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyPressedEvent: " << static_cast<int>(m_KeyCode);
        if (m_RepeatCount > 0) {
            oss << " (repeat: " << m_RepeatCount << ")";
        }
        if (m_Modifiers != 0) {
            oss << " [mods: " << m_Modifiers << "]";
        }
        return oss.str();
    }

    void Reset() override {
        KeyboardEvent::Reset();
        m_RepeatCount = 0;
    }

private:
    int m_RepeatCount;
};

/// @brief Событие: клавиша отпущена
class KeyReleasedEvent : public KeyboardEvent {
public:
    KeyReleasedEvent(KeyCode keyCode, int modifiers = 0)
        : KeyboardEvent(keyCode, modifiers) {}

    EventType GetType() const override { return EventType::KeyReleased; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyReleasedEvent: " << static_cast<int>(m_KeyCode);
        if (m_Modifiers != 0) {
            oss << " [mods: " << m_Modifiers << "]";
        }
        return oss.str();
    }
};

/// @brief Событие: ввод символа (Unicode)
/// @note Используется для текстового ввода (не для game controls)
class CharInputEvent : public Event {
public:
    explicit CharInputEvent(unsigned int codepoint)
        : m_Codepoint(codepoint) {}

    /// @brief Получить Unicode codepoint
    unsigned int GetCodepoint() const { return m_Codepoint; }

    /// @brief Получить символ как char (для ASCII)
    char GetChar() const { return static_cast<char>(m_Codepoint); }

    /// @brief Получить UTF-8 строку
    std::string GetUTF8() const {
        std::string result;
        
        if (m_Codepoint < 0x80) {
            result += static_cast<char>(m_Codepoint);
        } else if (m_Codepoint < 0x800) {
            result += static_cast<char>(0xC0 | ((m_Codepoint >> 6) & 0x1F));
            result += static_cast<char>(0x80 | (m_Codepoint & 0x3F));
        } else if (m_Codepoint < 0x10000) {
            result += static_cast<char>(0xE0 | ((m_Codepoint >> 12) & 0x0F));
            result += static_cast<char>(0x80 | ((m_Codepoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (m_Codepoint & 0x3F));
        } else if (m_Codepoint < 0x110000) {
            result += static_cast<char>(0xF0 | ((m_Codepoint >> 18) & 0x07));
            result += static_cast<char>(0x80 | ((m_Codepoint >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((m_Codepoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (m_Codepoint & 0x3F));
        }
        
        return result;
    }

    EventType GetType() const override { return EventType::CharInput; }

    int GetCategories() const override {
        return EventCategory::Input | EventCategory::Keyboard;
    }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "CharInputEvent: U+" << std::hex << m_Codepoint;
        if (m_Codepoint >= 32 && m_Codepoint < 127) {
            oss << " ('" << static_cast<char>(m_Codepoint) << "')";
        }
        return oss.str();
    }

    void Reset() override {
        Event::Reset();
        m_Codepoint = 0;
    }

private:
    unsigned int m_Codepoint;
};

/// @brief Событие: комбинация клавиш (для hotkeys)
class KeyChordEvent : public Event {
public:
    KeyChordEvent(KeyCode key1, KeyCode key2, int modifiers = 0)
        : m_Key1(key1), m_Key2(key2), m_Modifiers(modifiers) {}

    KeyCode GetFirstKey() const { return m_Key1; }
    KeyCode GetSecondKey() const { return m_Key2; }
    int GetModifiers() const { return m_Modifiers; }

    EventType GetType() const override { return EventType::KeyChord; }

    int GetCategories() const override {
        return EventCategory::Input | EventCategory::Keyboard;
    }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyChordEvent: " << static_cast<int>(m_Key1) 
            << " + " << static_cast<int>(m_Key2);
        if (m_Modifiers != 0) {
            oss << " [mods: " << m_Modifiers << "]";
        }
        return oss.str();
    }

    void Reset() override {
        Event::Reset();
        m_Key1 = KeyCode::Unknown;
        m_Key2 = KeyCode::Unknown;
        m_Modifiers = 0;
    }

private:
    KeyCode m_Key1;
    KeyCode m_Key2;
    int m_Modifiers;
};

/// @brief Модификаторы клавиш (битовая маска)
namespace KeyModifiers {
    constexpr int None  = 0;
    constexpr int Shift = 1 << 0;
    constexpr int Ctrl  = 1 << 1;
    constexpr int Alt   = 1 << 2;
    constexpr int Super = 1 << 3; // Windows/Command key
}

} // namespace SAGE
