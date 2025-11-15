#include "InputBridge.h"
#include "InputAction.h"
#include "Core/Logger.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace SAGE {

namespace {
    // Convert GLFW key codes to SAGE::Key
    Key GLFWKeyToSAGEKey(int glfwKey) {
        // Direct mapping for alphanumeric keys
        if (glfwKey >= GLFW_KEY_A && glfwKey <= GLFW_KEY_Z) {
            return static_cast<Key>(static_cast<int>(Key::A) + (glfwKey - GLFW_KEY_A));
        }
        if (glfwKey >= GLFW_KEY_0 && glfwKey <= GLFW_KEY_9) {
            return static_cast<Key>(static_cast<int>(Key::D0) + (glfwKey - GLFW_KEY_0));
        }
        if (glfwKey >= GLFW_KEY_F1 && glfwKey <= GLFW_KEY_F12) {
            return static_cast<Key>(static_cast<int>(Key::F1) + (glfwKey - GLFW_KEY_F1));
        }

        // Special keys
        switch (glfwKey) {
            case GLFW_KEY_SPACE: return Key::Space;
            case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
            case GLFW_KEY_COMMA: return Key::Comma;
            case GLFW_KEY_MINUS: return Key::Minus;
            case GLFW_KEY_PERIOD: return Key::Period;
            case GLFW_KEY_SLASH: return Key::Slash;
            case GLFW_KEY_SEMICOLON: return Key::Semicolon;
            case GLFW_KEY_EQUAL: return Key::Equal;
            case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
            case GLFW_KEY_BACKSLASH: return Key::Backslash;
            case GLFW_KEY_RIGHT_BRACKET: return Key::RightBracket;
            case GLFW_KEY_GRAVE_ACCENT: return Key::GraveAccent;
            case GLFW_KEY_ESCAPE: return Key::Escape;
            case GLFW_KEY_ENTER: return Key::Enter;
            case GLFW_KEY_TAB: return Key::Tab;
            case GLFW_KEY_BACKSPACE: return Key::Backspace;
            case GLFW_KEY_INSERT: return Key::Insert;
            case GLFW_KEY_DELETE: return Key::Delete;
            case GLFW_KEY_RIGHT: return Key::Right;
            case GLFW_KEY_LEFT: return Key::Left;
            case GLFW_KEY_DOWN: return Key::Down;
            case GLFW_KEY_UP: return Key::Up;
            case GLFW_KEY_PAGE_UP: return Key::PageUp;
            case GLFW_KEY_PAGE_DOWN: return Key::PageDown;
            case GLFW_KEY_HOME: return Key::Home;
            case GLFW_KEY_END: return Key::End;
            case GLFW_KEY_CAPS_LOCK: return Key::CapsLock;
            case GLFW_KEY_SCROLL_LOCK: return Key::ScrollLock;
            case GLFW_KEY_NUM_LOCK: return Key::NumLock;
            case GLFW_KEY_PRINT_SCREEN: return Key::PrintScreen;
            case GLFW_KEY_PAUSE: return Key::Pause;
            case GLFW_KEY_LEFT_SHIFT: return Key::LeftShift;
            case GLFW_KEY_LEFT_CONTROL: return Key::LeftControl;
            case GLFW_KEY_LEFT_ALT: return Key::LeftAlt;
            case GLFW_KEY_LEFT_SUPER: return Key::LeftSuper;
            case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
            case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
            case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;
            case GLFW_KEY_RIGHT_SUPER: return Key::RightSuper;
            case GLFW_KEY_MENU: return Key::Menu;
            default: return Key::Unknown;
        }
    }

    // Convert GLFW mouse button to SAGE::MouseButton
    MouseButton GLFWMouseButtonToSAGE(int glfwButton) {
        switch (glfwButton) {
            case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
            case GLFW_MOUSE_BUTTON_4: return MouseButton::Button4;
            case GLFW_MOUSE_BUTTON_5: return MouseButton::Button5;
            case GLFW_MOUSE_BUTTON_6: return MouseButton::Button6;
            case GLFW_MOUSE_BUTTON_7: return MouseButton::Button7;
            case GLFW_MOUSE_BUTTON_8: return MouseButton::Button8;
            default: return MouseButton::Left;
        }
    }
}

// Static callbacks
static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* windowData = static_cast<SAGE::Window::WindowData*>(glfwGetWindowUserPointer(window));
    if (!windowData || !windowData->InputBridge) return;
    
    auto* bridge = static_cast<SAGE::InputBridge*>(windowData->InputBridge);
    bridge->OnKeyEvent(key, scancode, action, mods);
}

static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* windowData = static_cast<SAGE::Window::WindowData*>(glfwGetWindowUserPointer(window));
    if (!windowData || !windowData->InputBridge) return;
    
    auto* bridge = static_cast<SAGE::InputBridge*>(windowData->InputBridge);
    bridge->OnMouseButton(button, action, mods);
}

static void GLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* windowData = static_cast<SAGE::Window::WindowData*>(glfwGetWindowUserPointer(window));
    if (!windowData || !windowData->InputBridge) return;
    
    auto* bridge = static_cast<SAGE::InputBridge*>(windowData->InputBridge);
    bridge->OnCursorPos(xpos, ypos);
}

static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* windowData = static_cast<SAGE::Window::WindowData*>(glfwGetWindowUserPointer(window));
    if (!windowData || !windowData->InputBridge) return;
    
    auto* bridge = static_cast<SAGE::InputBridge*>(windowData->InputBridge);
    bridge->OnScroll(xoffset, yoffset);
}

void InputBridge::InstallCallbacks(GLFWwindow* window) {
    if (!window) {
        SAGE_ERROR("InputBridge::InstallCallbacks - null window");
        return;
    }

    // Get WindowData and store this pointer
    auto* windowData = static_cast<Window::WindowData*>(glfwGetWindowUserPointer(window));
    if (!windowData) {
        SAGE_ERROR("InputBridge::InstallCallbacks - null WindowData!");
        return;
    }
    
    windowData->InputBridge = this;

    // Install GLFW callbacks
    glfwSetKeyCallback(window, GLFWKeyCallback);
    glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
    glfwSetCursorPosCallback(window, GLFWCursorPosCallback);
    glfwSetScrollCallback(window, GLFWScrollCallback);

    SAGE_INFO("InputBridge callbacks installed");
}

void InputBridge::UpdateActions() {
    auto* inputMap = GetActiveInputMap();
    if (!inputMap) return;

    for (auto& [name, action] : inputMap->GetActions()) {
        auto state = action->GetState();
        
        // Transition states
        if (state == ActionState::Pressed) {
            action->SetState(ActionState::Held);
        } else if (state == ActionState::Released) {
            action->SetState(ActionState::None);
            action->SetValue(0.0f);
        }
    }

    // NOTE: Scroll deltas are NOT reset here anymore
    // Call ConsumeScroll() explicitly after reading if needed
}

void InputBridge::OnKeyEvent(int key, int scancode, int action, int mods) {
    // Unused GLFW parameters in current implementation
    (void)scancode; (void)mods;
    auto* inputMap = GetActiveInputMap();
    if (!inputMap) return;

    Key sageKey = GLFWKeyToSAGEKey(key);
    if (sageKey == Key::Unknown) return;

    // FIXED: Only GLFW_PRESS triggers Pressed state, not GLFW_REPEAT
    bool pressed = (action == GLFW_PRESS);
    bool released = (action == GLFW_RELEASE);

    // CRITICAL FIX: Update InputManager's direct key state tracking
    Internal::UpdateKeyState(key, pressed);

    // Update all actions bound to this key
    for (auto& [name, actionPtr] : inputMap->GetActions()) {
        for (const auto& binding : actionPtr->GetBindings()) {
            if (binding.type == InputSource::Type::Keyboard &&
                std::get<Key>(binding.source) == sageKey) {
                if (pressed) {
                    UpdateActionForSource(actionPtr.get(), binding, true);
                } else if (released) {
                    UpdateActionForSource(actionPtr.get(), binding, false);
                }
            }
        }
    }
}

void InputBridge::OnMouseButton(int button, int action, int mods) {
    (void)mods; // currently unused
    auto* inputMap = GetActiveInputMap();
    if (!inputMap) return;

    MouseButton sageButton = GLFWMouseButtonToSAGE(button);
    bool pressed = (action == GLFW_PRESS);

    // CRITICAL FIX: Update InputManager's direct mouse button state tracking
    Internal::UpdateMouseButtonState(button, pressed);

    SAGE_INFO("Mouse button {} {}", button, pressed ? "pressed" : "released");

    // Update all actions bound to this mouse button
    for (auto& [name, actionPtr] : inputMap->GetActions()) {
        for (const auto& binding : actionPtr->GetBindings()) {
            if (binding.type == InputSource::Type::Mouse &&
                std::get<MouseButton>(binding.source) == sageButton) {
                UpdateActionForSource(actionPtr.get(), binding, pressed);
            }
        }
    }
}

void InputBridge::OnCursorPos(double xpos, double ypos) {
    m_MouseDeltaX = xpos - m_LastMouseX;
    m_MouseDeltaY = ypos - m_LastMouseY;
    m_LastMouseX = xpos;
    m_LastMouseY = ypos;

    // Update mouse movement actions (if mapped)
    InputMap* map = GetActiveInputMap();
    if (!map) return;
    
    // Mouse delta can be used for camera rotation, look controls, etc.
    // Actions can check GetMouseDelta() via InputSystem
}

void InputBridge::OnScroll(double xoffset, double yoffset) {
    m_ScrollX = xoffset;
    m_ScrollY = yoffset;

    // Update scroll actions (if mapped)
    InputMap* map = GetActiveInputMap();
    if (!map) return;
    
    // Scroll can be used for zoom, weapon switching, etc.
    // Actions can check GetScrollDelta() via InputSystem
    // Note: Scroll is typically consumed per-frame, not held like buttons
}

void InputBridge::UpdateActionForSource(InputAction* action, [[maybe_unused]] const InputSource& source, bool pressed) {
    if (!action) return;
    // (void)source; // suppressed via attribute

    if (pressed) {
        if (action->GetState() == ActionState::None || 
            action->GetState() == ActionState::Released) {
            action->SetState(ActionState::Pressed);
            action->SetValue(1.0f);
        }
    } else {
        if (action->GetState() == ActionState::Pressed || 
            action->GetState() == ActionState::Held) {
            action->SetState(ActionState::Released);
            action->SetValue(0.0f);
        }
    }
}

InputMap* InputBridge::GetActiveInputMap() {
    // Priority: context manager's active context → fallback input map
    if (m_ContextManager) {
        auto* activeContext = m_ContextManager->GetActiveContext();
        if (activeContext) {
            return &activeContext->GetInputMap();
        }
    }

    return m_InputMap;
}

} // namespace SAGE
