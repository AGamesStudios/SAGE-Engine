#include "SAGE/UI/Widget.h"
#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Texture.h"
#include <algorithm>

namespace SAGE {

    Widget::Widget() 
        : m_Position(0.0f, 0.0f)
        , m_Size(100.0f, 100.0f)
        , m_Color(1.0f, 1.0f, 1.0f, 1.0f)
    {
        // Default gradient to white
        for(int i=0; i<4; ++i) m_GradientColors[i] = Color::White();
    }

    void Widget::Update(float dt) {
        if (!m_IsVisible) return;

        for (auto& child : m_Children) {
            child->Update(dt);
        }
    }

    void Widget::Draw(RenderBackend* renderer) {
        if (!m_IsVisible) return;

        Vector2 globalPos = GetGlobalPosition();
        Vector2 centerPos = globalPos + m_Size * 0.5f;

        // Draw Background
        if (m_Texture) {
            Renderer::DrawQuad(centerPos, m_Size, m_Color, m_Texture.get());
        } else if (m_UseGradient) {
            renderer->DrawQuadGradient(centerPos, m_Size, m_GradientColors[0], m_GradientColors[1], m_GradientColors[2], m_GradientColors[3]);
        } else {
            Renderer::DrawQuad(centerPos, m_Size, m_Color);
        }

        // Draw Border
        if (m_BorderThickness > 0.0f) {
            Renderer::DrawRect(centerPos, m_Size, Color::Transparent(), m_BorderThickness, m_BorderColor);
        }

        // Draw Text
        if (!m_Text.empty()) {
            auto font = TextRenderer::GetDefaultFont();
            if (font) {
                Vector2 textSize = font->MeasureText(m_Text);
                Vector2 textPos = globalPos;
                
                // Vertical alignment
                // TextRenderer draws from baseline, so we need to add textSize.y to the top position
                switch (m_VAlign) {
                    case VerticalAlignment::Top:
                        textPos.y += textSize.y + 5.0f; // Padding
                        break;
                    case VerticalAlignment::Middle:
                        textPos.y += (m_Size.y - textSize.y) * 0.5f + textSize.y;
                        break;
                    case VerticalAlignment::Bottom:
                        textPos.y += m_Size.y - 5.0f; // Padding
                        break;
                }
                
                // Horizontal alignment
                switch (m_HAlign) {
                    case HorizontalAlignment::Center:
                        textPos.x += (m_Size.x - textSize.x) * 0.5f;
                        break;
                    case HorizontalAlignment::Right:
                        textPos.x += (m_Size.x - textSize.x) - 5.0f; // Padding
                        break;
                    case HorizontalAlignment::Left:
                    default:
                        textPos.x += 5.0f; // Padding
                        break;
                }
                
                TextRenderer::DrawText(m_Text, textPos, m_TextColor, font);
            }
        }

        for (auto& child : m_Children) {
            child->Draw(renderer);
        }
    }

    void Widget::AddChild(std::shared_ptr<Widget> child) {
        child->m_Parent = this;
        m_Children.push_back(child);
    }

    void Widget::RemoveChild(std::shared_ptr<Widget> child) {
        auto it = std::find(m_Children.begin(), m_Children.end(), child);
        if (it != m_Children.end()) {
            (*it)->m_Parent = nullptr;
            m_Children.erase(it);
        }
    }

    Vector2 Widget::GetGlobalPosition() const {
        Vector2 parentPos = Vector2::Zero();
        Vector2 parentSize = Vector2::Zero(); // We don't know parent size if null, assume 0 or handle root differently

        if (m_Parent) {
            parentPos = m_Parent->GetGlobalPosition();
            parentSize = m_Parent->GetSize();
        } else {
            // Root widget. If we had access to window size, we could use it for anchors.
            // For now, assume root widgets are manually positioned or use a "Root" container.
        }

        Vector2 pos = parentPos;

        switch (m_Anchor) {
            case Anchor::TopLeft:
                pos += m_Position;
                break;
            case Anchor::TopRight:
                pos.x += parentSize.x;
                pos += m_Position; // m_Position.x should be negative
                break;
            case Anchor::BottomLeft:
                pos.y += parentSize.y;
                pos += m_Position; // m_Position.y should be negative
                break;
            case Anchor::BottomRight:
                pos += parentSize;
                pos += m_Position; // Both negative
                break;
            case Anchor::Center:
                pos += parentSize * 0.5f;
                pos -= m_Size * 0.5f; // Center the widget itself
                pos += m_Position; // Offset from center
                break;
            case Anchor::Stretch:
                // Stretch logic usually affects size too, which is complex for GetGlobalPosition.
                // For now treat as TopLeft
                pos += m_Position;
                break;
        }

        return pos;
    }

    void Widget::SetGradient(const Color& c1, const Color& c2, const Color& c3, const Color& c4) {
        m_GradientColors[0] = c1;
        m_GradientColors[1] = c2;
        m_GradientColors[2] = c3;
        m_GradientColors[3] = c4;
        m_UseGradient = true;
    }

    bool Widget::OnMouseEnter() {
        m_IsHovered = true;
        return true;
    }

    bool Widget::OnMouseLeave() {
        m_IsHovered = false;
        m_IsPressed = false;
        return true;
    }

    bool Widget::OnMouseMove(const Vector2&) {
        return false;
    }

    bool Widget::OnMouseDown(int) {
        m_IsPressed = true;
        return true;
    }

    bool Widget::OnMouseUp(int button) {
        if (button == 0 && m_IsPressed) {
            m_IsPressed = false;
            // Only fire click if mouse is still inside
            if (m_IsHovered) {
                OnClick();
                return true;
            }
        }
        m_IsPressed = false;
        return false;
    }

    bool Widget::OnClick() {
        if (OnClickCallback) {
            OnClickCallback();
            return true;
        }
        return false;
    }

    bool Widget::Contains(const Vector2& point) {
        Vector2 globalPos = GetGlobalPosition();
        return point.x >= globalPos.x && point.x <= globalPos.x + m_Size.x &&
               point.y >= globalPos.y && point.y <= globalPos.y + m_Size.y;
    }

    std::shared_ptr<Widget> Widget::GetChildAt(const Vector2& point) {
        // Check children in reverse order (top-most first)
        for (auto it = m_Children.rbegin(); it != m_Children.rend(); ++it) {
            auto child = *it;
            if (child->Contains(point)) {
                // Recursively check if this child has a child at point
                auto grandChild = child->GetChildAt(point);
                if (grandChild) return grandChild;
                return child;
            }
        }
        return nullptr;
    }

    bool Widget::OnKeyDown(int key) { return false; }
    bool Widget::OnKeyUp(int key) { return false; }
    bool Widget::OnCharInput(unsigned int codepoint) { return false; }
    void Widget::OnFocus() {}
    void Widget::OnLostFocus() {}

}
