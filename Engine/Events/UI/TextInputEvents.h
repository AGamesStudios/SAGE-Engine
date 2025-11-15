#pragma once

#include "../Core/Event.h"
#include "Math/Vector2.h"

#include <string>
#include <sstream>

namespace SAGE {

// Forward declarations
namespace UI { class Widget; }

/// @brief Базовый класс для событий текстового ввода
class TextInputEventBase : public Event {
public:
    /// @brief Получить виджет-источник
    UI::Widget* GetSource() const { return m_Source; }
    
    /// @brief Установить источник
    void SetSource(UI::Widget* source) { m_Source = source; }
    
    int GetCategories() const override {
        return EventCategory::UI | EventCategory::Input | EventCategory::Keyboard;
    }
    
    void Reset() override {
        Event::Reset();
        m_Source = nullptr;
    }

protected:
    TextInputEventBase() = default;
    
    UI::Widget* m_Source = nullptr;
};

/// @brief Событие: символ введен в текстовое поле
class TextCharInputEvent : public TextInputEventBase {
public:
    TextCharInputEvent() = default;
    
    explicit TextCharInputEvent(unsigned int codepoint)
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
    
    EventType GetType() const override { return EventType::TextInput; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextCharInputEvent: U+" << std::hex << m_Codepoint;
        if (m_Codepoint >= 32 && m_Codepoint < 127) {
            oss << " ('" << static_cast<char>(m_Codepoint) << "')";
        }
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_Codepoint = 0;
    }

private:
    unsigned int m_Codepoint = 0;
};

/// @brief Событие: текст изменен
class TextChangedEvent : public TextInputEventBase {
public:
    TextChangedEvent() = default;
    
    TextChangedEvent(const std::string& oldText, const std::string& newText)
        : m_OldText(oldText), m_NewText(newText) {}
    
    /// @brief Получить старый текст
    const std::string& GetOldText() const { return m_OldText; }
    
    /// @brief Получить новый текст
    const std::string& GetNewText() const { return m_NewText; }
    
    /// @brief Получить разницу в длине
    int GetDeltaLength() const { 
        return static_cast<int>(m_NewText.length()) - static_cast<int>(m_OldText.length()); 
    }
    
    EventType GetType() const override { return EventType::TextChanged; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextChangedEvent: \"" << m_OldText << "\" -> \"" << m_NewText << "\"";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_OldText.clear();
        m_NewText.clear();
    }

private:
    std::string m_OldText;
    std::string m_NewText;
};

/// @brief Событие: текст отправлен (Enter)
class TextSubmittedEvent : public TextInputEventBase {
public:
    TextSubmittedEvent() = default;
    
    explicit TextSubmittedEvent(const std::string& text)
        : m_Text(text) {}
    
    /// @brief Получить отправленный текст
    const std::string& GetText() const { return m_Text; }
    
    EventType GetType() const override { return EventType::TextSubmitted; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextSubmittedEvent: \"" << m_Text << "\"";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_Text.clear();
    }

private:
    std::string m_Text;
};

/// @brief Событие: позиция курсора изменилась
class TextCursorMovedEvent : public TextInputEventBase {
public:
    TextCursorMovedEvent() = default;
    
    TextCursorMovedEvent(int oldPosition, int newPosition)
        : m_OldPosition(oldPosition), m_NewPosition(newPosition) {}
    
    /// @brief Получить старую позицию курсора
    int GetOldPosition() const { return m_OldPosition; }
    
    /// @brief Получить новую позицию курсора
    int GetNewPosition() const { return m_NewPosition; }
    
    /// @brief Получить смещение
    int GetDelta() const { return m_NewPosition - m_OldPosition; }
    
    EventType GetType() const override { return EventType::TextCursorMoved; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextCursorMovedEvent: " << m_OldPosition << " -> " << m_NewPosition;
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_OldPosition = 0;
        m_NewPosition = 0;
    }

private:
    int m_OldPosition = 0;
    int m_NewPosition = 0;
};

/// @brief Событие: выделение текста изменилось
class TextSelectionChangedEvent : public TextInputEventBase {
public:
    TextSelectionChangedEvent() = default;
    
    TextSelectionChangedEvent(int start, int end)
        : m_SelectionStart(start), m_SelectionEnd(end) {}
    
    /// @brief Получить начало выделения
    int GetSelectionStart() const { return m_SelectionStart; }
    
    /// @brief Получить конец выделения
    int GetSelectionEnd() const { return m_SelectionEnd; }
    
    /// @brief Получить длину выделения
    int GetSelectionLength() const { 
        return std::abs(m_SelectionEnd - m_SelectionStart); 
    }
    
    /// @brief Проверить, есть ли выделение
    bool HasSelection() const { return m_SelectionStart != m_SelectionEnd; }
    
    EventType GetType() const override { return EventType::TextSelectionChanged; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextSelectionChangedEvent: [" << m_SelectionStart 
            << ", " << m_SelectionEnd << "]";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_SelectionStart = 0;
        m_SelectionEnd = 0;
    }

private:
    int m_SelectionStart = 0;
    int m_SelectionEnd = 0;
};

/// @brief Событие: копирование текста
class TextCopiedEvent : public TextInputEventBase {
public:
    TextCopiedEvent() = default;
    
    explicit TextCopiedEvent(const std::string& text)
        : m_Text(text) {}
    
    /// @brief Получить скопированный текст
    const std::string& GetText() const { return m_Text; }
    
    EventType GetType() const override { return EventType::TextCopied; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextCopiedEvent: \"" << m_Text << "\"";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_Text.clear();
    }

private:
    std::string m_Text;
};

/// @brief Событие: вставка текста
class TextPastedEvent : public TextInputEventBase {
public:
    TextPastedEvent() = default;
    
    explicit TextPastedEvent(const std::string& text)
        : m_Text(text) {}
    
    /// @brief Получить вставленный текст
    const std::string& GetText() const { return m_Text; }
    
    EventType GetType() const override { return EventType::TextPasted; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextPastedEvent: \"" << m_Text << "\"";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_Text.clear();
    }

private:
    std::string m_Text;
};

/// @brief Событие: вырезание текста
class TextCutEvent : public TextInputEventBase {
public:
    TextCutEvent() = default;
    
    explicit TextCutEvent(const std::string& text)
        : m_Text(text) {}
    
    /// @brief Получить вырезанный текст
    const std::string& GetText() const { return m_Text; }
    
    EventType GetType() const override { return EventType::TextCut; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "TextCutEvent: \"" << m_Text << "\"";
        return oss.str();
    }
    
    void Reset() override {
        TextInputEventBase::Reset();
        m_Text.clear();
    }

private:
    std::string m_Text;
};

/// @brief Событие: отмена (Undo)
class TextUndoEvent : public TextInputEventBase {
public:
    TextUndoEvent() = default;
    
    EventType GetType() const override { return EventType::TextUndo; }
    
    std::string ToString() const override {
        return "TextUndoEvent";
    }
};

/// @brief Событие: повтор (Redo)
class TextRedoEvent : public TextInputEventBase {
public:
    TextRedoEvent() = default;
    
    EventType GetType() const override { return EventType::TextRedo; }
    
    std::string ToString() const override {
        return "TextRedoEvent";
    }
};

} // namespace SAGE
