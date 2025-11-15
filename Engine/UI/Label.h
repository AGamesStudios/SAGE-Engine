#pragma once

#include "Widget.h"
#include "Graphics/Core/Resources/Font.h"
#include "Memory/Ref.h"

#include <string>

namespace SAGE {
namespace UI {

/// @brief Label widget for displaying text
class Label : public Widget {
public:
    Label() = default;
    explicit Label(const std::string& text) : m_Text(text) {}

    void Render() override;

    // Text
    void SetText(const std::string& text) { m_Text = text; }
    const std::string& GetText() const { return m_Text; }

    // Color
    void SetColor(const Color& color) { m_Color = color; }
    Color GetColor() const { return m_Color; }

    // Scale
    void SetScale(float scale) { m_Scale = scale; }
    float GetScale() const { return m_Scale; }

    // Font
    void SetFont(Ref<Font> font) { m_Font = font; }
    Ref<Font> GetFont() const { return m_Font; }

private:
    std::string m_Text;
    Color m_Color{1.0f, 1.0f, 1.0f, 1.0f};
    float m_Scale = 1.0f;
    Ref<Font> m_Font;  // Will use default if not set
};

} // namespace UI
} // namespace SAGE
