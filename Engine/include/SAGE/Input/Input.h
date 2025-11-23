#pragma once

#include "SAGE/Math/Vector2.h"
#include <functional>

namespace SAGE {

enum class KeyCode {
    Unknown = -1,
    
    // Printable keys
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    Semicolon = 59,
    Equal = 61,
    
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    
    // Function keys
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
    
    // Keypad
    KP0 = 320, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,
    
    // Modifiers
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7
};

enum class InputState {
    Released = 0,
    Pressed = 1,
    Held = 2,
    JustReleased = 3
};

enum class CursorMode {
    Normal,
    Hidden,
    Disabled
};

enum class CursorShape {
    Arrow,
    IBeam,
    Crosshair,
    Hand,
    HResize,
    VResize
};

class Camera2D;

class Input {
public:
    using KeyCallback = std::function<void(KeyCode, InputState)>;
    using MouseButtonCallback = std::function<void(MouseButton, InputState)>;
    using MouseMoveCallback = std::function<void(const Vector2&)>;
    using MouseScrollCallback = std::function<void(const Vector2&)>;
    using CharCallback = std::function<void(unsigned int)>;

    static void Init(void* windowHandle);
    static void Shutdown();
    static void Update();

    // Keyboard
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyDown(KeyCode key);
    static bool IsKeyReleased(KeyCode key);
    static bool IsKeyJustPressed(KeyCode key); // alias для Pressed

    // Mouse buttons
    static bool IsMouseButtonPressed(MouseButton button);
    static bool IsMouseButtonDown(MouseButton button);
    static bool IsMouseButtonReleased(MouseButton button);

    // Mouse position
    static Vector2 GetMousePosition();
    static Vector2 GetMousePositionWorld(const Camera2D& camera);
    static Vector2 GetMouseDelta();
    static Vector2 GetScrollDelta();

    // Cursor
    static void SetCursorMode(CursorMode mode);
    static void SetCursorShape(CursorShape shape);
    static CursorMode GetCursorMode();
    static CursorShape GetCursorShape();

    // Callbacks
    static void SetKeyCallback(KeyCallback callback);
    static void SetMouseButtonCallback(MouseButtonCallback callback);
    static void SetMouseMoveCallback(MouseMoveCallback callback);
    static void SetMouseScrollCallback(MouseScrollCallback callback);
    static void SetCharCallback(CharCallback callback);

private:
    Input() = delete;
};

} // namespace SAGE
