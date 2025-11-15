#pragma once

#include "Core/Events/Event.h"
#include "Math/Vector2.h"

namespace SAGE {

/// @brief Event fired when camera position changes
class CameraMovedEvent : public Event {
public:
    CameraMovedEvent(const Vector2& position, const Vector2& previousPosition)
        : m_Position(position)
        , m_PreviousPosition(previousPosition)
    {
    }

    Vector2 GetPosition() const { return m_Position; }
    Vector2 GetPreviousPosition() const { return m_PreviousPosition; }
    Vector2 GetDelta() const { return m_Position - m_PreviousPosition; }

    EVENT_CLASS_TYPE(CameraMoved)
    EVENT_CLASS_CATEGORY(EventCategoryCamera)

private:
    Vector2 m_Position;
    Vector2 m_PreviousPosition;
};

/// @brief Event fired when camera zoom changes
class CameraZoomedEvent : public Event {
public:
    CameraZoomedEvent(float zoom, float previousZoom)
        : m_Zoom(zoom)
        , m_PreviousZoom(previousZoom)
    {
    }

    float GetZoom() const { return m_Zoom; }
    float GetPreviousZoom() const { return m_PreviousZoom; }
    float GetZoomDelta() const { return m_Zoom - m_PreviousZoom; }
    float GetZoomFactor() const { return m_Zoom / m_PreviousZoom; }

    EVENT_CLASS_TYPE(CameraZoomed)
    EVENT_CLASS_CATEGORY(EventCategoryCamera)

private:
    float m_Zoom;
    float m_PreviousZoom;
};

/// @brief Event fired when camera rotation changes
class CameraRotatedEvent : public Event {
public:
    CameraRotatedEvent(float rotation, float previousRotation)
        : m_Rotation(rotation)
        , m_PreviousRotation(previousRotation)
    {
    }

    float GetRotation() const { return m_Rotation; }
    float GetPreviousRotation() const { return m_PreviousRotation; }
    float GetRotationDelta() const { return m_Rotation - m_PreviousRotation; }

    EVENT_CLASS_TYPE(CameraRotated)
    EVENT_CLASS_CATEGORY(EventCategoryCamera)

private:
    float m_Rotation;
    float m_PreviousRotation;
};

} // namespace SAGE
