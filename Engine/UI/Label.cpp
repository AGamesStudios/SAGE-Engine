#include "Label.h"
#include "FontManager.h"
#include "Graphics/API/Renderer.h"

namespace SAGE {
namespace UI {

void Label::Render() {
    if (!m_Visible) return;

    if (!m_Text.empty()) {
        // Use custom font or default
        Ref<Font> font = m_Font ? m_Font : FontManager::GetDefaultFont();
        
        // Only render if we have a valid font
        if (font && font->IsLoaded()) {
            TextDesc text;
            text.text = m_Text;
            text.position = m_Position;
            text.color = m_Color;
            text.scale = m_Scale;
            text.font = font;
            text.screenSpace = true;
            Renderer::DrawText(text);
        }
    }
}

} // namespace UI
} // namespace SAGE
