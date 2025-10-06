#pragma once

#include "../Math/Vector2.h"

namespace SAGE {

    enum class GamepadButton {
        A,
        B,
        X,
        Y,
        LeftBumper,
        RightBumper,
        Back,
        Start,
        Guide,
        LeftThumb,
        RightThumb,
        DPadUp,
        DPadRight,
        DPadDown,
        DPadLeft
    };

    enum class GamepadAxis {
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger
    };

    class Input {
    public:
        static void Init();
        static void Update();

        static bool IsKeyPressed(int keycode);   // текущее состояние (зажата ли клавиша)
        static bool IsKeyJustPressed(int keycode); // нажатие в текущем кадре
        static bool IsKeyReleased(int keycode);    // отпускание в текущем кадре

        static bool IsMouseButtonPressed(int button);
        static bool IsMouseButtonJustPressed(int button);
        static bool IsMouseButtonReleased(int button);

        static Vector2 GetMousePosition();
        static Vector2 GetMouseDelta();
        static float GetMouseX();
        static float GetMouseY();

        static bool IsGamepadConnected(int index = 0);
        static bool IsGamepadButtonDown(GamepadButton button, int index = 0);
        static bool IsGamepadButtonPressed(GamepadButton button, int index = 0);
        static bool IsGamepadButtonReleased(GamepadButton button, int index = 0);
        static float GetGamepadAxis(GamepadAxis axis, int index = 0);
    };

    namespace Keys {
        bool Down(int keycode);
        bool Pressed(int keycode);
        bool Released(int keycode);
    }

    namespace Mouse {
        bool Down(int button);
        bool Pressed(int button);
        bool Released(int button);

        Vector2 Position();
        Vector2 Delta();
        float X();
        float Y();
    }

    namespace Gamepad {
        bool Connected(int index = 0);
        bool Down(GamepadButton button, int index = 0);
        bool Pressed(GamepadButton button, int index = 0);
        bool Released(GamepadButton button, int index = 0);
        float Axis(GamepadAxis axis, int index = 0);
    }

#ifdef SAGE_ENGINE_TESTING
    namespace Testing {
        void ResetInputState();
        void SetKeyState(int keycode, bool pressed, bool wasPressed);
        void SetMouseButtonState(int button, bool pressed, bool wasPressed);
        void SetMousePosition(const Vector2& position);
        void SetMousePosition(const Vector2& position, const Vector2& previousPosition);
        void SetGamepadAxisValue(GamepadAxis axis, float value, int index = 0);
        void SetGamepadButtonState(GamepadButton button, bool pressed, bool wasPressed, int index = 0);
        void SetGamepadConnected(bool connected, int index = 0);
    }
#endif

}
