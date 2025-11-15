#pragma once

#include "Widget.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Memory/Ref.h"

namespace SAGE {
namespace UI {

/// @brief Panel widget - rectangular background container
class Panel : public Widget {
public:
    Panel() = default;

    void Render() override;

    // Background color
    void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; }
    Color GetBackgroundColor() const { return m_BackgroundColor; }

    // Border
    void SetBorderColor(const Color& color) { m_BorderColor = color; }
    Color GetBorderColor() const { return m_BorderColor; }

    void SetBorderWidth(float width) { m_BorderWidth = width; }
    float GetBorderWidth() const { return m_BorderWidth; }

    // Texture (optional)
    void SetTexture(Ref<Texture> texture) { m_Texture = texture; }
    Ref<Texture> GetTexture() const { return m_Texture; }

private:
    Color m_BackgroundColor{0.2f, 0.2f, 0.2f, 0.9f};
    Color m_BorderColor{0.5f, 0.5f, 0.5f, 1.0f};
    float m_BorderWidth = 0.0f;
    Ref<Texture> m_Texture;
};

} // namespace UI
} // namespace SAGE
