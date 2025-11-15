#include "CameraController.h"
#include "Math/Constants.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace SAGE {

CameraController::CameraController()
    : m_Camera()
    , m_TargetPosition(m_Camera.position)
{
}

CameraController::CameraController(const Camera2D& camera)
    : m_Camera(camera)
    , m_TargetPosition(camera.position)
{
}

void CameraController::Update(float deltaTime) {
    // Update shake effect
    UpdateShake(deltaTime);

    // Smooth following
    if (m_FollowingTarget) {
        Vector2 delta = m_TargetPosition - m_Camera.position;
        m_Camera.position += delta * m_SmoothFactor;

        // Stop following when close enough
        if (delta.LengthSquared() < 0.01f) {
            m_Camera.position = m_TargetPosition;
            m_FollowingTarget = false;
        }
    }

    // Apply bounds constraints
    if (m_HasBounds) {
        ApplyBounds();
    }
}

void CameraController::SetTarget(const Vector2& target, float smoothness) {
    m_TargetPosition = target;
    m_SmoothFactor = std::clamp(smoothness, 0.0f, 1.0f);
    m_FollowingTarget = true;
}

void CameraController::SetBounds(const Rect& bounds) {
    m_Bounds = bounds;
    m_HasBounds = true;
    ApplyBounds();
}

void CameraController::ClearBounds() {
    m_HasBounds = false;
}

void CameraController::Shake(float amplitude, float duration) {
    m_ShakeAmplitude = amplitude;
    m_ShakeDuration = duration;
    m_ShakeTimer = duration;
}

void CameraController::StopShake() {
    m_ShakeTimer = 0.0f;
    m_ShakeOffset = Vector2::Zero();
}

void CameraController::Zoom(float factor, const Vector2& pivot) {
    float newZoom = m_Camera.zoom * factor;
    SetZoom(newZoom);
}

void CameraController::SetZoom(float zoom) {
    m_Camera.SetZoom(zoom);
}

void CameraController::SetPosition(const Vector2& position) {
    m_Camera.position = position;
    m_TargetPosition = position;
    m_FollowingTarget = false;

    if (m_HasBounds) {
        ApplyBounds();
    }
}

void CameraController::SetRotation(float radians) {
    m_Camera.SetRotation(radians);
}

void CameraController::Rotate(float deltaRadians) {
    m_Camera.Rotate(deltaRadians);
}

Camera2D CameraController::GetCameraWithShake() const {
    Camera2D shakeCam = m_Camera;
    shakeCam.position += m_ShakeOffset;
    return shakeCam;
}

void CameraController::ApplyBounds() {
    float halfWidth = (m_Camera.viewportWidth / m_Camera.zoom) * 0.5f;
    float halfHeight = (m_Camera.viewportHeight / m_Camera.zoom) * 0.5f;

    // Clamp camera position to bounds
    m_Camera.position.x = std::clamp(m_Camera.position.x,
        m_Bounds.x + halfWidth,
        m_Bounds.x + m_Bounds.width - halfWidth);

    m_Camera.position.y = std::clamp(m_Camera.position.y,
        m_Bounds.y + halfHeight,
        m_Bounds.y + m_Bounds.height - halfHeight);
}

void CameraController::UpdateShake(float deltaTime) {
    if (m_ShakeTimer <= 0.0f) {
        m_ShakeOffset = Vector2::Zero();
        return;
    }

    m_ShakeTimer -= deltaTime;

    // Generate random shake offset
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    float intensity = m_ShakeTimer / m_ShakeDuration;  // Fade out over time
    float currentAmplitude = m_ShakeAmplitude * intensity;

    m_ShakeOffset.x = dis(gen) * currentAmplitude;
    m_ShakeOffset.y = dis(gen) * currentAmplitude;

    if (m_ShakeTimer <= 0.0f) {
        m_ShakeOffset = Vector2::Zero();
    }
}

} // namespace SAGE
