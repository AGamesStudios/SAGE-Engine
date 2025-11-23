#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Matrix3.h"

namespace SAGE {

class Camera2D {
public:
    Camera2D() = default;
    Camera2D(float viewportWidth, float viewportHeight);

    void SetPosition(const Vector2& position);
    void SetZoom(float zoom);
    void SetRotation(float rotation);
    void SetViewportSize(float width, float height);

    float GetViewportWidth() const { return m_ViewportWidth; }
    float GetViewportHeight() const { return m_ViewportHeight; }

    const Vector2& GetPosition() const { return m_Position; }
    float GetZoom() const { return m_Zoom; }
    float GetRotation() const { return m_Rotation; }

    Matrix3 GetViewMatrix() const;
    Matrix3 GetProjectionMatrix() const;
    Matrix3 GetViewProjectionMatrix() const;

    // Convert screen space to world space
    Vector2 ScreenToWorld(const Vector2& screenPos) const;
    // Convert world space to screen space
    Vector2 WorldToScreen(const Vector2& worldPos) const;

    // Camera effects
    void Shake(float intensity, float duration);
    void SmoothFollow(const Vector2& target, float smoothing, float deltaTime);
    
    // Auto-follow
    void SetFollowTarget(const Vector2* target);
    void SetFollowSmoothness(float smoothness);
    void SetFollowOffset(const Vector2& offset);
    
    void Update(float deltaTime);
    
    // Bounds
    void SetBounds(const Vector2& min, const Vector2& max);
    void ClearBounds();

    enum class Origin {
        Center,
        TopLeft,
        BottomLeft
    };
    void SetOrigin(Origin origin);

private:
    void UpdateMatrices();
    void UpdateShake(float deltaTime);

    float m_ViewportWidth = 0.0f;
    float m_ViewportHeight = 0.0f;
    
    Vector2 m_Position{0.0f, 0.0f};
    float m_Rotation = 0.0f;
    float m_Zoom = 1.0f;
    Origin m_Origin = Origin::Center;

    mutable Matrix3 m_ViewMatrix = Matrix3::Identity();
    mutable Matrix3 m_ProjectionMatrix = Matrix3::Identity();
    mutable Matrix3 m_ViewProjectionMatrix = Matrix3::Identity();
    mutable bool m_MatricesDirty = true;
    
    // Camera shake
    float m_ShakeIntensity = 0.0f;
    float m_ShakeDuration = 0.0f;
    float m_ShakeTimer = 0.0f;
    Vector2 m_ShakeOffset{0.0f, 0.0f};
    
    // Bounds
    bool m_HasBounds = false;
    Vector2 m_BoundsMin{0.0f, 0.0f};
    Vector2 m_BoundsMax{0.0f, 0.0f};

    // Follow
    const Vector2* m_FollowTarget = nullptr;
    float m_FollowSmoothness = 0.0f;
    Vector2 m_FollowOffset{0.0f, 0.0f};
};

} // namespace SAGE
