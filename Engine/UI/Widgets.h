#pragma once

#include "Widget.h"
#include "Graphics/Core/Types/Color.h"
#include <string>

namespace SAGE {
namespace UI {

/**
 * @brief Slider widget - horizontal/vertical slider with value display
 */
class Slider : public Widget {
public:
    Slider(float x, float y, float width, float min, float max, float initial = 0.0f);
    
    void Render() override;
    void Update(float deltaTime) override;
    
    void OnMousePressed(MousePressedEvent& event) override;
    void OnMouseReleased(MouseReleasedEvent& event) override;
    void OnMouseMoved(MouseMovedEvent& event) override;
    
    float GetValue() const { return m_Value; }
    void SetValue(float value);
    
    void SetRange(float min, float max) { m_MinValue = min; m_MaxValue = max; }
    void SetLabel(const std::string& label) { m_Label = label; }
    
    std::function<void(float)> OnValueChanged;
    
private:
    float m_MinValue = 0.0f;
    float m_MaxValue = 1.0f;
    float m_Value = 0.0f;
    bool m_Dragging = false;
    std::string m_Label;
    
    Color m_TrackColor = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color m_HandleColor = Color(0.7f, 0.7f, 0.7f, 1.0f);
    Color m_HandleHoverColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
};

/**
 * @brief Checkbox widget - toggleable boolean value
 */
class Checkbox : public Widget {
public:
    Checkbox(float x, float y, float size = 20.0f, bool checked = false);
    
    void Render() override;
    void OnMousePressed(MousePressedEvent& event) override;
    
    bool IsChecked() const { return m_Checked; }
    void SetChecked(bool checked);
    
    void SetLabel(const std::string& label) { m_Label = label; }
    
    std::function<void(bool)> OnToggled;
    
private:
    bool m_Checked = false;
    std::string m_Label;
    
    Color m_BoxColor = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color m_CheckColor = Color(0.2f, 0.8f, 0.2f, 1.0f);
};

/**
 * @brief Dropdown menu widget - select from list of options
 */
class Dropdown : public Widget {
public:
    Dropdown(float x, float y, float width, float height = 30.0f);
    
    void Render() override;
    void Update(float deltaTime) override;
    
    void OnMousePressed(MousePressedEvent& event) override;
    
    void AddOption(const std::string& option);
    void ClearOptions();
    
    int GetSelectedIndex() const { return m_SelectedIndex; }
    std::string GetSelectedOption() const;
    
    void SetSelectedIndex(int index);
    
    std::function<void(int, const std::string&)> OnSelectionChanged;
    
private:
    std::vector<std::string> m_Options;
    int m_SelectedIndex = -1;
    bool m_IsOpen = false;
    
    Color m_ButtonColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
    Color m_ListColor = Color(0.35f, 0.35f, 0.35f, 1.0f);
    Color m_HighlightColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
};

} // namespace UI
} // namespace SAGE
