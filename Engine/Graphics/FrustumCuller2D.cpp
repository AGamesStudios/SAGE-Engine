#include "FrustumCuller2D.h"
#include "Graphics/Core/Camera2D.h"

namespace SAGE {

void FrustumCuller2D::Update(const Camera2D& camera) {
    // Get camera properties
    float width = camera.GetViewportWidth();
    float height = camera.GetViewportHeight();
    float zoom = camera.GetZoom();
    Vector2 pos = camera.GetPosition();
    
    // Calculate visible bounds in world space
    // Half-widths account for camera position and zoom
    float halfWidth = (width * 0.5f) / zoom;
    float halfHeight = (height * 0.5f) / zoom;
    
    m_MinX = pos.x - halfWidth;
    m_MaxX = pos.x + halfWidth;
    m_MinY = pos.y - halfHeight;
    m_MaxY = pos.y + halfHeight;
    
    // TODO: Handle camera rotation by expanding bounds or using oriented box
    // For now, axis-aligned bounds work for most 2D games
}

} // namespace SAGE
