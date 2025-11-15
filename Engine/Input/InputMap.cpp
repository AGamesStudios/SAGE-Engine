#include "InputMap.h"

namespace SAGE {

InputAction* InputMap::GetOrCreateAction(const std::string& name) {
    auto it = m_Actions.find(name);
    if (it != m_Actions.end()) {
        return it->second.get();
    }
    
    auto action = std::make_unique<InputAction>(name);
    auto* ptr = action.get();
    m_Actions[name] = std::move(action);
    return ptr;
}

InputAction* InputMap::GetAction(const std::string& name) {
    auto it = m_Actions.find(name);
    return (it != m_Actions.end()) ? it->second.get() : nullptr;
}

const InputAction* InputMap::GetAction(const std::string& name) const {
    auto it = m_Actions.find(name);
    return (it != m_Actions.end()) ? it->second.get() : nullptr;
}

bool InputMap::HasAction(const std::string& name) const {
    return m_Actions.find(name) != m_Actions.end();
}

void InputMap::RemoveAction(const std::string& name) {
    m_Actions.erase(name);
}

bool InputMap::IsActionPressed(const std::string& name) const {
    const auto* action = GetAction(name);
    return action ? action->IsPressed() : false;
}

bool InputMap::IsActionHeld(const std::string& name) const {
    const auto* action = GetAction(name);
    return action ? action->IsHeld() : false;
}

bool InputMap::IsActionReleased(const std::string& name) const {
    const auto* action = GetAction(name);
    return action ? action->IsReleased() : false;
}

float InputMap::GetActionValue(const std::string& name) const {
    const auto* action = GetAction(name);
    return action ? action->GetValue() : 0.0f;
}

} // namespace SAGE
