#include "TextInput.h"
#include "FontManager.h"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace SAGE {
namespace UI {

namespace {
    constexpr int MOD_SHIFT   = 0x0001; // GLFW_MOD_SHIFT
    constexpr int MOD_CONTROL = 0x0002; // GLFW_MOD_CONTROL

    constexpr int KEY_BACKSPACE = 259;   // GLFW_KEY_BACKSPACE
    constexpr int KEY_DELETE    = 261;   // GLFW_KEY_DELETE
    constexpr int KEY_RIGHT     = 262;   // GLFW_KEY_RIGHT
    constexpr int KEY_LEFT      = 263;   // GLFW_KEY_LEFT
    constexpr int KEY_DOWN      = 264;   // GLFW_KEY_DOWN
    constexpr int KEY_UP        = 265;   // GLFW_KEY_UP
    constexpr int KEY_HOME      = 268;   // GLFW_KEY_HOME
    constexpr int KEY_END       = 269;   // GLFW_KEY_END
    constexpr int KEY_ENTER     = 257;   // GLFW_KEY_ENTER
    constexpr int KEY_KP_ENTER  = 335;   // GLFW_KEY_KP_ENTER
    constexpr int KEY_TAB       = 258;   // GLFW_KEY_TAB
    constexpr int KEY_A         = 65;    // GLFW_KEY_A (used for Ctrl+A)
    constexpr int KEY_C         = 67;    // GLFW_KEY_C (Ctrl+C)
    constexpr int KEY_V         = 86;    // GLFW_KEY_V (Ctrl+V)
    constexpr int KEY_X         = 88;    // GLFW_KEY_X (Ctrl+X)

    // Characters that change with shift (US keyboard layout)
    const std::unordered_map<int, char> kShiftSymbolMap {
        { '1', '!' }, { '2', '@' }, { '3', '#' }, { '4', '$' }, { '5', '%' },
        { '6', '^' }, { '7', '&' }, { '8', '*' }, { '9', '(' }, { '0', ')' },
        { '-', '_' }, { '=', '+' }, { '[', '{' }, { ']', '}' }, { ';', ':' },
        { '\'', '"' }, { ',', '<' }, { '.', '>' }, { '/', '?' }, { '\\', '|' }
    };

    inline char ApplyShift(char ch, bool shiftDown) {
        if (!shiftDown) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }

        // Alphabetic characters become uppercase when shift is pressed
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }

        auto it = kShiftSymbolMap.find(ch);
        if (it != kShiftSymbolMap.end()) {
            return it->second;
        }

        return ch;
    }
}

TextInput::TextInput() {
    SetSize(Vector2(220.0f, 36.0f));
}

TextInput::TextInput(const std::string& id)
    : TextInput() {
    (void)id;
}

void TextInput::Update(float deltaTime) {
    Widget::Update(deltaTime);

    if (m_IsFocused) {
        m_CursorBlinkTimer += deltaTime;
        if (m_CursorBlinkTimer >= m_CursorBlinkInterval) {
            m_CursorBlinkTimer = 0.0f;
            m_ShowCursor = !m_ShowCursor;
        }
    }
}

void TextInput::Render() {
    if (!IsVisible()) {
        return;
    }

    DrawBackground();

    if (HasSelection()) {
        DrawSelection();
    }

    DrawText();

    if (m_IsFocused && m_ShowCursor) {
        DrawCursor();
    }
}

void TextInput::OnFocus() {
    Widget::OnFocus();
    m_IsFocused = true;
    ResetCursorBlink();
}

void TextInput::OnBlur() {
    Widget::OnBlur();
    m_IsFocused = false;
    m_ShowCursor = false;
    m_IsSelecting = false;
}

void TextInput::OnMousePressed(MousePressedEvent& event) {
    if (!IsEnabled() || !IsVisible()) {
        return;
    }

    if (event.GetButton() == MouseButtonEvent::Button::Left) {
        int cursor = GetCursorFromPosition(event.GetPosition());
        SetCursorPosition(cursor, false);
        m_SelectionAnchor = m_CursorIndex;
        m_IsSelecting = true;
        event.SetHandled();
        event.Handled = true;
    }
}

void TextInput::OnMouseReleased(MouseReleasedEvent& event) {
    if (!IsEnabled() || !IsVisible()) {
        return;
    }

    if (event.GetButton() == MouseButtonEvent::Button::Left) {
        m_IsSelecting = false;
        event.SetHandled();
        event.Handled = true;
    }
}

void TextInput::OnMouseMoved(MouseMovedEvent& event) {
    if (!IsEnabled() || !IsVisible()) {
        return;
    }

    if (m_IsSelecting) {
        int cursor = GetCursorFromPosition(event.GetPosition());
        SetCursorPosition(cursor, true);
    }
}

void TextInput::OnKeyPressed(KeyPressedEvent& event) {
    if (!IsEnabled() || !IsVisible() || !m_IsFocused) {
        return;
    }

    const int keyCode = event.GetKeyCode();
    const int mods = event.GetMods();

    if (TryHandleShortcut(keyCode, mods)) {
        event.SetHandled();
        event.StopPropagation();
        return;
    }

    if (TryHandleNavigationKey(keyCode, mods)) {
        event.SetHandled();
        event.StopPropagation();
        return;
    }

    if (TryHandleEditingKey(keyCode, mods)) {
        event.SetHandled();
        event.StopPropagation();
        return;
    }

    if (TryHandlePrintableKey(keyCode, mods)) {
        event.SetHandled();
        event.StopPropagation();
        return;
    }
}

void TextInput::SetText(const std::string& text) {
    if (m_MaxLength >= 0 && static_cast<int>(text.length()) > m_MaxLength) {
        m_Text = text.substr(0, m_MaxLength);
    } else {
        m_Text = text;
    }

    m_CursorIndex = static_cast<int>(m_Text.length());
    ClearSelection();
    NotifyTextChanged();
    ResetCursorBlink();
}

void TextInput::SetMaxLength(int maxLength) {
    m_MaxLength = maxLength;
    if (m_MaxLength >= 0 && static_cast<int>(m_Text.length()) > m_MaxLength) {
        m_Text.resize(static_cast<size_t>(m_MaxLength));
        m_CursorIndex = std::min(m_CursorIndex, m_MaxLength);
        ClearSelection();
        NotifyTextChanged();
        ResetCursorBlink();
    }
}

void TextInput::DrawBackground() const {
    const Color background = m_IsFocused ? m_FocusedBackgroundColor : m_BackgroundColor;

    QuadDesc quad;
    quad.position = m_Position;
    quad.size = m_Size;
    quad.color = background;
    quad.screenSpace = true;
    quad.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(quad);

    const Color borderColor = m_IsFocused ? m_FocusedBorderColor : m_BorderColor;

    if (m_BorderWidth <= 0.0f) {
        return;
    }

    QuadDesc top;
    top.position = m_Position;
    top.size = Vector2(m_Size.x, m_BorderWidth);
    top.color = borderColor;
    top.screenSpace = true;
    top.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(top);

    QuadDesc bottom;
    bottom.position = Vector2(m_Position.x, m_Position.y + m_Size.y - m_BorderWidth);
    bottom.size = Vector2(m_Size.x, m_BorderWidth);
    bottom.color = borderColor;
    bottom.screenSpace = true;
    bottom.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(bottom);

    QuadDesc left;
    left.position = m_Position;
    left.size = Vector2(m_BorderWidth, m_Size.y);
    left.color = borderColor;
    left.screenSpace = true;
    left.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(left);

    QuadDesc right;
    right.position = Vector2(m_Position.x + m_Size.x - m_BorderWidth, m_Position.y);
    right.size = Vector2(m_BorderWidth, m_Size.y);
    right.color = borderColor;
    right.screenSpace = true;
    right.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(right);
}

void TextInput::DrawSelection() const {
    if (!HasSelection()) {
        return;
    }

    Ref<Font> font = FontManager::GetDefaultFont();
    if (!font || !font->IsLoaded()) {
        return;
    }

    const float textScale = 0.5f;
    const float padding = 8.0f;
    const Vector2 textOrigin = Vector2(m_Position.x + padding, m_Position.y + (m_Size.y * 0.5f) - 8.0f);

    const int start = SelectionStart();
    const int end = SelectionEnd();

    const std::string displayText = GetDisplayText();
    const std::string prefix = displayText.substr(0, start);
    const std::string selection = displayText.substr(start, end - start);

    const float prefixWidth = Renderer::MeasureText(prefix, font, textScale).x;
    const float selectionWidth = Renderer::MeasureText(selection, font, textScale).x;

    if (selectionWidth <= 0.0f) {
        return;
    }

    QuadDesc selectionQuad;
    selectionQuad.position = Vector2(textOrigin.x + prefixWidth, m_Position.y + 4.0f);
    selectionQuad.size = Vector2(selectionWidth, m_Size.y - 8.0f);
    selectionQuad.color = m_SelectionColor;
    selectionQuad.screenSpace = true;
    selectionQuad.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(selectionQuad);
}

void TextInput::DrawText() const {
    Ref<Font> font = FontManager::GetDefaultFont();
    if (!font || !font->IsLoaded()) {
        return;
    }

    const float textScale = 0.5f;
    const float padding = 8.0f;
    const Vector2 textPosition(m_Position.x + padding, m_Position.y + (m_Size.y * 0.5f) - 8.0f);

    const bool showPlaceholder = m_Text.empty() && !m_Placeholder.empty() && !m_IsFocused;
    const std::string& textToDraw = showPlaceholder ? m_Placeholder : GetDisplayText();
    const Color textColor = showPlaceholder ? m_PlaceholderColor : m_TextColor;

    TextDesc textDesc;
    textDesc.text = textToDraw;
    textDesc.position = textPosition;
    textDesc.color = textColor;
    textDesc.scale = textScale;
    textDesc.font = font;
    textDesc.screenSpace = true;
    Renderer::DrawText(textDesc);
}

void TextInput::DrawCursor() const {
    Ref<Font> font = FontManager::GetDefaultFont();
    if (!font || !font->IsLoaded()) {
        return;
    }

    const float textScale = 0.5f;
    const float padding = 8.0f;
    const Vector2 textOrigin(m_Position.x + padding, m_Position.y + (m_Size.y * 0.5f) - 8.0f);

    const std::string displayText = GetDisplayText();
    const std::string prefix = displayText.substr(0, m_CursorIndex);
    const float prefixWidth = Renderer::MeasureText(prefix, font, textScale).x;

    QuadDesc cursor;
    cursor.position = Vector2(textOrigin.x + prefixWidth, m_Position.y + 6.0f);
    cursor.size = Vector2(1.5f, m_Size.y - 12.0f);
    cursor.color = m_TextColor;
    cursor.screenSpace = true;
    cursor.source = QuadDesc::QuadSource::UI;
    Renderer::DrawQuad(cursor);
}

void TextInput::ResetCursorBlink() {
    m_CursorBlinkTimer = 0.0f;
    m_ShowCursor = true;
}

void TextInput::MoveCursor(int delta, bool selecting) {
    if (!selecting && HasSelection()) {
        const int target = delta < 0 ? SelectionStart() : SelectionEnd();
        SetCursorPosition(target, false);
        return;
    }

    SetCursorPosition(m_CursorIndex + delta, selecting);
}

void TextInput::SetCursorPosition(int position, bool selecting) {
    const int clamped = std::clamp(position, 0, static_cast<int>(m_Text.length()));
    m_CursorIndex = clamped;

    if (!selecting) {
        m_SelectionAnchor = m_CursorIndex;
    }

    ResetCursorBlink();
}

void TextInput::ClearSelection() {
    m_SelectionAnchor = m_CursorIndex;
    m_IsSelecting = false;
}

void TextInput::SelectAll() {
    m_SelectionAnchor = 0;
    m_CursorIndex = static_cast<int>(m_Text.length());
    m_IsSelecting = false;
    ResetCursorBlink();
}

bool TextInput::HasSelection() const {
    return m_CursorIndex != m_SelectionAnchor;
}

int TextInput::SelectionStart() const {
    return std::min(m_CursorIndex, m_SelectionAnchor);
}

int TextInput::SelectionEnd() const {
    return std::max(m_CursorIndex, m_SelectionAnchor);
}

void TextInput::DeleteSelection() {
    if (!HasSelection()) {
        return;
    }

    const int start = SelectionStart();
    const int end = SelectionEnd();

    m_Text.erase(start, end - start);
    m_CursorIndex = start;
    ClearSelection();
    NotifyTextChanged();
}

int TextInput::GetCursorFromPosition(const Vector2& position) const {
    Ref<Font> font = FontManager::GetDefaultFont();
    if (!font || !font->IsLoaded()) {
        // Fallback: equal spacing approximation
        const float charWidth = 8.0f;
        const float padding = 8.0f;
        const float relativeX = std::max(0.0f, position.x - (m_Position.x + padding));
        const int index = static_cast<int>(relativeX / charWidth + 0.5f);
        return std::clamp(index, 0, static_cast<int>(m_Text.length()));
    }

    const float textScale = 0.5f;
    const float padding = 8.0f;
    const float relativeX = position.x - (m_Position.x + padding);

    if (relativeX <= 0.0f) {
        return 0;
    }

    const std::string displayText = GetDisplayText();
    float accumulatedWidth = 0.0f;

    for (size_t i = 0; i < displayText.length(); ++i) {
        const std::string singleChar(1, displayText[i]);
        const float charWidth = Renderer::MeasureText(singleChar, font, textScale).x;
        if (relativeX < accumulatedWidth + charWidth * 0.5f) {
            return static_cast<int>(i);
        }
        accumulatedWidth += charWidth;
    }

    return static_cast<int>(displayText.length());
}

void TextInput::InsertCharacter(char ch) {
    if (m_MaxLength >= 0 && static_cast<int>(m_Text.length()) >= m_MaxLength) {
        return;
    }

    if (m_ValidationCallback && !m_ValidationCallback(ch)) {
        return;
    }

    if (HasSelection()) {
        DeleteSelection();
    }

    m_Text.insert(static_cast<size_t>(m_CursorIndex), 1, ch);
    ++m_CursorIndex;
    ClearSelection();
    NotifyTextChanged();
    ResetCursorBlink();
}

void TextInput::DeleteCharacterBeforeCursor() {
    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    if (m_CursorIndex > 0 && !m_Text.empty()) {
        m_Text.erase(static_cast<size_t>(m_CursorIndex - 1), 1);
        --m_CursorIndex;
        ClearSelection();
        NotifyTextChanged();
        ResetCursorBlink();
    }
}

void TextInput::DeleteCharacterAtCursor() {
    if (HasSelection()) {
        DeleteSelection();
        return;
    }

    if (m_CursorIndex < static_cast<int>(m_Text.length())) {
        m_Text.erase(static_cast<size_t>(m_CursorIndex), 1);
        ClearSelection();
        NotifyTextChanged();
        ResetCursorBlink();
    }
}

std::string TextInput::GetDisplayText() const {
    if (m_IsPasswordMode && !m_Text.empty()) {
        return std::string(m_Text.length(), '*');
    }
    return m_Text;
}

void TextInput::NotifyTextChanged() {
    if (OnTextChanged) {
        OnTextChanged(m_Text);
    }
}

bool TextInput::TryHandleShortcut(int keyCode, int mods) {
    const bool ctrlDown = (mods & MOD_CONTROL) != 0;
    if (!ctrlDown) {
        return false;
    }

    switch (keyCode) {
        case KEY_A:
            SelectAll();
            return true;
        case KEY_C:
        case KEY_X:
            // Clipboard integration pending - log action for now
            SAGE_INFO("TextInput clipboard operation requested (Ctrl+{})", keyCode == KEY_C ? 'C' : 'X');
            if (keyCode == KEY_X) {
                DeleteSelection();
            }
            return true;
        case KEY_V:
            SAGE_INFO("TextInput paste requested (Ctrl+V) - clipboard integration pending");
            return true;
        default:
            return false;
    }
}

bool TextInput::TryHandleNavigationKey(int keyCode, int mods) {
    const bool shiftDown = (mods & MOD_SHIFT) != 0;

    switch (keyCode) {
        case KEY_LEFT:
            MoveCursor(-1, shiftDown);
            return true;
        case KEY_RIGHT:
            MoveCursor(1, shiftDown);
            return true;
        case KEY_HOME:
            SetCursorPosition(0, shiftDown);
            return true;
        case KEY_END:
            SetCursorPosition(static_cast<int>(m_Text.length()), shiftDown);
            return true;
        case KEY_UP:
        case KEY_DOWN:
            // Reserved for multiline support
            return true;
        default:
            return false;
    }
}

bool TextInput::TryHandleEditingKey(int keyCode, int mods) {
    (void)mods; // unused for now

    switch (keyCode) {
        case KEY_BACKSPACE:
            DeleteCharacterBeforeCursor();
            return true;
        case KEY_DELETE:
            DeleteCharacterAtCursor();
            return true;
        case KEY_ENTER:
        case KEY_KP_ENTER:
            if (OnSubmit) {
                OnSubmit();
            }
            return true;
        case KEY_TAB:
            // Allow focus manager to handle tab
            return false;
        default:
            return false;
    }
}

bool TextInput::TryHandlePrintableKey(int keyCode, int mods) {
    if (keyCode < 32 || keyCode > 126) {
        return false;
    }

    char ch = static_cast<char>(keyCode);
    const bool shiftDown = (mods & MOD_SHIFT) != 0;
    ch = ApplyShift(ch, shiftDown);

    InsertCharacter(ch);
    return true;
}

} // namespace UI
} // namespace SAGE
