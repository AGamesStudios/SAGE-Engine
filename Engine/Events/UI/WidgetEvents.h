#pragma once

#include "../Core/Event.h"
#include "Math/Vector2.h"

#include <string>
#include <sstream>

namespace SAGE {

// Forward declarations
namespace UI { class Widget; }

/// @brief Базовый класс для всех UI событий
class UIEventBase : public Event {
public:
    /// @brief Получить целевой виджет
    UI::Widget* GetTarget() const { return m_Target; }
    
    /// @brief Установить целевой виджет
    void SetTarget(UI::Widget* target) { m_Target = target; }
    
    /// @brief Получить позицию в локальных координатах виджета
    Vector2 GetLocalPosition() const { return m_LocalPosition; }
    
    /// @brief Установить локальную позицию
    void SetLocalPosition(const Vector2& pos) { m_LocalPosition = pos; }
    
    int GetCategories() const override {
        return EventCategory::UI;
    }
    
    void Reset() override {
        Event::Reset();
        m_Target = nullptr;
        m_LocalPosition = Vector2(0, 0);
    }

protected:
    UIEventBase() = default;
    
    UI::Widget* m_Target = nullptr;
    Vector2 m_LocalPosition{0, 0};
};

/// @brief Событие: виджет получил фокус
class WidgetFocusedEvent : public UIEventBase {
public:
    WidgetFocusedEvent() = default;
    explicit WidgetFocusedEvent(UI::Widget* previous) 
        : m_PreviousWidget(previous) {}
    
    /// @brief Получить предыдущий виджет с фокусом
    UI::Widget* GetPreviousWidget() const { return m_PreviousWidget; }
    
    EventType GetType() const override { return EventType::UIFocused; }
    
    std::string ToString() const override {
        return "WidgetFocusedEvent";
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_PreviousWidget = nullptr;
    }

private:
    UI::Widget* m_PreviousWidget = nullptr;
};

/// @brief Событие: виджет потерял фокус
class WidgetBlurredEvent : public UIEventBase {
public:
    WidgetBlurredEvent() = default;
    explicit WidgetBlurredEvent(UI::Widget* next)
        : m_NextWidget(next) {}
    
    /// @brief Получить следующий виджет с фокусом
    UI::Widget* GetNextWidget() const { return m_NextWidget; }
    
    EventType GetType() const override { return EventType::UIBlurred; }
    
    std::string ToString() const override {
        return "WidgetBlurredEvent";
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_NextWidget = nullptr;
    }

private:
    UI::Widget* m_NextWidget = nullptr;
};

/// @brief Событие: наведение мыши на виджет
class WidgetHoveredEvent : public UIEventBase {
public:
    explicit WidgetHoveredEvent(const Vector2& position)
        : m_Position(position) {}
    
    const Vector2& GetPosition() const { return m_Position; }
    
    EventType GetType() const override { return EventType::UIHovered; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetHoveredEvent at (" << m_Position.x << ", " << m_Position.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_Position = Vector2(0, 0);
    }

private:
    Vector2 m_Position;
};

/// @brief Событие: мышь покинула виджет
class WidgetUnhoveredEvent : public UIEventBase {
public:
    WidgetUnhoveredEvent() = default;
    
    EventType GetType() const override { return EventType::UIUnhovered; }
    
    std::string ToString() const override {
        return "WidgetUnhoveredEvent";
    }
};

/// @brief Событие: виджет нажат
class WidgetPressedEvent : public UIEventBase {
public:
    WidgetPressedEvent(const Vector2& position, int button = 0)
        : m_Position(position), m_Button(button) {}
    
    const Vector2& GetPosition() const { return m_Position; }
    int GetButton() const { return m_Button; }
    
    EventType GetType() const override { return EventType::UIPressed; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetPressedEvent: Button " << m_Button 
            << " at (" << m_Position.x << ", " << m_Position.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_Position = Vector2(0, 0);
        m_Button = 0;
    }

private:
    Vector2 m_Position;
    int m_Button;
};

/// @brief Событие: виджет отпущен
class WidgetReleasedEvent : public UIEventBase {
public:
    WidgetReleasedEvent(const Vector2& position, int button = 0)
        : m_Position(position), m_Button(button) {}
    
    const Vector2& GetPosition() const { return m_Position; }
    int GetButton() const { return m_Button; }
    
    EventType GetType() const override { return EventType::UIReleased; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetReleasedEvent: Button " << m_Button 
            << " at (" << m_Position.x << ", " << m_Position.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_Position = Vector2(0, 0);
        m_Button = 0;
    }

private:
    Vector2 m_Position;
    int m_Button;
};

/// @brief Событие: клик по виджету (Pressed + Released)
class WidgetClickedEvent : public UIEventBase {
public:
    WidgetClickedEvent(const Vector2& position, int button = 0, int clickCount = 1)
        : m_Position(position), m_Button(button), m_ClickCount(clickCount) {}
    
    const Vector2& GetPosition() const { return m_Position; }
    int GetButton() const { return m_Button; }
    int GetClickCount() const { return m_ClickCount; }
    bool IsDoubleClick() const { return m_ClickCount == 2; }
    
    EventType GetType() const override { return EventType::UIClicked; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetClickedEvent: Button " << m_Button;
        if (m_ClickCount > 1) {
            oss << " (x" << m_ClickCount << ")";
        }
        oss << " at (" << m_Position.x << ", " << m_Position.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_Position = Vector2(0, 0);
        m_Button = 0;
        m_ClickCount = 1;
    }

private:
    Vector2 m_Position;
    int m_Button;
    int m_ClickCount;
};

/// @brief Событие: значение виджета изменилось
class WidgetValueChangedEvent : public UIEventBase {
public:
    WidgetValueChangedEvent() = default;
    
    EventType GetType() const override { return EventType::UIValueChanged; }
    
    std::string ToString() const override {
        return "WidgetValueChangedEvent";
    }
};

/// @brief Событие: виджет показан
class WidgetShownEvent : public UIEventBase {
public:
    WidgetShownEvent() = default;
    
    EventType GetType() const override { return EventType::UIShown; }
    
    std::string ToString() const override {
        return "WidgetShownEvent";
    }
};

/// @brief Событие: виджет скрыт
class WidgetHiddenEvent : public UIEventBase {
public:
    WidgetHiddenEvent() = default;
    
    EventType GetType() const override { return EventType::UIHidden; }
    
    std::string ToString() const override {
        return "WidgetHiddenEvent";
    }
};

/// @brief Событие: виджет включен
class WidgetEnabledEvent : public UIEventBase {
public:
    WidgetEnabledEvent() = default;
    
    EventType GetType() const override { return EventType::UIEnabled; }
    
    std::string ToString() const override {
        return "WidgetEnabledEvent";
    }
};

/// @brief Событие: виджет выключен
class WidgetDisabledEvent : public UIEventBase {
public:
    WidgetDisabledEvent() = default;
    
    EventType GetType() const override { return EventType::UIDisabled; }
    
    std::string ToString() const override {
        return "WidgetDisabledEvent";
    }
};

/// @brief Событие: размер виджета изменился
class WidgetResizedEvent : public UIEventBase {
public:
    WidgetResizedEvent(const Vector2& oldSize, const Vector2& newSize)
        : m_OldSize(oldSize), m_NewSize(newSize) {}
    
    const Vector2& GetOldSize() const { return m_OldSize; }
    const Vector2& GetNewSize() const { return m_NewSize; }
    
    EventType GetType() const override { return EventType::UIResized; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetResizedEvent: ("
            << m_OldSize.x << "x" << m_OldSize.y << ") -> ("
            << m_NewSize.x << "x" << m_NewSize.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_OldSize = Vector2(0, 0);
        m_NewSize = Vector2(0, 0);
    }

private:
    Vector2 m_OldSize;
    Vector2 m_NewSize;
};

/// @brief Событие: позиция виджета изменилась
class WidgetMovedEvent : public UIEventBase {
public:
    WidgetMovedEvent(const Vector2& oldPosition, const Vector2& newPosition)
        : m_OldPosition(oldPosition), m_NewPosition(newPosition) {}
    
    const Vector2& GetOldPosition() const { return m_OldPosition; }
    const Vector2& GetNewPosition() const { return m_NewPosition; }
    Vector2 GetDelta() const { return m_NewPosition - m_OldPosition; }
    
    EventType GetType() const override { return EventType::UIMoved; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WidgetMovedEvent: ("
            << m_OldPosition.x << ", " << m_OldPosition.y << ") -> ("
            << m_NewPosition.x << ", " << m_NewPosition.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        UIEventBase::Reset();
        m_OldPosition = Vector2(0, 0);
        m_NewPosition = Vector2(0, 0);
    }

private:
    Vector2 m_OldPosition;
    Vector2 m_NewPosition;
};

} // namespace SAGE
