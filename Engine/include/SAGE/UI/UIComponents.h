#pragma once

#include "SAGE/UI/Widget.h"

namespace SAGE {

class Image : public Widget {
public:
    Image();
    virtual ~Image() = default;
    
    // Image specific methods can be added here if needed
    // Currently Widget already supports Texture, so Image is mostly a semantic wrapper
    // But we can add things like "PreserveAspect", "Tiling", etc.
    
    void SetPreserveAspect(bool preserve) { m_PreserveAspect = preserve; }
    bool GetPreserveAspect() const { return m_PreserveAspect; }

    void Draw(RenderBackend* renderer) override;

private:
    bool m_PreserveAspect = false;
};

class ProgressBar : public Widget {
public:
    ProgressBar();
    virtual ~ProgressBar() = default;

    void SetValue(float value); // 0.0 to 1.0
    float GetValue() const { return m_Value; }

    void SetFillColor(const Color& color) { m_FillColor = color; }
    const Color& GetFillColor() const { return m_FillColor; }

    void Draw(RenderBackend* renderer) override;

private:
    float m_Value = 0.5f;
    Color m_FillColor = Color::Green();
};

class InputField : public Widget {
public:
    InputField();
    virtual ~InputField() = default;

    void SetPlaceholder(const std::string& text) { m_Placeholder = text; }
    const std::string& GetPlaceholder() const { return m_Placeholder; }

    void SetMaxLength(int length) { m_MaxLength = length; }
    int GetMaxLength() const { return m_MaxLength; }

    void SetPasswordMode(bool password) { m_IsPassword = password; }
    bool IsPasswordMode() const { return m_IsPassword; }

    // Events
    bool OnMouseDown(int button) override;
    bool OnKeyDown(int key); // Needs to be called by UIContext
    bool OnCharInput(unsigned int codepoint) override; // Needs to be called by UIContext
    
    void OnFocus() override { m_IsFocused = true; m_CursorVisible = true; m_CursorBlinkTimer = 0.0f; }
    void OnLostFocus() override { m_IsFocused = false; m_CursorVisible = false; }

    void Update(float dt) override;
    void Draw(RenderBackend* renderer) override;

    std::function<void(const std::string&)> OnSubmit;
    std::function<void(const std::string&)> OnValueChanged;

private:
    std::string m_Placeholder;
    int m_MaxLength = 256;
    bool m_IsPassword = false;
    bool m_IsFocused = false;
    
    float m_CursorBlinkTimer = 0.0f;
    bool m_CursorVisible = false;
    float m_ScrollOffset = 0.0f;
    int m_CursorIndex = 0; // Byte offset into m_Text
};

} // namespace SAGE
