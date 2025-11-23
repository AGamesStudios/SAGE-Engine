#include "SAGE/Input/Input.h"
#include "SAGE/Log.h"
#include "SAGE/Graphics/Camera2D.h"

#include <GLFW/glfw3.h>
#include <array>
#include <cstring>
#include <unordered_map>
#include <algorithm> // For std::fill

namespace SAGE {

namespace {
    struct InputData {
        GLFWwindow* window = nullptr;
        
        // Key states
        std::array<bool, 512> keysDown{};
        std::array<bool, 512> keysPressed{};
        std::array<bool, 512> keysReleased{};

        // Mouse button states
        std::array<bool, 16> mouseButtonsDown{};
        std::array<bool, 16> mouseButtonsPressed{};
        std::array<bool, 16> mouseButtonsReleased{};
        
        Vector2 mousePos{};
        Vector2 lastMousePos{};
        Vector2 scrollDelta{};
        
        Input::KeyCallback keyCallback;
        Input::MouseButtonCallback mouseButtonCallback;
        Input::MouseMoveCallback mouseMoveCallback;
        Input::MouseScrollCallback mouseScrollCallback;
        Input::CharCallback charCallback;

        CursorMode cursorMode = CursorMode::Normal;
        CursorShape cursorShape = CursorShape::Arrow;
        std::unordered_map<CursorShape, GLFWcursor*> cursors;
    };

    InputData& GetInputData() {
        static InputData data;
        return data;
    }

    void GLFWKeyCallback(GLFWwindow*, int key, int, int action, int) {
        auto& data = GetInputData();
        if (key < 0 || key >= 512) return;

        InputState state = InputState::Released;
        if (action == GLFW_PRESS) {
            state = InputState::Pressed;
            data.keysDown[key] = true;
            data.keysPressed[key] = true;
        } else if (action == GLFW_REPEAT) {
            state = InputState::Held;
            data.keysDown[key] = true;
        } else if (action == GLFW_RELEASE) {
            state = InputState::JustReleased;
            data.keysDown[key] = false;
            data.keysReleased[key] = true;
        }

        if (data.keyCallback) {
            data.keyCallback(static_cast<KeyCode>(key), state);
        }
    }

    void GLFWMouseButtonCallback(GLFWwindow*, int button, int action, int) {
        auto& data = GetInputData();
        if (button < 0 || button >= 16) return;

        InputState state = InputState::Released;
        if (action == GLFW_PRESS) {
            state = InputState::Pressed;
            data.mouseButtonsDown[button] = true;
            data.mouseButtonsPressed[button] = true;
        } else if (action == GLFW_RELEASE) {
            state = InputState::JustReleased;
            data.mouseButtonsDown[button] = false;
            data.mouseButtonsReleased[button] = true;
        }
        
        if (data.mouseButtonCallback) {
            data.mouseButtonCallback(static_cast<MouseButton>(button), state);
        }
    }

    void GLFWCursorPosCallback(GLFWwindow*, double xpos, double ypos) {
        auto& data = GetInputData();
        data.mousePos = {static_cast<float>(xpos), static_cast<float>(ypos)};

        if (data.mouseMoveCallback) {
            data.mouseMoveCallback(data.mousePos);
        }
    }

    void GLFWScrollCallback(GLFWwindow*, double xoffset, double yoffset) {
        auto& data = GetInputData();
        data.scrollDelta.x += static_cast<float>(xoffset);
        data.scrollDelta.y += static_cast<float>(yoffset);

        if (data.mouseScrollCallback) {
            data.mouseScrollCallback({static_cast<float>(xoffset), static_cast<float>(yoffset)});
        }
    }

    void GLFWCharCallback(GLFWwindow*, unsigned int codepoint) {
        auto& data = GetInputData();
        if (data.charCallback) {
            data.charCallback(codepoint);
        }
    }
}

void Input::Init(void* windowHandle) {
    auto& data = GetInputData();
    data.window = static_cast<GLFWwindow*>(windowHandle);

    if (!data.window) {
        SAGE_ERROR("Input::Init - Null window handle provided");
        return;
    }

    glfwSetKeyCallback(data.window, GLFWKeyCallback);
    glfwSetMouseButtonCallback(data.window, GLFWMouseButtonCallback);
    glfwSetCursorPosCallback(data.window, GLFWCursorPosCallback);
    glfwSetScrollCallback(data.window, GLFWScrollCallback);
    glfwSetCharCallback(data.window, GLFWCharCallback);

    double xpos, ypos;
    glfwGetCursorPos(data.window, &xpos, &ypos);
    data.mousePos = {static_cast<float>(xpos), static_cast<float>(ypos)};
    data.lastMousePos = data.mousePos;

    // Initialize cursors
    data.cursors[CursorShape::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    data.cursors[CursorShape::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    data.cursors[CursorShape::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    data.cursors[CursorShape::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    data.cursors[CursorShape::HResize] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    data.cursors[CursorShape::VResize] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
}

void Input::Shutdown() {
    auto& data = GetInputData();
    if (data.window) {
        glfwSetKeyCallback(data.window, nullptr);
        glfwSetMouseButtonCallback(data.window, nullptr);
        glfwSetCursorPosCallback(data.window, nullptr);
        glfwSetScrollCallback(data.window, nullptr);
    }

    for (auto& [shape, cursor] : data.cursors) {
        if (cursor) {
            glfwDestroyCursor(cursor);
        }
    }
    data.cursors.clear();
}

void Input::Update() {
    auto& data = GetInputData();

    // Clear per-frame states
    std::fill(data.keysPressed.begin(), data.keysPressed.end(), false);
    std::fill(data.keysReleased.begin(), data.keysReleased.end(), false);
    std::fill(data.mouseButtonsPressed.begin(), data.mouseButtonsPressed.end(), false);
    std::fill(data.mouseButtonsReleased.begin(), data.mouseButtonsReleased.end(), false);

    // Update mouse delta
    data.lastMousePos = data.mousePos;
    
    // Reset scroll delta
    data.scrollDelta = Vector2::Zero();
}

bool Input::IsKeyPressed(KeyCode key) {
    auto& data = GetInputData();
    const int keyIndex = static_cast<int>(key);
    if (keyIndex < 0 || keyIndex >= 512) return false;
    return data.keysPressed[keyIndex];
}

bool Input::IsKeyDown(KeyCode key) {
    auto& data = GetInputData();
    const int keyIndex = static_cast<int>(key);
    if (keyIndex < 0 || keyIndex >= 512) return false;
    return data.keysDown[keyIndex];
}

bool Input::IsKeyReleased(KeyCode key) {
    auto& data = GetInputData();
    const int keyIndex = static_cast<int>(key);
    if (keyIndex < 0 || keyIndex >= 512) return false;
    return data.keysReleased[keyIndex];
}

bool Input::IsKeyJustPressed(KeyCode key) {
    return IsKeyPressed(key);
}

bool Input::IsMouseButtonPressed(MouseButton button) {
    auto& data = GetInputData();
    const int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 16) return false;
    return data.mouseButtonsPressed[buttonIndex];
}

bool Input::IsMouseButtonDown(MouseButton button) {
    auto& data = GetInputData();
    const int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 16) return false;
    return data.mouseButtonsDown[buttonIndex];
}

bool Input::IsMouseButtonReleased(MouseButton button) {
    auto& data = GetInputData();
    const int buttonIndex = static_cast<int>(button);
    if (buttonIndex < 0 || buttonIndex >= 16) return false;
    return data.mouseButtonsReleased[buttonIndex];
}

Vector2 Input::GetMousePosition() {
    return GetInputData().mousePos;
}

Vector2 Input::GetMouseDelta() {
    auto& data = GetInputData();
    return data.mousePos - data.lastMousePos;
}

Vector2 Input::GetScrollDelta() {
    return GetInputData().scrollDelta;
}

void Input::SetKeyCallback(KeyCallback callback) {
    GetInputData().keyCallback = std::move(callback);
}

void Input::SetMouseButtonCallback(MouseButtonCallback callback) {
    GetInputData().mouseButtonCallback = std::move(callback);
}

void Input::SetMouseMoveCallback(MouseMoveCallback callback) {
    GetInputData().mouseMoveCallback = std::move(callback);
}

void Input::SetMouseScrollCallback(MouseScrollCallback callback) {
    GetInputData().mouseScrollCallback = std::move(callback);
}

void Input::SetCharCallback(CharCallback callback) {
    GetInputData().charCallback = std::move(callback);
}

Vector2 Input::GetMousePositionWorld(const Camera2D& camera) {
    return camera.ScreenToWorld(GetMousePosition());
}

void Input::SetCursorMode(CursorMode mode) {
    auto& data = GetInputData();
    data.cursorMode = mode;
    
    int value = GLFW_CURSOR_NORMAL;
    switch (mode) {
        case CursorMode::Normal: value = GLFW_CURSOR_NORMAL; break;
        case CursorMode::Hidden: value = GLFW_CURSOR_HIDDEN; break;
        case CursorMode::Disabled: value = GLFW_CURSOR_DISABLED; break;
    }
    
    glfwSetInputMode(data.window, GLFW_CURSOR, value);
}

void Input::SetCursorShape(CursorShape shape) {
    auto& data = GetInputData();
    data.cursorShape = shape;
    
    if (data.cursors.find(shape) != data.cursors.end()) {
        glfwSetCursor(data.window, data.cursors[shape]);
    }
}

CursorMode Input::GetCursorMode() {
    return GetInputData().cursorMode;
}

CursorShape Input::GetCursorShape() {
    return GetInputData().cursorShape;
}

} // namespace SAGE
