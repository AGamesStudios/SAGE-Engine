#pragma once

#include "../Core/Event.h"
#include "Input/MouseButtons.h"

#include <string>
#include <sstream>

namespace SAGE {

/// @brief Базовый класс для всех мышиных событий
class MouseEvent : public Event {
public:
    int GetCategories() const override {
        return EventCategory::Input | EventCategory::Mouse;
    }

protected:
    MouseEvent() = default;
};

/// @brief Событие: кнопка мыши нажата
class MouseButtonPressedEvent : public MouseEvent {
public:
    explicit MouseButtonPressedEvent(MouseButton button, float x = 0.0f, float y = 0.0f)
        : m_Button(button), m_X(x), m_Y(y) {}

    /// @brief Получить код кнопки
    MouseButton GetButton() const { return m_Button; }

    /// @brief Получить позицию X
    float GetX() const { return m_X; }

    /// @brief Получить позицию Y
    float GetY() const { return m_Y; }

    EventType GetType() const override { return EventType::MouseButtonPressed; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseButtonPressedEvent: " << static_cast<int>(m_Button)
            << " at (" << m_X << ", " << m_Y << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_X = 0.0f;
        m_Y = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_X;
    float m_Y;
};

/// @brief Событие: кнопка мыши отпущена
class MouseButtonReleasedEvent : public MouseEvent {
public:
    explicit MouseButtonReleasedEvent(MouseButton button, float x = 0.0f, float y = 0.0f)
        : m_Button(button), m_X(x), m_Y(y) {}

    MouseButton GetButton() const { return m_Button; }
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }

    EventType GetType() const override { return EventType::MouseButtonReleased; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseButtonReleasedEvent: " << static_cast<int>(m_Button)
            << " at (" << m_X << ", " << m_Y << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_X = 0.0f;
        m_Y = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_X;
    float m_Y;
};

/// @brief Событие: мышь перемещена
class MouseMovedEvent : public MouseEvent {
public:
    MouseMovedEvent(float x, float y, float deltaX = 0.0f, float deltaY = 0.0f)
        : m_X(x), m_Y(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

    /// @brief Получить абсолютную позицию X
    float GetX() const { return m_X; }

    /// @brief Получить абсолютную позицию Y
    float GetY() const { return m_Y; }

    /// @brief Получить смещение X
    float GetDeltaX() const { return m_DeltaX; }

    /// @brief Получить смещение Y
    float GetDeltaY() const { return m_DeltaY; }

    EventType GetType() const override { return EventType::MouseMoved; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseMovedEvent: (" << m_X << ", " << m_Y << ")";
        if (m_DeltaX != 0.0f || m_DeltaY != 0.0f) {
            oss << " delta: (" << m_DeltaX << ", " << m_DeltaY << ")";
        }
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_X = 0.0f;
        m_Y = 0.0f;
        m_DeltaX = 0.0f;
        m_DeltaY = 0.0f;
    }

private:
    float m_X;
    float m_Y;
    float m_DeltaX;
    float m_DeltaY;
};

/// @brief Событие: колесико мыши прокручено
class MouseScrolledEvent : public MouseEvent {
public:
    MouseScrolledEvent(float xOffset, float yOffset, float x = 0.0f, float y = 0.0f)
        : m_XOffset(xOffset), m_YOffset(yOffset), m_X(x), m_Y(y) {}

    /// @brief Получить смещение по X (горизонтальная прокрутка)
    float GetXOffset() const { return m_XOffset; }

    /// @brief Получить смещение по Y (вертикальная прокрутка)
    float GetYOffset() const { return m_YOffset; }

    /// @brief Получить позицию курсора X
    float GetX() const { return m_X; }

    /// @brief Получить позицию курсора Y
    float GetY() const { return m_Y; }

    EventType GetType() const override { return EventType::MouseScrolled; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseScrolledEvent: (" << m_XOffset << ", " << m_YOffset << ")";
        if (m_X != 0.0f || m_Y != 0.0f) {
            oss << " at (" << m_X << ", " << m_Y << ")";
        }
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_XOffset = 0.0f;
        m_YOffset = 0.0f;
        m_X = 0.0f;
        m_Y = 0.0f;
    }

private:
    float m_XOffset;
    float m_YOffset;
    float m_X;
    float m_Y;
};

/// @brief Событие: курсор вошел в окно
class MouseEnteredEvent : public MouseEvent {
public:
    MouseEnteredEvent() = default;

    EventType GetType() const override { return EventType::MouseEntered; }

    std::string ToString() const override {
        return "MouseEnteredEvent";
    }
};

/// @brief Событие: курсор покинул окно
class MouseLeftEvent : public MouseEvent {
public:
    MouseLeftEvent() = default;

    EventType GetType() const override { return EventType::MouseLeft; }

    std::string ToString() const override {
        return "MouseLeftEvent";
    }
};

/// @brief Событие: двойной клик
class MouseDoubleClickEvent : public MouseEvent {
public:
    MouseDoubleClickEvent(MouseButton button, float x, float y)
        : m_Button(button), m_X(x), m_Y(y) {}

    MouseButton GetButton() const { return m_Button; }
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }

    EventType GetType() const override { return EventType::MouseDoubleClick; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseDoubleClickEvent: " << static_cast<int>(m_Button)
            << " at (" << m_X << ", " << m_Y << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_X = 0.0f;
        m_Y = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_X;
    float m_Y;
};

/// @brief Событие: начало drag операции
class MouseDragStartedEvent : public MouseEvent {
public:
    MouseDragStartedEvent(MouseButton button, float startX, float startY)
        : m_Button(button), m_StartX(startX), m_StartY(startY) {}

    MouseButton GetButton() const { return m_Button; }
    float GetStartX() const { return m_StartX; }
    float GetStartY() const { return m_StartY; }

    EventType GetType() const override { return EventType::MouseDragStarted; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseDragStartedEvent: " << static_cast<int>(m_Button)
            << " from (" << m_StartX << ", " << m_StartY << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_StartX = 0.0f;
        m_StartY = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_StartX;
    float m_StartY;
};

/// @brief Событие: drag в процессе
class MouseDraggedEvent : public MouseEvent {
public:
    MouseDraggedEvent(MouseButton button, float x, float y, float deltaX, float deltaY)
        : m_Button(button), m_X(x), m_Y(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

    MouseButton GetButton() const { return m_Button; }
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }
    float GetDeltaX() const { return m_DeltaX; }
    float GetDeltaY() const { return m_DeltaY; }

    EventType GetType() const override { return EventType::MouseDragged; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseDraggedEvent: " << static_cast<int>(m_Button)
            << " to (" << m_X << ", " << m_Y << ")"
            << " delta: (" << m_DeltaX << ", " << m_DeltaY << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_X = 0.0f;
        m_Y = 0.0f;
        m_DeltaX = 0.0f;
        m_DeltaY = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_X;
    float m_Y;
    float m_DeltaX;
    float m_DeltaY;
};

/// @brief Событие: завершение drag операции
class MouseDragEndedEvent : public MouseEvent {
public:
    MouseDragEndedEvent(MouseButton button, float endX, float endY)
        : m_Button(button), m_EndX(endX), m_EndY(endY) {}

    MouseButton GetButton() const { return m_Button; }
    float GetEndX() const { return m_EndX; }
    float GetEndY() const { return m_EndY; }

    EventType GetType() const override { return EventType::MouseDragEnded; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseDragEndedEvent: " << static_cast<int>(m_Button)
            << " at (" << m_EndX << ", " << m_EndY << ")";
        return oss.str();
    }

    void Reset() override {
        MouseEvent::Reset();
        m_Button = MouseButton::Left;
        m_EndX = 0.0f;
        m_EndY = 0.0f;
    }

private:
    MouseButton m_Button;
    float m_EndX;
    float m_EndY;
};

} // namespace SAGE
