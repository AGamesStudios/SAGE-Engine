#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Log.h"
#include <cmath>
#include <random>

namespace SAGE {

namespace {
    std::random_device s_RandomDevice;
    std::mt19937 s_RandomEngine(s_RandomDevice());
}

Camera2D::Camera2D(float viewportWidth, float viewportHeight)
    : m_ViewportWidth(viewportWidth)
    , m_ViewportHeight(viewportHeight)
{
    UpdateMatrices();
}

void Camera2D::SetPosition(const Vector2& position) {
    m_Position = position;
    m_MatricesDirty = true;
}

void Camera2D::SetZoom(float zoom) {
    if (zoom <= 0.0f) {
        SAGE_WARN("Camera2D::SetZoom - Invalid zoom value: {}, using 0.01f", zoom);
        m_Zoom = 0.01f;
    } else {
        m_Zoom = zoom;
    }
    m_MatricesDirty = true;
}

void Camera2D::SetRotation(float rotation) {
    m_Rotation = rotation;
    m_MatricesDirty = true;
}

void Camera2D::SetViewportSize(float width, float height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_MatricesDirty = true;
}

Matrix3 Camera2D::GetViewMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera2D*>(this)->UpdateMatrices();
    }
    return m_ViewMatrix;
}

Matrix3 Camera2D::GetProjectionMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera2D*>(this)->UpdateMatrices();
    }
    return m_ProjectionMatrix;
}

Matrix3 Camera2D::GetViewProjectionMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera2D*>(this)->UpdateMatrices();
    }
    return m_ViewProjectionMatrix;
}

Vector2 Camera2D::ScreenToWorld(const Vector2& screenPos) const {
    if (m_MatricesDirty) {
        const_cast<Camera2D*>(this)->UpdateMatrices();
    }
    
    if (m_ViewportWidth <= 0.0f || m_ViewportHeight <= 0.0f) {
        return Vector2::Zero();
    }
    
    // Convert screen coordinates to NDC (Normalized Device Coordinates)
    // Screen origin is typically top-left, need to adjust based on viewport
    Vector2 ndc = {
        (screenPos.x / m_ViewportWidth) * 2.0f - 1.0f,
        1.0f - (screenPos.y / m_ViewportHeight) * 2.0f  // Flip Y
    };
    
    // Transform: inverse(Projection) * inverse(View)
    Matrix3 invViewProj = m_ViewProjectionMatrix.Inverse();
    return invViewProj.TransformPoint(ndc);
}

Vector2 Camera2D::WorldToScreen(const Vector2& worldPos) const {
    if (m_MatricesDirty) {
        const_cast<Camera2D*>(this)->UpdateMatrices();
    }
    
    if (m_ViewportWidth <= 0.0f || m_ViewportHeight <= 0.0f) {
        return Vector2::Zero();
    }
    
    // Transform world to NDC
    Vector2 ndc = m_ViewProjectionMatrix.TransformPoint(worldPos);
    
    // Convert NDC to screen coordinates
    return {
        (ndc.x + 1.0f) * 0.5f * m_ViewportWidth,
        (1.0f - ndc.y) * 0.5f * m_ViewportHeight  // Flip Y
    };
}

void Camera2D::SetOrigin(Origin origin) {
    m_Origin = origin;
    m_MatricesDirty = true;
}

void Camera2D::UpdateMatrices() {
    // View matrix (camera transform)
    Vector2 effectivePosition = m_Position + m_ShakeOffset;
    Matrix3 translation = Matrix3::Translation(-effectivePosition);
    Matrix3 rotation = Matrix3::Rotation(-m_Rotation);
    Matrix3 scale = Matrix3::Scale(Vector2{m_Zoom, m_Zoom});
    
    m_ViewMatrix = scale * rotation * translation;

    // Projection matrix (orthographic)
    float left = 0.0f, right = 0.0f, bottom = 0.0f, top = 0.0f;

    switch (m_Origin) {
        case Origin::Center:
            left = -m_ViewportWidth * 0.5f;
            right = m_ViewportWidth * 0.5f;
            bottom = -m_ViewportHeight * 0.5f;
            top = m_ViewportHeight * 0.5f;
            break;
        case Origin::TopLeft:
            left = 0.0f;
            right = m_ViewportWidth;
            bottom = m_ViewportHeight; // Y down
            top = 0.0f;
            break;
        case Origin::BottomLeft:
            left = 0.0f;
            right = m_ViewportWidth;
            bottom = 0.0f;
            top = m_ViewportHeight; // Y up
            break;
    }

    m_ProjectionMatrix = Matrix3::Ortho(left, right, bottom, top);

    // Combined
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

    m_MatricesDirty = false;
}

void Camera2D::Shake(float intensity, float duration) {
    m_ShakeIntensity = intensity;
    m_ShakeDuration = duration;
    m_ShakeTimer = 0.0f;
}

void Camera2D::SmoothFollow(const Vector2& target, float smoothing, float deltaTime) {
    // Target is the world position to look at (center of view)
    Vector2 desiredCenter = target;
    
    // Calculate view dimensions in world units
    float viewW = m_ViewportWidth / m_Zoom;
    float viewH = m_ViewportHeight / m_Zoom;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    // Apply bounds to the center position
    if (m_HasBounds) {
        // Clamp center so that [center-halfW, center+halfW] is within [BoundsMin.x, BoundsMax.x]
        // center >= BoundsMin.x + halfW
        // center <= BoundsMax.x - halfW
        float minX = m_BoundsMin.x + halfW;
        float maxX = m_BoundsMax.x - halfW;
        float minY = m_BoundsMin.y + halfH;
        float maxY = m_BoundsMax.y - halfH;

        if (minX > maxX) {
            // Bounds are smaller than view, center in bounds
            desiredCenter.x = (m_BoundsMin.x + m_BoundsMax.x) * 0.5f;
        } else {
            desiredCenter.x = std::max(minX, std::min(desiredCenter.x, maxX));
        }

        if (minY > maxY) {
            desiredCenter.y = (m_BoundsMin.y + m_BoundsMax.y) * 0.5f;
        } else {
            desiredCenter.y = std::max(minY, std::min(desiredCenter.y, maxY));
        }
    }
    
    // Calculate desired m_Position based on Origin
    Vector2 desiredPos = desiredCenter;
    if (m_Origin == Origin::TopLeft) {
        desiredPos.x -= halfW;
        desiredPos.y -= halfH;
    } else if (m_Origin == Origin::BottomLeft) {
        desiredPos.x -= halfW;
        desiredPos.y += halfH; // Assuming Y up
    }
    
    // Smoothly interpolate current position to desired position
    Vector2 newPos = {
        m_Position.x + (desiredPos.x - m_Position.x) * smoothing * deltaTime,
        m_Position.y + (desiredPos.y - m_Position.y) * smoothing * deltaTime
    };
    
    SetPosition(newPos);
}

void Camera2D::SetFollowTarget(const Vector2* target) {
    m_FollowTarget = target;
}

void Camera2D::SetFollowSmoothness(float smoothness) {
    m_FollowSmoothness = smoothness;
}

void Camera2D::SetFollowOffset(const Vector2& offset) {
    m_FollowOffset = offset;
}

void Camera2D::Update(float deltaTime) {
    if (m_FollowTarget) {
        Vector2 targetPos = *m_FollowTarget + m_FollowOffset;
        if (m_FollowSmoothness > 0.0f) {
            SmoothFollow(targetPos, m_FollowSmoothness, deltaTime);
        } else {
            // Instant follow
            // Calculate position to center the target
            Vector2 newPos = targetPos;
            float halfW = (m_ViewportWidth * 0.5f) / m_Zoom;
            float halfH = (m_ViewportHeight * 0.5f) / m_Zoom;

            if (m_Origin == Origin::TopLeft) {
                newPos.x -= halfW;
                newPos.y -= halfH;
            } else if (m_Origin == Origin::BottomLeft) {
                newPos.x -= halfW;
                newPos.y += halfH; // Assuming Y up
            }
            SetPosition(newPos);
        }
    }
    UpdateShake(deltaTime);
}

void Camera2D::UpdateShake(float deltaTime) {
    if (m_ShakeDuration <= 0.0f) {
        m_ShakeOffset = {0.0f, 0.0f};
        return;
    }
    
    m_ShakeTimer += deltaTime;
    
    if (m_ShakeTimer >= m_ShakeDuration) {
        m_ShakeDuration = 0.0f;
        m_ShakeOffset = {0.0f, 0.0f};
        m_MatricesDirty = true;
        return;
    }
    
    // Generate random shake offset
    float progress = m_ShakeTimer / m_ShakeDuration;
    float amplitude = m_ShakeIntensity * (1.0f - progress); // Fade out
    
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    m_ShakeOffset.x = dist(s_RandomEngine) * amplitude;
    m_ShakeOffset.y = dist(s_RandomEngine) * amplitude;
    
    m_MatricesDirty = true;
}

void Camera2D::SetBounds(const Vector2& min, const Vector2& max) {
    m_HasBounds = true;
    m_BoundsMin = min;
    m_BoundsMax = max;
}

void Camera2D::ClearBounds() {
    m_HasBounds = false;
}

} // namespace SAGE
