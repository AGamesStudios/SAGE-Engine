#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Input/KeyCodes.h"
#include "Input/MouseButtons.h"
#include "Input/InputBridge.h"
#include "Input/InputMap.h"
#include "Input/InputAction.h"
#include "Input/InputManager.h"
#include "Math/Vector2.h"
#include "Scripting/Lua/Core/LuaForward.h"

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief Input system bindings for Lua
     * 
     * Provides keyboard, mouse, and gamepad input access.
     */
    class InputBindings {
    public:
        static void BindAll(sol::state& lua, InputBridge* inputBridge = nullptr) {
            s_InputBridge = inputBridge;
            
            BindKeys(lua);
            BindMouse(lua);
            BindInput(lua);
        }

    private:
        static InputBridge* s_InputBridge;
        
        static void BindKeys(sol::state& lua) {
            // Key enum - основные клавиши
            lua.new_enum("Key",
                "Space", SAGE_KEY_SPACE,
                "Enter", SAGE_KEY_ENTER,
                "Escape", SAGE_KEY_ESCAPE,
                "Tab", SAGE_KEY_TAB,
                "Backspace", SAGE_KEY_BACKSPACE,
                
                // Arrow keys
                "Up", SAGE_KEY_UP,
                "Down", SAGE_KEY_DOWN,
                "Left", SAGE_KEY_LEFT,
                "Right", SAGE_KEY_RIGHT,
                
                // Letters
                "A", SAGE_KEY_A, "B", SAGE_KEY_B, "C", SAGE_KEY_C, "D", SAGE_KEY_D,
                "E", SAGE_KEY_E, "F", SAGE_KEY_F, "G", SAGE_KEY_G, "H", SAGE_KEY_H,
                "I", SAGE_KEY_I, "J", SAGE_KEY_J, "K", SAGE_KEY_K, "L", SAGE_KEY_L,
                "M", SAGE_KEY_M, "N", SAGE_KEY_N, "O", SAGE_KEY_O, "P", SAGE_KEY_P,
                "Q", SAGE_KEY_Q, "R", SAGE_KEY_R, "S", SAGE_KEY_S, "T", SAGE_KEY_T,
                "U", SAGE_KEY_U, "V", SAGE_KEY_V, "W", SAGE_KEY_W, "X", SAGE_KEY_X,
                "Y", SAGE_KEY_Y, "Z", SAGE_KEY_Z,
                
                // Numbers
                "Num0", SAGE_KEY_0, "Num1", SAGE_KEY_1, "Num2", SAGE_KEY_2,
                "Num3", SAGE_KEY_3, "Num4", SAGE_KEY_4, "Num5", SAGE_KEY_5,
                "Num6", SAGE_KEY_6, "Num7", SAGE_KEY_7, "Num8", SAGE_KEY_8,
                "Num9", SAGE_KEY_9,
                
                // Function keys
                "F1", SAGE_KEY_F1, "F2", SAGE_KEY_F2, "F3", SAGE_KEY_F3, "F4", SAGE_KEY_F4,
                "F5", SAGE_KEY_F5, "F6", SAGE_KEY_F6, "F7", SAGE_KEY_F7, "F8", SAGE_KEY_F8,
                "F9", SAGE_KEY_F9, "F10", SAGE_KEY_F10, "F11", SAGE_KEY_F11, "F12", SAGE_KEY_F12,
                
                // Modifiers
                "LeftShift", SAGE_KEY_LEFT_SHIFT,
                "RightShift", SAGE_KEY_RIGHT_SHIFT,
                "LeftCtrl", SAGE_KEY_LEFT_CONTROL,
                "RightCtrl", SAGE_KEY_RIGHT_CONTROL,
                "LeftAlt", SAGE_KEY_LEFT_ALT,
                "RightAlt", SAGE_KEY_RIGHT_ALT
            );
        }

        static void BindMouse(sol::state& lua) {
            // MouseButton enum
            lua.new_enum("MouseButton",
                "Left", MouseButton::Left,
                "Right", MouseButton::Right,
                "Middle", MouseButton::Middle,
                "Button4", MouseButton::Button4,
                "Button5", MouseButton::Button5
            );
        }

        static void BindInput(sol::state& lua) {
            // Input API (статические функции через table)
            auto inputTable = lua.create_table();
            
            // FIXED: Real implementation via InputManager
            inputTable["IsKeyDown"] = [](Key key) -> bool {
                return InputManager::Get().IsKeyHeld(key);
            };
            
            inputTable["IsKeyPressed"] = [](Key key) -> bool {
                return InputManager::Get().IsKeyPressed(key);
            };
            
            inputTable["IsKeyReleased"] = [](Key key) -> bool {
                return InputManager::Get().IsKeyReleased(key);
            };
            
            inputTable["IsMouseButtonDown"] = [](MouseButton button) -> bool {
                return InputManager::Get().IsMouseButtonHeld(button);
            };
            
            inputTable["IsMouseButtonPressed"] = [](MouseButton button) -> bool {
                return InputManager::Get().IsMouseButtonPressed(button);
            };
            
            inputTable["GetMousePosition"] = []() -> Vector2 {
                return InputManager::Get().GetMousePosition();
            };
            
            inputTable["GetMouseDelta"] = []() -> Vector2 {
                return InputManager::Get().GetMouseDelta();
            };
            
            inputTable["GetMouseScroll"] = []() -> float {
                return InputManager::Get().GetScrollDelta();
            };
            
            lua["Input"] = inputTable;
        }
    };

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    class InputBindings {
    public:
        static void BindAll(sol::state&, InputBridge* = nullptr) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif
