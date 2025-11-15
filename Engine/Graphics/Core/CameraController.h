#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Types/MathTypes.h"
#include "Math/Vector2.h"

namespace SAGE {

/// @brief Camera controller for smooth camera movement and effects
class CameraController {
public:
    CameraController();
    explicit CameraController(const Camera2D& camera);

    /// @brief Update camera state (smooth following, shake, etc.)
    /// @param deltaTime Time since last update in seconds
    void Update(float deltaTime);

    /// @brief Set target position for smooth camera following
    /// @param target Target position in world space
    /// @param smoothness Interpolation factor (0 = instant, 1 = very smooth)
    void SetTarget(const Vector2& target, float smoothness = 0.1f);

    /// @brief Set camera bounds (prevents camera from going outside area)
    /// @param bounds Rect defining valid camera area
    void SetBounds(const Rect& bounds);

    /// @brief Remove camera bounds restriction
    void ClearBounds();

    /// @brief Check if camera has bounds set
    bool HasBounds() const { return m_HasBounds; }

    /// @brief Apply screen shake effect
    /// @param amplitude Shake strength (pixels)
    /// @param duration Shake duration (seconds)
    void Shake(float amplitude, float duration);

    /// @brief Stop current shake effect
    void StopShake();

    /// @brief Check if camera is currently shaking
    bool IsShaking() const { return m_ShakeTimer > 0.0f; }

    /// @brief Zoom camera relative to a pivot point
    /// @param factor Zoom multiplier (1.5 = 150%, 0.5 = 50%)
    /// @param pivot Pivot point in screen space
    void Zoom(float factor, const Vector2& pivot);

    /// @brief Set camera zoom directly
    /// @param zoom Zoom level (clamped to valid range)
    void SetZoom(float zoom);

    /// @brief Get current camera state
    const Camera2D& GetCamera() const { return m_Camera; }

    /// @brief Get camera with shake applied
    Camera2D GetCameraWithShake() const;

    /// @brief Set camera position directly (disables smooth following)
    void SetPosition(const Vector2& position);

    /// @brief Set camera rotation
    /// @param radians Rotation angle in radians
    void SetRotation(float radians);

    /// @brief Rotate camera by delta
    /// @param deltaRadians Rotation delta in radians
    void Rotate(float deltaRadians);

private:
    Camera2D m_Camera;
    Vector2 m_TargetPosition;
    float m_SmoothFactor = 0.1f;
    bool m_FollowingTarget = false;

    Rect m_Bounds;
    bool m_HasBounds = false;

    float m_ShakeAmplitude = 0.0f;
    float m_ShakeDuration = 0.0f;
    float m_ShakeTimer = 0.0f;
    Vector2 m_ShakeOffset;

    /// @brief Apply bounds constraints to camera position
    void ApplyBounds();

    /// @brief Update shake effect
    void UpdateShake(float deltaTime);
};

} // namespace SAGE
