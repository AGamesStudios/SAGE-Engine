#pragma once

#include "InputMap.h"
#include <string>
#include <memory>

/**
 * @file ActionContext.h
 * @brief Context-specific input mappings (menu, gameplay, pause, etc.)
 */

namespace SAGE {

/**
 * @brief Input context for different game states
 * 
 * Allows different key bindings for different contexts.
 * Example: ESC opens menu in gameplay, but closes menu in menu context.
 */
class ActionContext {
public:
    explicit ActionContext(const std::string& name) : m_Name(name), m_Active(false) {}
    
    // Name
    const std::string& GetName() const { return m_Name; }
    
    // Active state
    bool IsActive() const { return m_Active; }
    void SetActive(bool active) { m_Active = active; }
    
    // Priority (higher priority contexts override lower ones)
    int GetPriority() const { return m_Priority; }
    void SetPriority(int priority) { m_Priority = priority; }
    
    // Input map for this context
    InputMap& GetInputMap() { return m_InputMap; }
    const InputMap& GetInputMap() const { return m_InputMap; }
    
private:
    std::string m_Name;
    bool m_Active;
    int m_Priority = 0;
    InputMap m_InputMap;
};

/**
 * @brief Manages multiple input contexts with priority system
 */
class ActionContextManager {
public:
    ActionContextManager() = default;
    
    /**
     * @brief Create a new context
     */
    ActionContext* CreateContext(const std::string& name, int priority = 0);
    
    /**
     * @brief Get a context by name
     */
    ActionContext* GetContext(const std::string& name);
    const ActionContext* GetContext(const std::string& name) const;
    
    /**
     * @brief Activate a context (can have multiple active)
     */
    void ActivateContext(const std::string& name);
    
    /**
     * @brief Deactivate a context
     */
    void DeactivateContext(const std::string& name);
    
    /**
     * @brief Deactivate all contexts
     */
    void DeactivateAll();
    
    /**
     * @brief Get highest priority active context
     * @return nullptr if no active contexts
     */
    ActionContext* GetActiveContext();
    const ActionContext* GetActiveContext() const;
    
    /**
     * @brief Check action state in active contexts (highest priority wins)
     */
    bool IsActionPressed(const std::string& actionName) const;
    bool IsActionHeld(const std::string& actionName) const;
    bool IsActionReleased(const std::string& actionName) const;
    float GetActionValue(const std::string& actionName) const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<ActionContext>> m_Contexts;
};

} // namespace SAGE
