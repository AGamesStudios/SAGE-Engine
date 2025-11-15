#pragma once

#include "Widget.h"
#include "Graphics/Core/Types/Color.h"

#include <functional>
#include <string>
#include <utility>

namespace SAGE {
namespace UI {

/// @brief Text input widget with cursor, selection, and basic editing support.
class TextInput : public Widget {
public:
    TextInput();
    explicit TextInput(const std::string& id);

    void Update(float deltaTime) override;
    void Render() override;

    void OnFocus() override;
    void OnBlur() override;

    void OnMousePressed(MousePressedEvent& event) override;
    void OnMouseReleased(MouseReleasedEvent& event) override;
    void OnMouseMoved(MouseMovedEvent& event) override;
    void OnKeyPressed(KeyPressedEvent& event) override;

    void SetText(const std::string& text);
    const std::string& GetText() const { return m_Text; }

    void SetPlaceholder(const std::string& placeholder) { m_Placeholder = placeholder; }
    const std::string& GetPlaceholder() const { return m_Placeholder; }

    void SetMaxLength(int maxLength);
    int GetMaxLength() const { return m_MaxLength; }

    void SetPasswordMode(bool enabled) { m_IsPasswordMode = enabled; }
    bool IsPasswordMode() const { return m_IsPasswordMode; }

    void SetValidationCallback(std::function<bool(char)> callback) { m_ValidationCallback = std::move(callback); }

    std::function<void(const std::string&)> OnTextChanged;
    std::function<void()> OnSubmit;

    void SetOnTextChanged(std::function<void(const std::string&)> callback) { OnTextChanged = std::move(callback); }
    void SetOnEnterPressed(std::function<void()> callback) { OnSubmit = std::move(callback); }

    void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; }
    const Color& GetBackgroundColor() const { return m_BackgroundColor; }

    void SetFocusedBackgroundColor(const Color& color) { m_FocusedBackgroundColor = color; }
    const Color& GetFocusedBackgroundColor() const { return m_FocusedBackgroundColor; }

    void SetBorderColor(const Color& color) { m_BorderColor = color; }
    const Color& GetBorderColor() const { return m_BorderColor; }

    void SetFocusedBorderColor(const Color& color) { m_FocusedBorderColor = color; }
    const Color& GetFocusedBorderColor() const { return m_FocusedBorderColor; }

    void SetTextColor(const Color& color) { m_TextColor = color; }
    const Color& GetTextColor() const { return m_TextColor; }

    void SetPlaceholderColor(const Color& color) { m_PlaceholderColor = color; }
    const Color& GetPlaceholderColor() const { return m_PlaceholderColor; }

    void SetSelectionColor(const Color& color) { m_SelectionColor = color; }
    const Color& GetSelectionColor() const { return m_SelectionColor; }

    void SetBorderWidth(float width) { m_BorderWidth = width; }
    float GetBorderWidth() const { return m_BorderWidth; }

    void SetCursorBlinkInterval(float interval) { m_CursorBlinkInterval = interval; }
    float GetCursorBlinkInterval() const { return m_CursorBlinkInterval; }

    void SelectAll();

private:
    // Rendering helpers
    void DrawBackground() const;
    void DrawSelection() const;
    void DrawText() const;
    void DrawCursor() const;

    // Cursor & selection helpers
    void ResetCursorBlink();
    void MoveCursor(int delta, bool selecting);
    void SetCursorPosition(int position, bool selecting);
    void ClearSelection();
    bool HasSelection() const;
    int SelectionStart() const;
    int SelectionEnd() const;
    void DeleteSelection();
    int GetCursorFromPosition(const Vector2& position) const;

    // Text manipulation
    void InsertCharacter(char ch);
    void DeleteCharacterBeforeCursor();
    void DeleteCharacterAtCursor();
    std::string GetDisplayText() const;
    void NotifyTextChanged();

    // Input helpers
    bool TryHandleShortcut(int keyCode, int mods);
    bool TryHandleNavigationKey(int keyCode, int mods);
    bool TryHandleEditingKey(int keyCode, int mods);
    bool TryHandlePrintableKey(int keyCode, int mods);

private:
    std::string m_Text;
    std::string m_Placeholder;
    int m_MaxLength = -1; // -1 = unlimited

    int m_CursorIndex = 0;
    int m_SelectionAnchor = 0;
    bool m_IsSelecting = false;

    bool m_IsFocused = false;
    float m_CursorBlinkTimer = 0.0f;
    bool m_ShowCursor = true;
    float m_CursorBlinkInterval = 0.5f;

    bool m_IsPasswordMode = false;

    // Styling
    Color m_BackgroundColor = Color(0.15f, 0.15f, 0.18f, 1.0f);
    Color m_FocusedBackgroundColor = Color(0.20f, 0.20f, 0.28f, 1.0f);
    Color m_BorderColor = Color(0.35f, 0.35f, 0.35f, 1.0f);
    Color m_FocusedBorderColor = Color(0.50f, 0.70f, 1.00f, 1.0f);
    Color m_TextColor = Color(0.95f, 0.96f, 0.97f, 1.0f);
    Color m_PlaceholderColor = Color(0.60f, 0.64f, 0.68f, 1.0f);
    Color m_SelectionColor = Color(0.30f, 0.55f, 0.90f, 0.35f);
    float m_BorderWidth = 1.0f;

    std::function<bool(char)> m_ValidationCallback;
};

} // namespace UI
} // namespace SAGE
