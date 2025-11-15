#pragma once

#include "KeyCodes.h"
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

/**
 * @file InputAction.h
 * @brief Action-based input system for rebindable controls
 */

namespace SAGE {

/**
 * @brief Input source (key, mouse button, or gamepad button)
 */
struct InputSource {
    enum class Type {
        Keyboard,
        Mouse,
        GamepadButton,
        GamepadAxis
    };
    
    Type type;
    std::variant<Key, MouseButton, GamepadButton, GamepadAxis> source;
    
    // For axes: value threshold to trigger action (e.g., 0.5 for analog stick)
    float axisThreshold = 0.5f;
    
    // Constructors
    InputSource(Key key) : type(Type::Keyboard), source(key) {}
    InputSource(MouseButton button) : type(Type::Mouse), source(button) {}
    InputSource(GamepadButton button) : type(Type::GamepadButton), source(button) {}
    InputSource(GamepadAxis axis, float threshold = 0.5f) 
        : type(Type::GamepadAxis), source(axis), axisThreshold(threshold) {}
    
    bool operator==(const InputSource& other) const {
        return type == other.type && source == other.source;
    }
};

/**
 * @brief Input action state
 */
enum class ActionState {
    None,       // Not pressed
    Pressed,    // Just pressed this frame
    Held,       // Held down
    Released    // Just released this frame
};

/**
 * @brief Named input action with multiple bindings
 * 
 * Example: "Jump" action can be bound to Space, Gamepad A button, etc.
 */
class InputAction {
public:
    InputAction() = default;
    explicit InputAction(const std::string& name) : m_Name(name) {}
    
    // Name
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    // Bindings
    void AddBinding(const InputSource& source) { 
        // FIXED: Check for duplicates before adding
        if (std::find(m_Bindings.begin(), m_Bindings.end(), source) == m_Bindings.end()) {
            m_Bindings.push_back(source); 
        }
    }
    void RemoveBinding(const InputSource& source);
    void ClearBindings() { m_Bindings.clear(); }
    const std::vector<InputSource>& GetBindings() const { return m_Bindings; }
    
    // State
    ActionState GetState() const { return m_State; }
    void SetState(ActionState state) { m_State = state; }
    
    // Convenience checks
    bool IsPressed() const { return m_State == ActionState::Pressed; }
    bool IsHeld() const { return m_State == ActionState::Held || m_State == ActionState::Pressed; }
    bool IsReleased() const { return m_State == ActionState::Released; }
    
    // Analog value (for axes, 0-1 range)
    float GetValue() const { return m_Value; }
    void SetValue(float value) { m_Value = value; }
    
private:
    std::string m_Name;
    std::vector<InputSource> m_Bindings;
    ActionState m_State = ActionState::None;
    float m_Value = 0.0f;
};

} // namespace SAGE
