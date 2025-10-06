#include "Input.h"
#include "InputBindings.h"
#include "../Core/Application.h"

#include <GLFW/glfw3.h>

#include <array>

namespace SAGE {

    namespace {
        std::array<bool, GLFW_KEY_LAST + 1> s_CurrentKeys{};
        std::array<bool, GLFW_KEY_LAST + 1> s_PreviousKeys{};

        std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> s_CurrentMouse{};
        std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> s_PreviousMouse{};

        Vector2 s_MousePosition{ 0.0f, 0.0f };
        Vector2 s_LastMousePosition{ 0.0f, 0.0f };
#ifdef SAGE_ENGINE_TESTING
        bool s_TestOverridesEnabled = false;
#endif
        std::array<unsigned char, GLFW_GAMEPAD_BUTTON_LAST + 1> s_CurrentGamepadButtons{};
        std::array<unsigned char, GLFW_GAMEPAD_BUTTON_LAST + 1> s_PreviousGamepadButtons{};
        std::array<float, GLFW_GAMEPAD_AXIS_LAST + 1> s_GamepadAxes{};
        bool s_GamepadActive = false;
        bool s_Initialized = false;

        GLFWwindow* GetWindowHandle() {
            return Application::Get().GetWindow().GetNativeWindow();
        }

        bool ReadGamepadState(int index, bool advanceState) {
#ifdef SAGE_ENGINE_TESTING
            if (s_TestOverridesEnabled) {
                (void)index;
                (void)advanceState;
                return s_GamepadActive;
            }
#endif
            if (index < 0 || index > GLFW_JOYSTICK_LAST) {
                return false;
            }

            if (!glfwJoystickIsGamepad(index)) {
                return false;
            }

            GLFWgamepadstate state;
            if (!glfwGetGamepadState(index, &state)) {
                return false;
            }

            if (advanceState) {
                s_PreviousGamepadButtons = s_CurrentGamepadButtons;
            }

            for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button) {
                s_CurrentGamepadButtons[button] = state.buttons[button];
            }

            for (int axis = 0; axis <= GLFW_GAMEPAD_AXIS_LAST; ++axis) {
                s_GamepadAxes[axis] = state.axes[axis];
            }

            s_GamepadActive = true;
            return true;
        }
    }

    void Input::Init() {
        if (s_Initialized)
            return;

        s_Initialized = true;
        Update();
    }

    void Input::Update() {
        if (!s_Initialized)
            Init();

#ifdef SAGE_ENGINE_TESTING
        if (s_TestOverridesEnabled) {
            InputBindings::Update();
            return;
        }
#endif

        GLFWwindow* window = GetWindowHandle();

        s_PreviousKeys = s_CurrentKeys;
        s_CurrentKeys.fill(false);
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
            int state = glfwGetKey(window, key);
            s_CurrentKeys[key] = (state == GLFW_PRESS || state == GLFW_REPEAT);
        }

        s_PreviousMouse = s_CurrentMouse;
        s_CurrentMouse.fill(false);
        for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
            int state = glfwGetMouseButton(window, button);
            s_CurrentMouse[button] = (state == GLFW_PRESS);
        }

        s_LastMousePosition = s_MousePosition;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        s_MousePosition = Vector2(static_cast<float>(xpos), static_cast<float>(ypos));

        s_GamepadActive = false;
        ReadGamepadState(GLFW_JOYSTICK_1, true);

        InputBindings::Update();
    }

    bool Input::IsKeyPressed(int keycode) {
        if (keycode < 0 || keycode > GLFW_KEY_LAST) return false;
        return s_CurrentKeys[keycode];
    }

    bool Input::IsKeyJustPressed(int keycode) {
        if (keycode < 0 || keycode > GLFW_KEY_LAST) return false;
        return s_CurrentKeys[keycode] && !s_PreviousKeys[keycode];
    }

    bool Input::IsKeyReleased(int keycode) {
        if (keycode < 0 || keycode > GLFW_KEY_LAST) return false;
        return !s_CurrentKeys[keycode] && s_PreviousKeys[keycode];
    }

    bool Input::IsMouseButtonPressed(int button) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
        return s_CurrentMouse[button];
    }

    bool Input::IsMouseButtonJustPressed(int button) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
        return s_CurrentMouse[button] && !s_PreviousMouse[button];
    }

    bool Input::IsMouseButtonReleased(int button) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
        return !s_CurrentMouse[button] && s_PreviousMouse[button];
    }

    Vector2 Input::GetMousePosition() {
        return s_MousePosition;
    }

    Vector2 Input::GetMouseDelta() {
        return s_MousePosition - s_LastMousePosition;
    }

    float Input::GetMouseX() {
        return s_MousePosition.x;
    }

    float Input::GetMouseY() {
        return s_MousePosition.y;
    }

    bool Input::IsGamepadConnected(int index) {
        return ReadGamepadState(index, false);
    }

    bool Input::IsGamepadButtonDown(GamepadButton button, int index) {
        if (!ReadGamepadState(index, false)) return false;
        int glfwButton = static_cast<int>(button);
        return glfwButton >= 0 && glfwButton <= GLFW_GAMEPAD_BUTTON_LAST &&
               s_CurrentGamepadButtons[glfwButton] == GLFW_PRESS;
    }

    bool Input::IsGamepadButtonPressed(GamepadButton button, int index) {
        if (!ReadGamepadState(index, false)) return false;
        int glfwButton = static_cast<int>(button);
        return glfwButton >= 0 && glfwButton <= GLFW_GAMEPAD_BUTTON_LAST &&
               s_CurrentGamepadButtons[glfwButton] == GLFW_PRESS &&
               s_PreviousGamepadButtons[glfwButton] != GLFW_PRESS;
    }

    bool Input::IsGamepadButtonReleased(GamepadButton button, int index) {
        if (!ReadGamepadState(index, false)) return false;
        int glfwButton = static_cast<int>(button);
        return glfwButton >= 0 && glfwButton <= GLFW_GAMEPAD_BUTTON_LAST &&
               s_CurrentGamepadButtons[glfwButton] != GLFW_PRESS &&
               s_PreviousGamepadButtons[glfwButton] == GLFW_PRESS;
    }

    float Input::GetGamepadAxis(GamepadAxis axis, int index) {
        if (!ReadGamepadState(index, false)) return 0.0f;
        int glfwAxis = static_cast<int>(axis);
        if (glfwAxis < 0 || glfwAxis > GLFW_GAMEPAD_AXIS_LAST) {
            return 0.0f;
        }
        return s_GamepadAxes[glfwAxis];
    }

    namespace Keys {
        bool Down(int keycode) {
            return Input::IsKeyPressed(keycode);
        }

        bool Pressed(int keycode) {
            return Input::IsKeyJustPressed(keycode);
        }

        bool Released(int keycode) {
            return Input::IsKeyReleased(keycode);
        }
    }

    namespace Mouse {
        bool Down(int button) {
            return Input::IsMouseButtonPressed(button);
        }

        bool Pressed(int button) {
            return Input::IsMouseButtonJustPressed(button);
        }

        bool Released(int button) {
            return Input::IsMouseButtonReleased(button);
        }

        Vector2 Position() {
            return Input::GetMousePosition();
        }

        Vector2 Delta() {
            return Input::GetMouseDelta();
        }

        float X() {
            return Input::GetMouseX();
        }

        float Y() {
            return Input::GetMouseY();
        }
    }

    namespace Gamepad {
        bool Connected(int index) {
            return Input::IsGamepadConnected(index);
        }

        bool Down(GamepadButton button, int index) {
            return Input::IsGamepadButtonDown(button, index);
        }

        bool Pressed(GamepadButton button, int index) {
            return Input::IsGamepadButtonPressed(button, index);
        }

        bool Released(GamepadButton button, int index) {
            return Input::IsGamepadButtonReleased(button, index);
        }

        float Axis(GamepadAxis axis, int index) {
            return Input::GetGamepadAxis(axis, index);
        }
    }

#ifdef SAGE_ENGINE_TESTING
    namespace Testing {

        namespace {
            void EnableOverrides() {
                s_TestOverridesEnabled = true;
            }
        }

        void ResetInputState() {
            EnableOverrides();
            s_CurrentKeys.fill(false);
            s_PreviousKeys.fill(false);
            s_CurrentMouse.fill(false);
            s_PreviousMouse.fill(false);
            s_MousePosition = Vector2::Zero();
            s_LastMousePosition = Vector2::Zero();
            s_CurrentGamepadButtons.fill(static_cast<unsigned char>(GLFW_RELEASE));
            s_PreviousGamepadButtons.fill(static_cast<unsigned char>(GLFW_RELEASE));
            s_GamepadAxes.fill(0.0f);
            s_GamepadActive = false;
        }

        void SetKeyState(int keycode, bool pressed, bool wasPressed) {
            if (keycode < 0 || keycode > GLFW_KEY_LAST) {
                return;
            }
            EnableOverrides();
            s_CurrentKeys[static_cast<std::size_t>(keycode)] = pressed;
            s_PreviousKeys[static_cast<std::size_t>(keycode)] = wasPressed;
        }

        void SetMouseButtonState(int button, bool pressed, bool wasPressed) {
            if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
                return;
            }
            EnableOverrides();
            s_CurrentMouse[static_cast<std::size_t>(button)] = pressed;
            s_PreviousMouse[static_cast<std::size_t>(button)] = wasPressed;
        }

        void SetMousePosition(const Vector2& position) {
            EnableOverrides();
            s_LastMousePosition = position;
            s_MousePosition = position;
        }

        void SetMousePosition(const Vector2& position, const Vector2& previousPosition) {
            EnableOverrides();
            s_LastMousePosition = previousPosition;
            s_MousePosition = position;
        }

        void SetGamepadAxisValue(GamepadAxis axis, float value, int index) {
            if (index != 0) {
                return;
            }

            int glfwAxis = static_cast<int>(axis);
            if (glfwAxis < 0 || glfwAxis > GLFW_GAMEPAD_AXIS_LAST) {
                return;
            }

            EnableOverrides();
            s_GamepadAxes[static_cast<std::size_t>(glfwAxis)] = value;
            s_GamepadActive = true;
        }

        void SetGamepadButtonState(GamepadButton button, bool pressed, bool wasPressed, int index) {
            if (index != 0) {
                return;
            }

            int glfwButton = static_cast<int>(button);
            if (glfwButton < 0 || glfwButton > GLFW_GAMEPAD_BUTTON_LAST) {
                return;
            }

            EnableOverrides();
            s_CurrentGamepadButtons[static_cast<std::size_t>(glfwButton)] = pressed ? GLFW_PRESS : GLFW_RELEASE;
            s_PreviousGamepadButtons[static_cast<std::size_t>(glfwButton)] = wasPressed ? GLFW_PRESS : GLFW_RELEASE;
            s_GamepadActive = true;
        }

        void SetGamepadConnected(bool connected, int index) {
            if (index != 0) {
                return;
            }

            EnableOverrides();
            s_GamepadActive = connected;
        }

    } // namespace Testing
#endif

}
