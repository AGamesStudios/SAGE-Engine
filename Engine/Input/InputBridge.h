#pragma once

#include "InputMap.h"
#include "ActionContext.h"
#include "Math/Vector2.h"
#include <memory>

struct GLFWwindow;

namespace SAGE {

// Forward declarations
namespace Internal {
    void UpdateKeyState(int key, bool pressed);
    void UpdateMouseButtonState(int button, bool pressed);
}

/**
 * @brief Bridge between GLFW callbacks and InputMap/ActionContext
 * 
 * Translates raw GLFW events into InputAction state updates.
 * Supports keyboard, mouse, and gamepad input.
 */
class InputBridge {
public:
    InputBridge() = default;
    ~InputBridge() = default;

    /**
     * @brief Set the input map to route events to
     */
    void SetInputMap(InputMap* inputMap) { m_InputMap = inputMap; }
    InputMap* GetInputMap() const { return m_InputMap; }

    /**
     * @brief Set the context manager for priority-based routing
     */
    void SetContextManager(ActionContextManager* manager) { m_ContextManager = manager; }
    ActionContextManager* GetContextManager() const { return m_ContextManager; }

    /**
     * @brief Install GLFW callbacks on a window
     * 
     * Adds input callbacks without overwriting window's user pointer.
     * Stores 'this' in WindowData::InputBridge field.
     */
    void InstallCallbacks(GLFWwindow* window);

    /**
     * @brief Update action states (call once per frame before processing input)
     * 
     * Transitions Pressed → Held, Released → None, etc.
     */
    void UpdateActions();

    /**
     * @brief GLFW callback handlers (called from static callbacks)
     */
    void OnKeyEvent(int key, int scancode, int action, int mods);
    void OnMouseButton(int button, int action, int mods);
    void OnCursorPos(double xpos, double ypos);
    void OnScroll(double xoffset, double yoffset);

    /**
     * @brief Get mouse position
     */
    Vector2 GetMousePosition() const { return Vector2(static_cast<float>(m_LastMouseX), static_cast<float>(m_LastMouseY)); }
    
    /**
     * @brief Get mouse movement delta
     */
    Vector2 GetMouseDelta() const { return Vector2(static_cast<float>(m_MouseDeltaX), static_cast<float>(m_MouseDeltaY)); }
    
    /**
     * @brief Get scroll delta (Y axis)
     */
    float GetScrollDelta() const { return static_cast<float>(m_ScrollY); }
    
    /**
     * @brief Consume scroll delta (reset to 0)
     */
    void ConsumeScroll() { m_ScrollX = 0.0; m_ScrollY = 0.0; }
    
    /**
     * @brief Reset mouse delta (called per frame after reading)
     */
    void ResetMouseDelta() { m_MouseDeltaX = 0.0; m_MouseDeltaY = 0.0; }

private:
    InputMap* m_InputMap = nullptr;
    ActionContextManager* m_ContextManager = nullptr;

    // Mouse state
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;
    double m_MouseDeltaX = 0.0;
    double m_MouseDeltaY = 0.0;
    double m_ScrollX = 0.0;
    double m_ScrollY = 0.0;

    /**
     * @brief Update action state based on input source
     */
    void UpdateActionForSource(InputAction* action, const InputSource& source, bool pressed);

    /**
     * @brief Get active input map (from context manager or fallback)
     */
    InputMap* GetActiveInputMap();
};

} // namespace SAGE
