#include "Widgets.h"
#include "Graphics/API/Renderer.h"
#include <algorithm>

namespace SAGE {
namespace UI {

// ==================== SLIDER ====================

Slider::Slider(float x, float y, float width, float min, float max, float initial)
    : m_MinValue(min), m_MaxValue(max), m_Value(std::clamp(initial, min, max)) {
    m_Position = Vector2(x, y);
    m_Size = Vector2(width, 20.0f);
}

void Slider::Render() {
    if (!m_Visible) return;
    
    // Track
    QuadDesc track;
    track.position = Float2(m_Position.x, m_Position.y + m_Size.y * 0.4f);
    track.size = Float2(m_Size.x, m_Size.y * 0.2f);
    track.color = m_TrackColor;
    track.screenSpace = true;
    Renderer::DrawQuad(track);
    
    // Handle
    float t = (m_Value - m_MinValue) / (m_MaxValue - m_MinValue);
    float handleX = m_Position.x + t * m_Size.x;
    
    QuadDesc handle;
    handle.position = Float2(handleX - 5.0f, m_Position.y);
    handle.size = Float2(10.0f, m_Size.y);
    handle.color = m_Hovered || m_Dragging ? m_HandleHoverColor : m_HandleColor;
    handle.screenSpace = true;
    Renderer::DrawQuad(handle);
    
    // Label
    if (!m_Label.empty()) {
        // TODO: Render text with label + value
    }
}

void Slider::Update(float deltaTime) {
    Widget::Update(deltaTime);
}

void Slider::OnMousePressed(MousePressedEvent& event) {
    if (Contains(event.GetPosition())) {
        m_Dragging = true;
        
        float t = (event.GetPosition().x - m_Position.x) / m_Size.x;
        t = std::clamp(t, 0.0f, 1.0f);
        SetValue(m_MinValue + t * (m_MaxValue - m_MinValue));
        
        event.Handled = true;
    }
}

void Slider::OnMouseReleased([[maybe_unused]] MouseReleasedEvent& event) {
    m_Dragging = false;
}

void Slider::OnMouseMoved(MouseMovedEvent& event) {
    if (m_Dragging) {
        float t = (event.GetPosition().x - m_Position.x) / m_Size.x;
        t = std::clamp(t, 0.0f, 1.0f);
        SetValue(m_MinValue + t * (m_MaxValue - m_MinValue));
    }
}

void Slider::SetValue(float value) {
    float oldValue = m_Value;
    m_Value = std::clamp(value, m_MinValue, m_MaxValue);
    
    if (oldValue != m_Value && OnValueChanged) {
        OnValueChanged(m_Value);
    }
}

// ==================== CHECKBOX ====================

Checkbox::Checkbox(float x, float y, float size, bool checked)
    : m_Checked(checked) {
    m_Position = Vector2(x, y);
    m_Size = Vector2(size, size);
}

void Checkbox::Render() {
    if (!m_Visible) return;
    
    // Box
    QuadDesc box;
    box.position = Float2(m_Position.x, m_Position.y);
    box.size = Float2(m_Size.x, m_Size.y);
    box.color = m_BoxColor;
    box.screenSpace = true;
    Renderer::DrawQuad(box);
    
    // Check mark
    if (m_Checked) {
        QuadDesc check;
        check.position = Float2(m_Position.x + m_Size.x * 0.2f, m_Position.y + m_Size.y * 0.2f);
        check.size = Float2(m_Size.x * 0.6f, m_Size.y * 0.6f);
        check.color = m_CheckColor;
        check.screenSpace = true;
        Renderer::DrawQuad(check);
    }
    
    // Label
    if (!m_Label.empty()) {
        // TODO: Render text label
    }
}

void Checkbox::OnMousePressed(MousePressedEvent& event) {
    if (Contains(event.GetPosition())) {
        SetChecked(!m_Checked);
        event.Handled = true;
    }
}

void Checkbox::SetChecked(bool checked) {
    if (m_Checked != checked) {
        m_Checked = checked;
        if (OnToggled) {
            OnToggled(m_Checked);
        }
    }
}

// ==================== DROPDOWN ====================

Dropdown::Dropdown(float x, float y, float width, float height) {
    m_Position = Vector2(x, y);
    m_Size = Vector2(width, height);
}

void Dropdown::Render() {
    if (!m_Visible) return;
    
    // Button
    QuadDesc button;
    button.position = Float2(m_Position.x, m_Position.y);
    button.size = Float2(m_Size.x, m_Size.y);
    button.color = m_ButtonColor;
    button.screenSpace = true;
    Renderer::DrawQuad(button);
    
    // Selected text
    if (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Options.size()) {
        // TODO: Render selected option text
    }
    
    // Dropdown list (if open)
    if (m_IsOpen) {
        float itemHeight = m_Size.y;
        for (int i = 0; i < (int)m_Options.size(); ++i) {
            QuadDesc item;
            item.position = Float2(m_Position.x, m_Position.y + m_Size.y + i * itemHeight);
            item.size = Float2(m_Size.x, itemHeight);
            item.color = (i == m_SelectedIndex) ? m_HighlightColor : m_ListColor;
            item.screenSpace = true;
            Renderer::DrawQuad(item);
            
            // TODO: Render option text
        }
    }
}

void Dropdown::Update(float deltaTime) {
    Widget::Update(deltaTime);
}

void Dropdown::OnMousePressed(MousePressedEvent& event) {
    Vector2 mousePos = event.GetPosition();
    
    // Click on button
    if (Contains(mousePos)) {
        m_IsOpen = !m_IsOpen;
        event.Handled = true;
        return;
    }
    
    // Click on dropdown items
    if (m_IsOpen) {
        float itemHeight = m_Size.y;
        for (int i = 0; i < (int)m_Options.size(); ++i) {
            Vector2 itemPos = m_Position + Vector2(0, m_Size.y + i * itemHeight);
            Vector2 itemSize = Vector2(m_Size.x, itemHeight);
            
            if (mousePos.x >= itemPos.x && mousePos.x <= itemPos.x + itemSize.x &&
                mousePos.y >= itemPos.y && mousePos.y <= itemPos.y + itemSize.y) {
                SetSelectedIndex(i);
                m_IsOpen = false;
                event.Handled = true;
                return;
            }
        }
        
        // Clicked outside - close dropdown
        m_IsOpen = false;
    }
}

void Dropdown::AddOption(const std::string& option) {
    m_Options.push_back(option);
}

void Dropdown::ClearOptions() {
    m_Options.clear();
    m_SelectedIndex = -1;
}

std::string Dropdown::GetSelectedOption() const {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Options.size()) {
        return m_Options[m_SelectedIndex];
    }
    return "";
}

void Dropdown::SetSelectedIndex(int index) {
    if (index >= -1 && index < (int)m_Options.size() && index != m_SelectedIndex) {
        m_SelectedIndex = index;
        if (OnSelectionChanged && m_SelectedIndex >= 0) {
            OnSelectionChanged(m_SelectedIndex, m_Options[m_SelectedIndex]);
        }
    }
}

} // namespace UI
} // namespace SAGE
