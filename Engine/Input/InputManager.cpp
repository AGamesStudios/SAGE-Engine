#include "InputManager.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace SAGE {

InputManager& InputManager::Get() {
    static InputManager instance;
    return instance;
}

void InputManager::Initialize(IWindow* window) {
    std::unique_lock lock(m_Mutex);
    
    if (m_Initialized.load()) {
        void* newHandle = window ? window->GetNativeHandle() : nullptr;
        if (m_WindowHandle == newHandle) {
            SAGE_WARN("InputManager already initialized with this window");
            return;
        }
        // Different window - re-initialize
        SAGE_WARN("InputManager re-initialized with different window");
    }
    
    if (!window) {
        SAGE_ERROR("InputManager::Initialize - null window");
        return;
    }
    
    m_Window = window;
    m_WindowHandle = window->GetNativeHandle();
    
    // Get GLFW window for legacy bridge compatibility
    GLFWwindow* glfwWindow = nullptr;
    if (window->GetWindowType() == IWindow::Type::GLFW) {
        glfwWindow = static_cast<GLFWwindow*>(m_WindowHandle);
    } else {
        SAGE_ERROR("InputManager currently only supports GLFW windows");
        return;
    }
    
    // Setup InputBridge
    m_InputBridge.SetInputMap(&m_InputMap);
    m_InputBridge.SetContextManager(&m_ContextManager);
    m_InputBridge.InstallCallbacks(glfwWindow);
    
    // Initialize gamepad states
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (glfwJoystickPresent(jid)) {
            m_Gamepads[jid].connected = true;
            if (glfwJoystickIsGamepad(jid)) {
                m_Gamepads[jid].name = glfwGetGamepadName(jid);
                SAGE_INFO("Gamepad {} connected: {}", jid, m_Gamepads[jid].name);
            }
        }
    }
    
    m_Initialized.store(true);
    SAGE_INFO("InputManager initialized");
}

void InputManager::Initialize(GLFWwindow* window) {
    // Legacy wrapper - create temporary GLFW adapter
    if (!window) {
        SAGE_ERROR("InputManager::Initialize - null GLFW window");
        return;
    }
    
    // Create temporary adapter (note: this is inefficient, users should migrate to IWindow*)
    static GLFWWindowAdapter adapter(window);
    Initialize(&adapter);
}

void InputManager::Shutdown() {
    std::unique_lock lock(m_Mutex);
    if (!m_Initialized.load()) return;
    
    m_Window = nullptr;
    m_WindowHandle = nullptr;
    m_Initialized.store(false);
    
    SAGE_INFO("InputManager shutdown");
}

void InputManager::Update() {
    if (!m_Initialized.load()) return;
    
    std::shared_lock lock(m_Mutex);
    
    // Update action states (Pressed->Held, Released->None)
    m_InputBridge.UpdateActions();
    
    // Update keyboard states for direct queries
    for (int i = 0; i < 512; ++i) {
        if (m_KeyStates[i].pressedThisFrame) {
            m_KeyStates[i].pressedThisFrame = false;
            m_KeyStates[i].held = true;
        } else if (m_KeyStates[i].releasedThisFrame) {
            m_KeyStates[i].releasedThisFrame = false;
            m_KeyStates[i].held = false;
        }
    }
    
    // Update mouse button states
    for (int i = 0; i < 8; ++i) {
        if (m_MouseButtonStates[i].pressedThisFrame) {
            m_MouseButtonStates[i].pressedThisFrame = false;
            m_MouseButtonStates[i].held = true;
        } else if (m_MouseButtonStates[i].releasedThisFrame) {
            m_MouseButtonStates[i].releasedThisFrame = false;
            m_MouseButtonStates[i].held = false;
        }
    }
    
    // FIXED BUG #7: Reset mouse delta after frame
    m_InputBridge.ResetMouseDelta();
    
    // Poll gamepads
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        auto& gp = m_Gamepads[jid];
        
        // Check connection status
        bool nowConnected = glfwJoystickPresent(jid);
        if (nowConnected != gp.connected) {
            gp.connected = nowConnected;
            if (nowConnected && glfwJoystickIsGamepad(jid)) {
                gp.name = glfwGetGamepadName(jid);
                SAGE_INFO("Gamepad {} connected: {}", jid, gp.name);
            } else {
                SAGE_INFO("Gamepad {} disconnected", jid);
                gp.name.clear();
            }
        }
        
        if (!nowConnected || !glfwJoystickIsGamepad(jid)) continue;
        
        // Poll gamepad state
        GLFWgamepadstate state;
        if (glfwGetGamepadState(jid, &state)) {
            // Update button states
            for (int btn = 0; btn < 15; ++btn) {
                bool currentlyPressed = (state.buttons[btn] == GLFW_PRESS);
                bool wasHeld = gp.buttonsHeld[btn];
                
                if (currentlyPressed && !wasHeld) {
                    // Just pressed
                    gp.buttonsPressedThisFrame[btn] = true;
                    gp.buttonsHeld[btn] = true;
                    gp.buttonsReleasedThisFrame[btn] = false;
                } else if (!currentlyPressed && wasHeld) {
                    // Just released
                    gp.buttonsPressedThisFrame[btn] = false;
                    gp.buttonsHeld[btn] = false;
                    gp.buttonsReleasedThisFrame[btn] = true;
                } else {
                    // Clear edge triggers
                    gp.buttonsPressedThisFrame[btn] = false;
                    gp.buttonsReleasedThisFrame[btn] = false;
                }
            }
            
            // Update axis values with DEAD ZONE (BUG #6 FIX)
            constexpr float DEAD_ZONE = 0.15f;
            for (int axis = 0; axis < 6; ++axis) {
                float value = state.axes[axis];
                if (std::abs(value) < DEAD_ZONE) {
                    value = 0.0f;
                }
                gp.axes[axis] = value;
            }
        }
    }
}

// ============================================================================
// KEYBOARD
// ============================================================================

bool InputManager::IsKeyPressed(Key key) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int keyCode = static_cast<int>(key);
    if (keyCode < 0 || keyCode >= 512) return false;
    
    return m_KeyStates[keyCode].pressedThisFrame;
}

bool InputManager::IsKeyHeld(Key key) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int keyCode = static_cast<int>(key);
    if (keyCode < 0 || keyCode >= 512) return false;
    
    return m_KeyStates[keyCode].held;
}

bool InputManager::IsKeyReleased(Key key) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int keyCode = static_cast<int>(key);
    if (keyCode < 0 || keyCode >= 512) return false;
    return m_KeyStates[keyCode].releasedThisFrame;
}

// ============================================================================
// MOUSE
// ============================================================================

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 8) return false;
    return m_MouseButtonStates[btn].pressedThisFrame;
}

bool InputManager::IsMouseButtonHeld(MouseButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 8) return false;
    
    return m_MouseButtonStates[btn].held;
}

bool InputManager::IsMouseButtonReleased(MouseButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 8) return false;
    return m_MouseButtonStates[btn].releasedThisFrame;
}

Vector2 InputManager::GetMousePosition() const {
    if (!m_Initialized.load()) return Vector2::Zero();
    std::shared_lock lock(m_Mutex);
    return m_InputBridge.GetMousePosition();
}

Vector2 InputManager::GetMouseDelta() const {
    if (!m_Initialized.load()) return Vector2::Zero();
    std::shared_lock lock(m_Mutex);
    return m_InputBridge.GetMouseDelta();
}

float InputManager::GetScrollDelta() const {
    if (!m_Initialized.load()) return 0.0f;
    std::shared_lock lock(m_Mutex);
    return m_InputBridge.GetScrollDelta();
}

void InputManager::ConsumeScroll() {
    if (!m_Initialized.load()) return;
    std::unique_lock lock(m_Mutex);
    m_InputBridge.ConsumeScroll();
}

// ============================================================================
// GAMEPAD
// ============================================================================

bool InputManager::IsGamepadConnected(int gamepadId) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return false;
    return m_Gamepads[gamepadId].connected;
}

const char* InputManager::GetGamepadName(int gamepadId) const {
    if (!m_Initialized.load()) return "";
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return "";
    return m_Gamepads[gamepadId].name.c_str();
}

bool InputManager::IsGamepadButtonPressed(int gamepadId, GamepadButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return false;
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 15) return false;
    return m_Gamepads[gamepadId].buttonsPressedThisFrame[btn];
}

bool InputManager::IsGamepadButtonHeld(int gamepadId, GamepadButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return false;
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 15) return false;
    return m_Gamepads[gamepadId].buttonsHeld[btn];
}

bool InputManager::IsGamepadButtonReleased(int gamepadId, GamepadButton button) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return false;
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= 15) return false;
    return m_Gamepads[gamepadId].buttonsReleasedThisFrame[btn];
}

float InputManager::GetGamepadAxis(int gamepadId, GamepadAxis axis) const {
    if (!m_Initialized.load()) return 0.0f;
    std::shared_lock lock(m_Mutex);
    if (gamepadId < 0 || gamepadId > GLFW_JOYSTICK_LAST) return 0.0f;
    int axisIndex = static_cast<int>(axis);
    if (axisIndex < 0 || axisIndex >= 6) return 0.0f;
    return m_Gamepads[gamepadId].axes[axisIndex];
}

// ============================================================================
// ACTIONS
// ============================================================================

bool InputManager::IsActionPressed(const std::string& actionName) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    // Check context manager first (priority)
    if (m_ContextManager.GetActiveContext()) {
        return m_ContextManager.IsActionPressed(actionName);
    }
    
    // Fallback to default input map
    return m_InputMap.IsActionPressed(actionName);
}

bool InputManager::IsActionHeld(const std::string& actionName) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    if (m_ContextManager.GetActiveContext()) {
        return m_ContextManager.IsActionHeld(actionName);
    }
    return m_InputMap.IsActionHeld(actionName);
}

bool InputManager::IsActionReleased(const std::string& actionName) const {
    if (!m_Initialized.load()) return false;
    std::shared_lock lock(m_Mutex);
    
    if (m_ContextManager.GetActiveContext()) {
        return m_ContextManager.IsActionReleased(actionName);
    }
    return m_InputMap.IsActionReleased(actionName);
}

float InputManager::GetActionValue(const std::string& actionName) const {
    if (!m_Initialized.load()) return 0.0f;
    std::shared_lock lock(m_Mutex);
    
    if (m_ContextManager.GetActiveContext()) {
        return m_ContextManager.GetActionValue(actionName);
    }
    return m_InputMap.GetActionValue(actionName);
}

// ============================================================================
// INTERNAL: Update keyboard state from callbacks
// ============================================================================

namespace Internal {
    void UpdateKeyState(int key, bool pressed) {
        auto& mgr = InputManager::Get();
        auto* states = mgr.GetKeyStates();
        
        if (key < 0 || key >= 512) return;
        
        if (pressed) {
            if (!states[key].held) {
                states[key].pressedThisFrame = true;
                states[key].held = true;
                states[key].releasedThisFrame = false;
            }
        } else {
            if (states[key].held) {
                states[key].pressedThisFrame = false;
                states[key].held = false;
                states[key].releasedThisFrame = true;
            }
        }
    }
    
    void UpdateMouseButtonState(int button, bool pressed) {
        auto& mgr = InputManager::Get();
        auto* states = mgr.GetMouseButtonStates();
        
        if (button < 0 || button >= 8) return;
        
        if (pressed) {
            if (!states[button].held) {
                states[button].pressedThisFrame = true;
                states[button].held = true;
                states[button].releasedThisFrame = false;
            }
        } else {
            if (states[button].held) {
                states[button].pressedThisFrame = false;
                states[button].held = false;
                states[button].releasedThisFrame = true;
            }
        }
    }
}

} // namespace SAGE
