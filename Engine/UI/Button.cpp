#include "Button.h"
#include "FontManager.h"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"

namespace SAGE {
namespace UI {

void Button::Render() {
    if (!m_Visible) return;

    // Choose color based on state
    Color currentColor = m_NormalColor;
    if (m_Pressed) {
        currentColor = m_PressedColor;
    } else if (m_Hovered) {
        currentColor = m_HoverColor;
    }

    // Draw button background
    QuadDesc quad;
    quad.position = m_Position;
    quad.size = m_Size;
    quad.color = currentColor;
    quad.screenSpace = true;
    Renderer::DrawQuad(quad);

    // Draw focus border if focused
    if (m_Focused) {
        const float borderWidth = 2.0f;
        Color focusColor(0.3f, 0.6f, 1.0f, 1.0f);  // Голубой цвет для фокуса
        
        // Top border
        QuadDesc top;
        top.position = Vector2(m_Position.x - borderWidth, m_Position.y - borderWidth);
        top.size = Vector2(m_Size.x + borderWidth * 2, borderWidth);
        top.color = focusColor;
        top.screenSpace = true;
        Renderer::DrawQuad(top);
        
        // Bottom border
        QuadDesc bottom;
        bottom.position = Vector2(m_Position.x - borderWidth, m_Position.y + m_Size.y);
        bottom.size = Vector2(m_Size.x + borderWidth * 2, borderWidth);
        bottom.color = focusColor;
        bottom.screenSpace = true;
        Renderer::DrawQuad(bottom);
        
        // Left border
        QuadDesc left;
        left.position = Vector2(m_Position.x - borderWidth, m_Position.y);
        left.size = Vector2(borderWidth, m_Size.y);
        left.color = focusColor;
        left.screenSpace = true;
        Renderer::DrawQuad(left);
        
        // Right border
        QuadDesc right;
        right.position = Vector2(m_Position.x + m_Size.x, m_Position.y);
        right.size = Vector2(borderWidth, m_Size.y);
        right.color = focusColor;
        right.screenSpace = true;
        Renderer::DrawQuad(right);
    }

    // Draw button text
    if (!m_Text.empty()) {
        // Use custom font or default
        Ref<Font> font = m_Font ? m_Font : FontManager::GetDefaultFont();
        
        // Only render if we have a valid font
        if (font && font->IsLoaded()) {
            const Float2 measuredSize = Renderer::MeasureText(m_Text, font, 0.5f);
            const Vector2 textSize(measuredSize.x, measuredSize.y);

            TextDesc text;
            text.text = m_Text;
            text.position = Vector2(
                m_Position.x + (m_Size.x - textSize.x) * 0.5f,
                m_Position.y + (m_Size.y - textSize.y) * 0.5f
            );
            text.color = m_TextColor;
            text.scale = 0.5f;
            text.font = font;
            text.screenSpace = true;
            Renderer::DrawText(text);
        }
    }
}

void Button::OnClick(const Vector2& mousePos) {
    if (!m_Visible || !m_Enabled) return;

    if (Contains(mousePos)) {
        SAGE_INFO("Button '{}' clicked", m_Text);
        if (m_OnClickCallback) {
            m_OnClickCallback();
        }
    }
}

void Button::OnMousePressed(MousePressedEvent& event) {
    if (!m_Visible || !m_Enabled) return;
    
    if (event.GetButton() == MouseButtonEvent::Button::Left) {
        m_Pressed = true;
        SAGE_INFO("Button '{}' pressed", m_Text);
    }
}

void Button::OnMouseReleased(MouseReleasedEvent& event) {
    if (!m_Visible || !m_Enabled) return;
    
    if (event.GetButton() == MouseButtonEvent::Button::Left && m_Pressed) {
        m_Pressed = false;
        
        // Only trigger click if mouse is still over button
        if (Contains(event.GetPosition())) {
            SAGE_INFO("Button '{}' clicked", m_Text);
            if (m_OnClickCallback) {
                m_OnClickCallback();
            }
        }
    }
}

void Button::OnKeyPressed(KeyPressedEvent& event) {
    if (!m_Visible || !m_Enabled || !m_Focused) return;
    
    // Enter or Space activates the button
    int keyCode = event.GetKeyCode();
    if (keyCode == 257 || keyCode == 32) { // GLFW_KEY_ENTER = 257, GLFW_KEY_SPACE = 32
        SAGE_INFO("Button '{}' activated via keyboard", m_Text);
        if (m_OnClickCallback) {
            m_OnClickCallback();
        }
        event.StopPropagation();
    }
}

void Button::OnHover(const Vector2& mousePos) {
    if (!m_Visible || !m_Enabled) return;

    m_Hovered = Contains(mousePos);
}

} // namespace UI
} // namespace SAGE
