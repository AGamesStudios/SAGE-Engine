#include "SAGE/UI/UIComponents.h"
#include "SAGE/Graphics/RenderBackend.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Texture.h"
#include "SAGE/Input/Input.h"
#include <algorithm>

namespace SAGE {

// --- Image ---
Image::Image() {
    m_Color = Color::White();
}

void Image::Draw(RenderBackend* renderer) {
    if (!m_IsVisible) return;
    
    Vector2 globalPos = GetGlobalPosition();
    Vector2 centerPos = globalPos + m_Size * 0.5f;
    
    if (m_Texture) {
        if (m_PreserveAspect) {
            // Calculate aspect ratio
            float aspect = (float)m_Texture->GetWidth() / (float)m_Texture->GetHeight();
            Vector2 drawSize = m_Size;
            if (m_Size.x / m_Size.y > aspect) {
                drawSize.x = m_Size.y * aspect;
            } else {
                drawSize.y = m_Size.x / aspect;
            }
            Renderer::DrawQuad(centerPos, drawSize, m_Color, m_Texture.get());
        } else {
            Renderer::DrawQuad(centerPos, m_Size, m_Color, m_Texture.get());
        }
    } else {
        Renderer::DrawQuad(centerPos, m_Size, m_Color);
    }
    
    // Draw children
    for (auto& child : m_Children) {
        child->Draw(renderer);
    }
}

// --- ProgressBar ---
ProgressBar::ProgressBar() {
    m_Color = Color(0.2f, 0.2f, 0.2f, 1.0f); // Background
}

void ProgressBar::SetValue(float value) {
    m_Value = std::clamp(value, 0.0f, 1.0f);
}

void ProgressBar::Draw(RenderBackend* /*renderer*/) {
    if (!m_IsVisible) return;

    Vector2 globalPos = GetGlobalPosition();
    Vector2 centerPos = globalPos + m_Size * 0.5f;

    // Background
    Renderer::DrawQuad(centerPos, m_Size, m_Color);

    // Fill
    if (m_Value > 0.0f) {
        Vector2 fillSize = m_Size;
        fillSize.x *= m_Value;
        // Align left
        Vector2 fillPos = globalPos + Vector2(fillSize.x * 0.5f, m_Size.y * 0.5f);
        Renderer::DrawQuad(fillPos, fillSize, m_FillColor);
    }
    
    // Border (optional, from Widget)
    if (m_BorderThickness > 0.0f) {
        Renderer::DrawRect(centerPos, m_Size, Color::Transparent(), m_BorderThickness, m_BorderColor);
    }
}

// --- InputField ---
InputField::InputField() {
    m_Color = Color(0.1f, 0.1f, 0.1f, 1.0f);
    m_BorderColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
    m_BorderThickness = 1.0f;
    m_Size = Vector2(200, 30);
    m_TextColor = Color::White();
}

bool InputField::OnMouseDown(int button) {
    if (button == 0) { // Left click
        Vector2 mousePos = Input::GetMousePosition();
        Vector2 globalPos = GetGlobalPosition();
        
        // Calculate relative X inside the text area
        float padding = 5.0f;
        float relativeX = mousePos.x - (globalPos.x + padding) + m_ScrollOffset;
        
        // Find index
        auto font = TextRenderer::GetDefaultFont();
        if (font && !m_Text.empty()) {
            // Iterate and measure
            float minDiff = std::abs(relativeX); // Distance to 0
            int bestIndex = 0;
            
            int index = 0;
            while (index < (int)m_Text.length()) {
                int nextIndex = index + 1;
                while (nextIndex < (int)m_Text.length() && (m_Text[nextIndex] & 0xC0) == 0x80) {
                    nextIndex++;
                }
                
                std::string sub = m_Text.substr(0, nextIndex);
                if (m_IsPassword) sub = std::string(nextIndex, '*');
                
                Vector2 size = font->MeasureText(sub);
                float diff = std::abs(relativeX - size.x);
                
                if (diff < minDiff) {
                    minDiff = diff;
                    bestIndex = nextIndex;
                }
                
                index = nextIndex;
            }
            
            m_CursorIndex = bestIndex;
        } else {
            m_CursorIndex = 0;
        }
        
        m_CursorBlinkTimer = 0.0f;
        m_CursorVisible = true;
    }
    return true; // Consumed
}

void InputField::Update(float dt) {
    Widget::Update(dt);
    if (m_IsFocused) {
        m_CursorBlinkTimer += dt;
        if (m_CursorBlinkTimer >= 0.5f) {
            m_CursorBlinkTimer = 0.0f;
            m_CursorVisible = !m_CursorVisible;
        }
    } else {
        m_CursorVisible = false;
    }
}

bool InputField::OnKeyDown(int key) {
    if (!m_IsFocused) return false;
    
    KeyCode keyCode = static_cast<KeyCode>(key);

    // Reset blink timer on interaction
    m_CursorBlinkTimer = 0.0f;
    m_CursorVisible = true;

    if (keyCode == KeyCode::Left) {
        if (m_CursorIndex > 0) {
            m_CursorIndex--;
            while (m_CursorIndex > 0 && (m_Text[m_CursorIndex] & 0xC0) == 0x80) {
                m_CursorIndex--;
            }
        }
        return true;
    }
    
    if (keyCode == KeyCode::Right) {
        if (m_CursorIndex < (int)m_Text.length()) {
            m_CursorIndex++;
            while (m_CursorIndex < (int)m_Text.length() && (m_Text[m_CursorIndex] & 0xC0) == 0x80) {
                m_CursorIndex++;
            }
        }
        return true;
    }

    if (keyCode == KeyCode::Home) {
        m_CursorIndex = 0;
        return true;
    }

    if (keyCode == KeyCode::End) {
        m_CursorIndex = (int)m_Text.length();
        return true;
    }

    if (keyCode == KeyCode::Delete) {
        if (m_CursorIndex < (int)m_Text.length()) {
            int nextIndex = m_CursorIndex + 1;
            while (nextIndex < (int)m_Text.length() && (m_Text[nextIndex] & 0xC0) == 0x80) {
                nextIndex++;
            }
            m_Text.erase(m_CursorIndex, nextIndex - m_CursorIndex);
            if (OnValueChanged) OnValueChanged(m_Text);
        }
        return true;
    }

    if (keyCode == KeyCode::Backspace) {
        if (m_CursorIndex > 0) {
            int prevIndex = m_CursorIndex - 1;
            while (prevIndex > 0 && (m_Text[prevIndex] & 0xC0) == 0x80) {
                prevIndex--;
            }
            m_Text.erase(prevIndex, m_CursorIndex - prevIndex);
            m_CursorIndex = prevIndex;
            if (OnValueChanged) OnValueChanged(m_Text);
        }
        return true;
    }

    if (keyCode == KeyCode::Enter || keyCode == KeyCode::KPEnter) {
        if (OnSubmit) OnSubmit(m_Text);
        // Lose focus?
        return true;
    }
    return false;
}

bool InputField::OnCharInput(unsigned int codepoint) {
    if (!m_IsFocused) return false;
    
    if (m_Text.length() < m_MaxLength) {
        std::string newChar;
        if (codepoint < 0x80) {
            newChar += (char)codepoint;
        } else if (codepoint < 0x800) {
            newChar += (char)(0xC0 | (codepoint >> 6));
            newChar += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint < 0x10000) {
            newChar += (char)(0xE0 | (codepoint >> 12));
            newChar += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            newChar += (char)(0x80 | (codepoint & 0x3F));
        } else {
             newChar += (char)(0xF0 | (codepoint >> 18));
             newChar += (char)(0x80 | ((codepoint >> 12) & 0x3F));
             newChar += (char)(0x80 | ((codepoint >> 6) & 0x3F));
             newChar += (char)(0x80 | (codepoint & 0x3F));
        }
        
        m_Text.insert(m_CursorIndex, newChar);
        m_CursorIndex += (int)newChar.length();
        
        if (OnValueChanged) OnValueChanged(m_Text);
    }
    return true;
}

void InputField::Draw(RenderBackend* /*renderer*/) {
    if (!m_IsVisible) return;

    Vector2 globalPos = GetGlobalPosition();
    Vector2 centerPos = globalPos + m_Size * 0.5f;

    // Background
    Renderer::DrawQuad(centerPos, m_Size, m_Color);
    
    // Border
    Color borderColor = m_IsFocused ? Color::Green() : m_BorderColor;
    Renderer::DrawRect(centerPos, m_Size, Color::Transparent(), m_BorderThickness, borderColor);

    // Text
    std::string displayText = m_IsPassword ? std::string(m_Text.length(), '*') : m_Text;
    bool isPlaceholder = false;
    if (displayText.empty()) {
        displayText = m_Placeholder;
        isPlaceholder = true;
    }

    auto font = TextRenderer::GetDefaultFont();
    if (font) {
        float fontSize = static_cast<float>(font->GetFontSize());
        float padding = 5.0f;
        float availableWidth = m_Size.x - (padding * 2.0f);
        
        Vector2 textSize = font->MeasureText(displayText);
        
        // Calculate scroll offset
        if (!isPlaceholder && m_IsFocused) {
            // Measure text up to cursor
            std::string textBeforeCursor = displayText.substr(0, m_CursorIndex);
            if (m_IsPassword) textBeforeCursor = std::string(m_CursorIndex, '*');
            
            Vector2 cursorSize = font->MeasureText(textBeforeCursor);
            float cursorX = cursorSize.x;
            
            if (cursorX - m_ScrollOffset > availableWidth) {
                m_ScrollOffset = cursorX - availableWidth;
            } else if (cursorX - m_ScrollOffset < 0.0f) {
                m_ScrollOffset = cursorX;
            }
            
            // Clamp scroll
            if (m_ScrollOffset < 0.0f) m_ScrollOffset = 0.0f;
            if (textSize.x <= availableWidth) m_ScrollOffset = 0.0f;
        } else {
            m_ScrollOffset = 0.0f;
        }

        Vector2 textPos = globalPos;
        textPos.x += padding - m_ScrollOffset;
        
        // Vertical centering
        // TextRenderer draws from baseline.
        // Top = (BoxHeight - FontSize) / 2
        // Baseline = Top + FontSize (approx)
        textPos.y += (m_Size.y - fontSize) * 0.5f + fontSize;
        
        Color textColor = isPlaceholder ? Color(0.5f, 0.5f, 0.5f, 1.0f) : m_TextColor;
        
        // Clip text to input field area
        Renderer::PushScissor((int)globalPos.x, (int)globalPos.y, (int)m_Size.x, (int)m_Size.y);
        
        TextRenderer::DrawText(displayText, textPos, textColor, font);
        
        // Cursor
        if (m_IsFocused && m_CursorVisible) {
            Vector2 cursorPos = textPos;
            
            if (!isPlaceholder) {
                std::string textBeforeCursor = displayText.substr(0, m_CursorIndex);
                if (m_IsPassword) textBeforeCursor = std::string(m_CursorIndex, '*');
                Vector2 cursorSize = font->MeasureText(textBeforeCursor);
                cursorPos.x += cursorSize.x;
            }
            
            // Draw cursor
            // Cursor center should be at baseline - fontSize/2 (middle of text)
            Renderer::DrawQuad(cursorPos + Vector2(1, -fontSize/2), Vector2(2, fontSize), m_TextColor);
        }
        
        Renderer::PopScissor();
    }
}

} // namespace SAGE
