#pragma once

/**
 * @file UIEventBridge.h
 * @brief Мост между старыми UI событиями (Engine/UI/UIEvent.h) и новыми (Events/UI/)
 * 
 * Этот файл обеспечивает обратную совместимость и интеграцию
 */

#include "Events/UI/UIEvents.h"
#include "Core/EventBus.h"
#include "UI/UIEvent.h"
#include "UI/Widget.h"

namespace SAGE {
namespace UI {

/// @brief Адаптер для конвертации старых UI событий в новые
class UIEventBridge {
public:
    UIEventBridge(EventBus& eventBus) : m_EventBus(eventBus) {}
    
    /// @brief Конвертировать старое MousePressedEvent в новое
    void BridgeMousePressed(const MousePressedEvent& oldEvent, Widget* target) {
        WidgetPressedEvent newEvent(
            Vector2(oldEvent.GetPosition().x, oldEvent.GetPosition().y),
            static_cast<int>(oldEvent.GetButton())
        );
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое MouseReleasedEvent в новое
    void BridgeMouseReleased(const MouseReleasedEvent& oldEvent, Widget* target) {
        WidgetReleasedEvent newEvent(
            Vector2(oldEvent.GetPosition().x, oldEvent.GetPosition().y),
            static_cast<int>(oldEvent.GetButton())
        );
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое MouseMovedEvent в новое
    void BridgeMouseMoved(const MouseMovedEvent& oldEvent, Widget* target) {
        // Можно использовать для hover detection
        WidgetHoveredEvent newEvent(
            Vector2(oldEvent.GetPosition().x, oldEvent.GetPosition().y)
        );
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое MouseEnterEvent в новое
    void BridgeMouseEnter(const MouseEnterEvent& oldEvent, Widget* target) {
        WidgetHoveredEvent newEvent(
            Vector2(oldEvent.GetPosition().x, oldEvent.GetPosition().y)
        );
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое MouseLeaveEvent в новое
    void BridgeMouseLeave(const MouseLeaveEvent& oldEvent, Widget* target) {
        WidgetUnhoveredEvent newEvent;
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое KeyPressedEvent в новое
    void BridgeKeyPressed(const KeyEvent& oldEvent, Widget* target) {
        // Можно создать специализированное событие для UI
        // Пока просто логируем
    }
    
    /// @brief Конвертировать старое FocusEvent в новое
    void BridgeFocus(const FocusEvent& oldEvent, Widget* target, Widget* previous = nullptr) {
        WidgetFocusedEvent newEvent(previous);
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }
    
    /// @brief Конвертировать старое BlurEvent в новое
    void BridgeBlur(const BlurEvent& oldEvent, Widget* target, Widget* next = nullptr) {
        WidgetBlurredEvent newEvent(next);
        newEvent.SetTarget(target);
        m_EventBus.Publish(newEvent);
    }

private:
    EventBus& m_EventBus;
};

/// @brief Менеджер для автоматической публикации UI событий
class UIEventPublisher {
public:
    UIEventPublisher(EventBus& eventBus) : m_EventBus(eventBus) {}
    
    /// @brief Опубликовать событие клика
    void PublishClick(Widget* target, const Vector2& position, int button = 0, int clickCount = 1) {
        WidgetClickedEvent event(position, button, clickCount);
        event.SetTarget(target);
        event.SetLocalPosition(position);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие изменения значения
    void PublishValueChanged(Widget* target) {
        WidgetValueChangedEvent event;
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие изменения размера
    void PublishResize(Widget* target, const Vector2& oldSize, const Vector2& newSize) {
        WidgetResizedEvent event(oldSize, newSize);
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие перемещения
    void PublishMove(Widget* target, const Vector2& oldPos, const Vector2& newPos) {
        WidgetMovedEvent event(oldPos, newPos);
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие показа
    void PublishShow(Widget* target) {
        WidgetShownEvent event;
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие скрытия
    void PublishHide(Widget* target) {
        WidgetHiddenEvent event;
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие включения
    void PublishEnable(Widget* target) {
        WidgetEnabledEvent event;
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }
    
    /// @brief Опубликовать событие выключения
    void PublishDisable(Widget* target) {
        WidgetDisabledEvent event;
        event.SetTarget(target);
        m_EventBus.Publish(event);
    }

private:
    EventBus& m_EventBus;
};

} // namespace UI
} // namespace SAGE
