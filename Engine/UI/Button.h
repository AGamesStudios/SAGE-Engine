#pragma once

#include "Widget.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Font.h"
#include "Memory/Ref.h"

#include <functional>
#include <string>

namespace SAGE {
namespace UI {

/// @brief Button widget with click callback
class Button : public Widget {
public:
    using ClickCallback = std::function<void()>;

    Button() = default;
    explicit Button(const std::string& text) : m_Text(text) {}

    void Render() override;
    void OnClick(const Vector2& mousePos) override;
    void OnHover(const Vector2& mousePos) override;
    
    // New event system overrides
    void OnMousePressed(MousePressedEvent& event) override;
    void OnMouseReleased(MouseReleasedEvent& event) override;
    void OnKeyPressed(KeyPressedEvent& event) override;

    // Text
    void SetText(const std::string& text) { m_Text = text; }
    const std::string& GetText() const { return m_Text; }

    // Colors
    void SetNormalColor(const Color& color) { m_NormalColor = color; }
    void SetHoverColor(const Color& color) { m_HoverColor = color; }
    void SetPressedColor(const Color& color) { m_PressedColor = color; }
    void SetTextColor(const Color& color) { m_TextColor = color; }

    // Callback
    void SetOnClick(ClickCallback callback) { m_OnClickCallback = callback; }

    // State
    void SetPressed(bool pressed) { m_Pressed = pressed; }
    bool IsPressed() const { return m_Pressed; }

    // Font
    void SetFont(Ref<Font> font) { m_Font = font; }
    Ref<Font> GetFont() const { return m_Font; }

private:
    std::string m_Text;
    Color m_NormalColor{0.3f, 0.3f, 0.3f, 1.0f};
    Color m_HoverColor{0.4f, 0.4f, 0.4f, 1.0f};
    Color m_PressedColor{0.2f, 0.2f, 0.2f, 1.0f};
    Color m_TextColor{1.0f, 1.0f, 1.0f, 1.0f};
    
    bool m_Pressed = false;
    Ref<Font> m_Font;  // Will use default if not set
    ClickCallback m_OnClickCallback;
};

} // namespace UI
} // namespace SAGE
