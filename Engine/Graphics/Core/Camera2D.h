#pragma once

// CRITICAL: Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Additional safety: undef if already defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "Math/Vector2.h"
#include "Math/Matrix4.h"

#include <algorithm>
#include <cmath>

namespace SAGE {

/// @brief Простая 2D камера для ортогональной проекции.
/// Соглашение: мир центрирован (проекция: [-halfW,+halfW], [-halfH,+halfH]).
/// Zoom применяется только во view-матрице (масштаб), не влияет на проекцию.
/// Rotation хранится в радианах. Pivot определяет точку вокруг которой происходит вращение.
class Camera2D {
public:
    enum class ProjectionMode {
        PixelPerfect,
        FixedWorldHeight,
        FixedWorldWidth
    };

    Camera2D()
        : m_Position(0.0f, 0.0f)
        , m_Zoom(1.0f)
        , m_RotationRadians(0.0f)
        , m_Pivot(0.0f, 0.0f)
        , m_ViewportWidth(800.0f)
        , m_ViewportHeight(600.0f)
        , m_ProjectionMode(ProjectionMode::PixelPerfect)
        , m_FixedWorldHeight(10.0f)
        , m_FixedWorldWidth(10.0f)
        , m_Dirty(true)
    {}

    Camera2D(float viewportWidth, float viewportHeight)
        : m_Position(0.0f, 0.0f)
        , m_Zoom(1.0f)
        , m_RotationRadians(0.0f)
        , m_Pivot(0.0f, 0.0f)
        , m_ViewportWidth(std::max(1.0f, viewportWidth))
        , m_ViewportHeight(std::max(1.0f, viewportHeight))
        , m_ProjectionMode(ProjectionMode::PixelPerfect)
        , m_FixedWorldHeight(10.0f)
        , m_FixedWorldWidth(10.0f)
        , m_Dirty(true)
    {
        if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) {
            std::fprintf(stderr, "Camera2D - Invalid viewport dimensions: %.2f x %.2f, clamped to 1.0\n",
                         viewportWidth, viewportHeight);
        }
    }

    // Позиция камеры
    void SetPosition(const Vector2& position) { m_Position = position; }
    void SetPosition(float x, float y) { m_Position = Vector2(x, y); }
    const Vector2& GetPosition() const { return m_Position; }

    // Зум камеры ( > 0 )
    void SetZoom(float zoom) { m_Zoom = zoom > 0.0f ? zoom : 0.001f; m_Dirty = true; }
    float GetZoom() const { return m_Zoom; }

    // Поворот камеры (в радианах)
    void SetRotationRadians(float radians) { m_RotationRadians = radians; m_Dirty = true; }
    float GetRotationRadians() const { return m_RotationRadians; }
    void SetRotationDegrees(float deg) { SetRotationRadians(deg * 3.14159265358979323846f / 180.0f); }
    float GetRotationDegrees() const { return m_RotationRadians * 180.0f / 3.14159265358979323846f; }

    // Pivot (точка вращения в мировых координатах)
    void SetPivot(const Vector2& pivot) { m_Pivot = pivot; m_Dirty = true; }
    const Vector2& GetPivot() const { return m_Pivot; }

    // Размеры viewport
    void SetViewportSize(float width, float height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        m_Dirty = true;
    }
    void SetViewportWidth(float width) { m_ViewportWidth = width; m_Dirty = true; }
    void SetViewportHeight(float height) { m_ViewportHeight = height; m_Dirty = true; }
    float GetViewportWidth() const { return m_ViewportWidth; }
    float GetViewportHeight() const { return m_ViewportHeight; }

    // Настройка проекции
    void SetProjectionMode(ProjectionMode mode) {
        if (m_ProjectionMode != mode) {
            m_ProjectionMode = mode;
            m_Dirty = true;
        }
    }
    ProjectionMode GetProjectionMode() const { return m_ProjectionMode; }

    void SetFixedWorldHeight(float height) {
        m_FixedWorldHeight = height > 0.0f ? height : 0.0001f;
        if (m_ProjectionMode == ProjectionMode::FixedWorldHeight) {
            m_Dirty = true;
        }
    }
    float GetFixedWorldHeight() const { return m_FixedWorldHeight; }

    void SetFixedWorldWidth(float width) {
        m_FixedWorldWidth = width > 0.0f ? width : 0.0001f;
        if (m_ProjectionMode == ProjectionMode::FixedWorldWidth) {
            m_Dirty = true;
        }
    }
    float GetFixedWorldWidth() const { return m_FixedWorldWidth; }

    // Движение камеры
    void Move(const Vector2& delta) { m_Position += delta; }
    void Move(float dx, float dy) { m_Position += Vector2(dx, dy); }

    // Получить матрицу вида (Translate * PivotRotate * Scale)
    Matrix4 GetViewMatrix() const {
        RecalculateIfDirty();
        return m_View;
    }

    // Получить матрицу проекции (центрированная, НЕ учитывает zoom)
    Matrix4 GetProjectionMatrix() const {
        RecalculateIfDirty();
        return m_Projection;
    }

    // Комбинированная матрица
    Matrix4 GetViewProjectionMatrix() const {
        RecalculateIfDirty();
        return m_ViewProjection;
    }

    // Преобразование экранных координат в мировые
    // Преобразование экранных координат в мировые (через инверсию VP)
    Vector2 ScreenToWorld(const Vector2& screenPos) const {
        RecalculateIfDirty();
        // Приводим к NDC
        float ndcX = (2.0f * screenPos.x) / m_ViewportWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * screenPos.y) / m_ViewportHeight;
        // Используем обратную матрицу viewProjection
        const Matrix4& inv = m_ViewProjectionInverse;
        const float* d = inv.Data();
        // Применяем матрицу (предполагаем z=0)
        float x = d[0] * ndcX + d[4] * ndcY + d[12];
        float y = d[1] * ndcX + d[5] * ndcY + d[13];
        float w = d[3] * ndcX + d[7] * ndcY + d[15];
        
        // Apply perspective divide if needed
        constexpr float kEpsilon = 1e-6f;
        if (std::abs(w - 1.0f) > kEpsilon && std::abs(w) > kEpsilon) {
            x /= w;
            y /= w;
        }
        
        return Vector2(x, y);
    }

    // Преобразование мировых координат в экранные (через VP)
    Vector2 WorldToScreen(const Vector2& worldPos) const {
        RecalculateIfDirty();
        const Matrix4& vp = m_ViewProjection;
        const float* d = vp.Data();
        float x = d[0] * worldPos.x + d[4] * worldPos.y + d[12];
        float y = d[1] * worldPos.x + d[5] * worldPos.y + d[13];
        float w = d[3] * worldPos.x + d[7] * worldPos.y + d[15];
        
        // Apply perspective divide if needed
        constexpr float kEpsilon = 1e-6f;
        if (std::abs(w - 1.0f) > kEpsilon && std::abs(w) > kEpsilon) {
            x /= w;
            y /= w;
        }
        
        // Это уже NDC, переводим в пиксели
        Vector2 screen(
            (x + 1.0f) * 0.5f * m_ViewportWidth,
            (1.0f - y) * 0.5f * m_ViewportHeight
        );
        return screen;
    }

    // Получить границы видимой области в мировых координатах
    struct Bounds {
        float left, right, bottom, top;
    };

    Bounds GetWorldBounds() const {
        float halfWidth = 0.0f;
        float halfHeight = 0.0f;
        ComputeProjectionExtents(halfWidth, halfHeight);
        if (m_Zoom > 0.0f) {
            halfWidth /= m_Zoom;
            halfHeight /= m_Zoom;
        }
        return Bounds{ m_Position.x - halfWidth, m_Position.x + halfWidth, m_Position.y - halfHeight, m_Position.y + halfHeight };
    }

    void MarkDirty() { m_Dirty = true; }

private:
    void RecalculateIfDirty() const {
        if (!m_Dirty) return;
    float halfWidth = 0.0f;
    float halfHeight = 0.0f;
    ComputeProjectionExtents(halfWidth, halfHeight);
    m_Projection = Matrix4::Orthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);
        // View matrix assembled so that vertices experience Translate(-pos) → pivoted rotation → zoom
        Matrix4 view = Matrix4::Identity();

        if (std::abs(m_Zoom - 1.0f) > 0.000001f) {
            view = view * Matrix4::Scale(m_Zoom, m_Zoom, 1.0f);
        }

        if (std::abs(m_RotationRadians) > 0.000001f) {
            const bool hasPivot = (m_Pivot.x != 0.0f || m_Pivot.y != 0.0f);
            if (hasPivot) {
                view = view * Matrix4::Translate(m_Pivot.x, m_Pivot.y, 0.0f);
            }
            view = view * Matrix4::RotateZ(m_RotationRadians);
            if (hasPivot) {
                view = view * Matrix4::Translate(-m_Pivot.x, -m_Pivot.y, 0.0f);
            }
        }

        view = view * Matrix4::Translate(-m_Position.x, -m_Position.y, 0.0f);
        m_View = view;
        m_ViewProjection = m_Projection * m_View;
        m_ViewProjectionInverse = Matrix4::Inverse(m_ViewProjection);
        m_Dirty = false;
    }

    void ComputeProjectionExtents(float& halfWidth, float& halfHeight) const {
        const float safeWidth = std::max(m_ViewportWidth, 1.0f);
        const float safeHeight = std::max(m_ViewportHeight, 1.0f);
        const float aspect = safeWidth / safeHeight;

        switch (m_ProjectionMode) {
        case ProjectionMode::PixelPerfect:
            halfWidth = safeWidth * 0.5f;
            halfHeight = safeHeight * 0.5f;
            break;
        case ProjectionMode::FixedWorldHeight:
            halfHeight = m_FixedWorldHeight * 0.5f;
            halfWidth = halfHeight * aspect;
            break;
        case ProjectionMode::FixedWorldWidth:
            halfWidth = m_FixedWorldWidth * 0.5f;
            halfHeight = halfWidth / aspect;
            break;
        }
    }
    Vector2 m_Position;
    float m_Zoom;
    float m_RotationRadians;
    Vector2 m_Pivot;
    float m_ViewportWidth;
    float m_ViewportHeight;
    ProjectionMode m_ProjectionMode;
    float m_FixedWorldHeight;
    float m_FixedWorldWidth;
    mutable bool m_Dirty;
    mutable Matrix4 m_View;
    mutable Matrix4 m_Projection;
    mutable Matrix4 m_ViewProjection;
    mutable Matrix4 m_ViewProjectionInverse;
};

} // namespace SAGE
