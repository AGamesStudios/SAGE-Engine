#include "Panel.h"
#include "Graphics/API/Renderer.h"

namespace SAGE {
namespace UI {

void Panel::Render() {
    if (!m_Visible) return;

    // Draw background
    QuadDesc quad;
    quad.position = m_Position;
    quad.size = m_Size;
    quad.color = m_BackgroundColor;
    quad.texture = m_Texture;
    quad.screenSpace = true;
    Renderer::DrawQuad(quad);

    // Draw border if width > 0
    if (m_BorderWidth > 0.0f) {
        // Top border
        QuadDesc top;
        top.position = m_Position;
        top.size = Vector2(m_Size.x, m_BorderWidth);
        top.color = m_BorderColor;
    top.screenSpace = true;
        Renderer::DrawQuad(top);

        // Bottom border
        QuadDesc bottom;
        bottom.position = Vector2(m_Position.x, m_Position.y + m_Size.y - m_BorderWidth);
        bottom.size = Vector2(m_Size.x, m_BorderWidth);
        bottom.color = m_BorderColor;
    bottom.screenSpace = true;
        Renderer::DrawQuad(bottom);

        // Left border
        QuadDesc left;
        left.position = m_Position;
        left.size = Vector2(m_BorderWidth, m_Size.y);
        left.color = m_BorderColor;
    left.screenSpace = true;
        Renderer::DrawQuad(left);

        // Right border
        QuadDesc right;
        right.position = Vector2(m_Position.x + m_Size.x - m_BorderWidth, m_Position.y);
        right.size = Vector2(m_BorderWidth, m_Size.y);
        right.color = m_BorderColor;
    right.screenSpace = true;
        Renderer::DrawQuad(right);
    }
}

} // namespace UI
} // namespace SAGE
