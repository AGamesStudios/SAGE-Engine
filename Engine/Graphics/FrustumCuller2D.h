#pragma once

#include "Graphics/Core/Types/MathTypes.h" // Provides Vector2 and Rect

namespace SAGE {

class Camera2D; // Forward declaration

/**
 * @brief Frustum culling for 2D camera
 * 
 * Performs visibility tests against camera view bounds.
 * Objects outside camera view are culled before rendering.
 */
class FrustumCuller2D {
public:
    FrustumCuller2D() = default;
    
    /**
     * @brief Update frustum bounds from camera
     */
    void Update(const Camera2D& camera);
    
    /**
     * @brief Test if point is visible
     */
    bool IsPointVisible(const Vector2& point) const {
        return point.x >= m_MinX && point.x <= m_MaxX &&
               point.y >= m_MinY && point.y <= m_MaxY;
    }
    
    /**
     * @brief Test if circle is visible
     */
    bool IsCircleVisible(const Vector2& center, float radius) const {
        return center.x + radius >= m_MinX && center.x - radius <= m_MaxX &&
               center.y + radius >= m_MinY && center.y - radius <= m_MaxY;
    }
    
    /**
     * @brief Test if axis-aligned rectangle is visible
     */
    bool IsRectVisible(float x, float y, float width, float height) const {
        return x + width >= m_MinX && x <= m_MaxX &&
               y + height >= m_MinY && y <= m_MaxY;
    }
    
    /**
     * @brief Test if Rect is visible
     */
    bool IsRectVisible(const Rect& rect) const {
        return IsRectVisible(rect.x, rect.y, rect.width, rect.height);
    }
    
    /**
     * @brief Get frustum bounds
     */
    void GetBounds(float& minX, float& minY, float& maxX, float& maxY) const {
        minX = m_MinX;
        minY = m_MinY;
        maxX = m_MaxX;
        maxY = m_MaxY;
    }
    
    /**
     * @brief Get frustum width
     */
    float GetWidth() const { return m_MaxX - m_MinX; }
    
    /**
     * @brief Get frustum height
     */
    float GetHeight() const { return m_MaxY - m_MinY; }
    
    /**
     * @brief Get frustum center
     */
    Vector2 GetCenter() const {
        return Vector2((m_MinX + m_MaxX) * 0.5f, (m_MinY + m_MaxY) * 0.5f);
    }
    
private:
    float m_MinX = 0.0f;
    float m_MinY = 0.0f;
    float m_MaxX = 0.0f;
    float m_MaxY = 0.0f;
};

} // namespace SAGE
