#pragma once

#include "InputAction.h"
#include <unordered_map>
#include <memory>

/**
 * @file InputMap.h
 * @brief Maps input sources to named actions
 */

namespace SAGE {

/**
 * @brief Input mapping system
 * 
 * Manages all input actions and their bindings.
 * Supports multiple bindings per action and rebinding at runtime.
 */
class InputMap {
public:
    InputMap() = default;
    ~InputMap() = default;
    
    /**
     * @brief Create or get an action by name
     */
    InputAction* GetOrCreateAction(const std::string& name);
    
    /**
     * @brief Get an action by name
     * @return nullptr if action doesn't exist
     */
    InputAction* GetAction(const std::string& name);
    const InputAction* GetAction(const std::string& name) const;
    
    /**
     * @brief Check if action exists
     */
    bool HasAction(const std::string& name) const;
    
    /**
     * @brief Remove an action
     */
    void RemoveAction(const std::string& name);
    
    /**
     * @brief Clear all actions
     */
    void Clear() { m_Actions.clear(); }
    
    /**
     * @brief Get all actions
     */
    const std::unordered_map<std::string, std::unique_ptr<InputAction>>& GetActions() const {
        return m_Actions;
    }
    
    /**
     * @brief Quick action state checks
     */
    bool IsActionPressed(const std::string& name) const;
    bool IsActionHeld(const std::string& name) const;
    bool IsActionReleased(const std::string& name) const;
    float GetActionValue(const std::string& name) const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<InputAction>> m_Actions;
};

} // namespace SAGE
