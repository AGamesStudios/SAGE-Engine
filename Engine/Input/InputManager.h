#pragma once

#include "InputBridge.h"
#include "InputMap.h"
#include "ActionContext.h"
#include "KeyCodes.h"
#include "IWindow.h"
#include "Math/Vector2.h"
#include <memory>
#include <shared_mutex>
#include <atomic>

// Forward declaration для обратной совместимости
struct GLFWwindow;

/**
 * @file InputManager.h
 * @brief Global input system manager - singleton interface
 */

namespace SAGE {

/**
 * @brief Global input manager (singleton)
 * 
 * Provides convenient API for input queries:
 * - Input::IsKeyPressed(Key::Space)
 * - Input::GetMousePosition()
 * - Input::IsGamepadConnected(0)
 * 
 * Wraps InputBridge and provides both:
 * 1. Direct key/button queries (simple API)
 * 2. Action-based queries (flexible, rebindable)
 */
class InputManager {
public:
    /**
     * @brief Get singleton instance
     */
    static InputManager& Get();
    
    /**
     * @brief Initialize input system with window (platform-independent)
     * @param window Абстрактный интерфейс окна (IWindow)
     * @note Идемпотентна - можно вызывать повторно без эффекта
     */
    void Initialize(IWindow* window);
    
    /**
     * @brief Initialize with GLFW window (legacy compatibility)
     * @param window GLFW window pointer
     * @note Идемпотентна - можно вызывать повторно без эффекта
     * @deprecated Используйте Initialize(IWindow*) для портируемости
     */
    void Initialize(GLFWwindow* window);
    
    /**
     * @brief Проверить, инициализирован ли менеджер
     */
    bool IsInitialized() const { return m_Initialized.load(); }
    
    /**
     * @brief Shutdown input system
     */
    void Shutdown();
    
    /**
     * @brief Update per frame (transitions Pressed->Held, Released->None)
     */
    void Update();
    
    // ============================================================================
    // KEYBOARD INPUT (Direct)
    // ============================================================================
    
    /**
     * @brief Check if key was pressed this frame (edge trigger)
     */
    bool IsKeyPressed(Key key) const;
    
    /**
     * @brief Check if key is held down
     */
    bool IsKeyHeld(Key key) const;
    
    /**
     * @brief Check if key was released this frame
     */
    bool IsKeyReleased(Key key) const;
    
    // ============================================================================
    // MOUSE INPUT (Direct)
    // ============================================================================
    
    /**
     * @brief Check if mouse button was pressed this frame
     */
    bool IsMouseButtonPressed(MouseButton button) const;
    
    /**
     * @brief Check if mouse button is held
     */
    bool IsMouseButtonHeld(MouseButton button) const;
    
    /**
     * @brief Check if mouse button was released this frame
     */
    bool IsMouseButtonReleased(MouseButton button) const;
    
    /**
     * @brief Get current mouse position (window coordinates)
     */
    Vector2 GetMousePosition() const;
    
    /**
     * @brief Get mouse movement delta since last frame
     */
    Vector2 GetMouseDelta() const;
    
    /**
     * @brief Get scroll wheel delta this frame
     */
    float GetScrollDelta() const;
    
    /**
     * @brief Consume scroll delta (set to 0 after reading)
     */
    void ConsumeScroll();
    
    // ============================================================================
    // GAMEPAD INPUT (Direct)
    // ============================================================================
    
    /**
     * @brief Check if gamepad is connected
     * @param gamepadId Gamepad index (0-15)
     */
    bool IsGamepadConnected(int gamepadId) const;
    
    /**
     * @brief Get gamepad name
     */
    const char* GetGamepadName(int gamepadId) const;
    
    /**
     * @brief Check if gamepad button was pressed this frame
     */
    bool IsGamepadButtonPressed(int gamepadId, GamepadButton button) const;
    
    /**
     * @brief Check if gamepad button is held
     */
    bool IsGamepadButtonHeld(int gamepadId, GamepadButton button) const;
    
    /**
     * @brief Check if gamepad button was released this frame
     */
    bool IsGamepadButtonReleased(int gamepadId, GamepadButton button) const;
    
    /**
     * @brief Get gamepad axis value (-1.0 to 1.0)
     */
    float GetGamepadAxis(int gamepadId, GamepadAxis axis) const;
    
    // ============================================================================
    // ACTION-BASED INPUT (Advanced)
    // ============================================================================
    
    /**
     * @brief Get default input map
     */
    InputMap* GetInputMap() { return &m_InputMap; }
    const InputMap* GetInputMap() const { return &m_InputMap; }
    
    /**
     * @brief Get context manager
     */
    ActionContextManager* GetContextManager() { return &m_ContextManager; }
    const ActionContextManager* GetContextManager() const { return &m_ContextManager; }
    
    /**
     * @brief Check if action is pressed (via InputMap or active context)
     */
    bool IsActionPressed(const std::string& actionName) const;
    
    /**
     * @brief Check if action is held
     */
    bool IsActionHeld(const std::string& actionName) const;
    
    /**
     * @brief Check if action is released
     */
    bool IsActionReleased(const std::string& actionName) const;
    
    /**
     * @brief Get action value (for analog inputs like gamepad axes)
     */
    float GetActionValue(const std::string& actionName) const;
    
    /**
     * @brief Get input bridge (advanced usage)
     */
    InputBridge* GetBridge() { return &m_InputBridge; }
    const InputBridge* GetBridge() const { return &m_InputBridge; }
    
    // Internal access for callbacks
    struct KeyState {
        bool pressedThisFrame = false;
        bool held = false;
        bool releasedThisFrame = false;
    };
    
    struct GamepadState {
        bool connected = false;
        std::string name;
        bool buttonsPressedThisFrame[15] = {false};
        bool buttonsHeld[15] = {false};
        bool buttonsReleasedThisFrame[15] = {false};
        float axes[6] = {0.0f};
    };
    
    KeyState* GetKeyStates() { return m_KeyStates; }
    KeyState* GetMouseButtonStates() { return m_MouseButtonStates; }
    GamepadState* GetGamepadStates() { return m_Gamepads; }
    
private:
    InputManager() = default;
    ~InputManager() = default;
    
    // Non-copyable
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    InputBridge m_InputBridge;
    InputMap m_InputMap;
    ActionContextManager m_ContextManager;
    
    /**
     * @brief Thread-safety mutex (mutable for use in const methods)
     * 
     * Marked mutable to allow locking in const query methods (e.g., IsKeyPressed()).
     * Const correctness refers to logical constness (not mutating input state),
     * while mutex locking is an implementation detail for thread-safety.
     * 
     * Uses std::shared_mutex for reader-writer pattern:
     * - Query methods (IsKey*, IsMouse*, GetAction*) use std::shared_lock
     * - Update/initialization use std::unique_lock
     */
    mutable std::shared_mutex m_Mutex;
    std::atomic<bool> m_Initialized{false}; // Idempotent initialization guard
    IWindow* m_Window = nullptr; // Platform-independent window interface
    void* m_WindowHandle = nullptr; // For re-initialization check
    
    // FIXED BUG #4: Thread-safe state (not global static)
    KeyState m_KeyStates[512];
    KeyState m_MouseButtonStates[8];
    GamepadState m_Gamepads[16]; // GLFW_JOYSTICK_LAST + 1
};

// ============================================================================
// CONVENIENCE NAMESPACE (Global functions)
// ============================================================================

namespace Input {
    // Keyboard
    inline bool IsKeyPressed(Key key) { return InputManager::Get().IsKeyPressed(key); }
    inline bool IsKeyHeld(Key key) { return InputManager::Get().IsKeyHeld(key); }
    inline bool IsKeyReleased(Key key) { return InputManager::Get().IsKeyReleased(key); }
    
    // Mouse
    inline bool IsMouseButtonPressed(MouseButton button) { return InputManager::Get().IsMouseButtonPressed(button); }
    inline bool IsMouseButtonHeld(MouseButton button) { return InputManager::Get().IsMouseButtonHeld(button); }
    inline bool IsMouseButtonReleased(MouseButton button) { return InputManager::Get().IsMouseButtonReleased(button); }
    inline Vector2 GetMousePosition() { return InputManager::Get().GetMousePosition(); }
    inline Vector2 GetMouseDelta() { return InputManager::Get().GetMouseDelta(); }
    inline float GetScrollDelta() { return InputManager::Get().GetScrollDelta(); }
    inline void ConsumeScroll() { InputManager::Get().ConsumeScroll(); }
    
    // Gamepad
    inline bool IsGamepadConnected(int id) { return InputManager::Get().IsGamepadConnected(id); }
    inline const char* GetGamepadName(int id) { return InputManager::Get().GetGamepadName(id); }
    inline bool IsGamepadButtonPressed(int id, GamepadButton button) { return InputManager::Get().IsGamepadButtonPressed(id, button); }
    inline bool IsGamepadButtonHeld(int id, GamepadButton button) { return InputManager::Get().IsGamepadButtonHeld(id, button); }
    inline bool IsGamepadButtonReleased(int id, GamepadButton button) { return InputManager::Get().IsGamepadButtonReleased(id, button); }
    inline float GetGamepadAxis(int id, GamepadAxis axis) { return InputManager::Get().GetGamepadAxis(id, axis); }
    
    // Actions
    inline bool IsActionPressed(const std::string& action) { return InputManager::Get().IsActionPressed(action); }
    inline bool IsActionHeld(const std::string& action) { return InputManager::Get().IsActionHeld(action); }
    inline bool IsActionReleased(const std::string& action) { return InputManager::Get().IsActionReleased(action); }
    inline float GetActionValue(const std::string& action) { return InputManager::Get().GetActionValue(action); }
    
    // Advanced
    inline InputMap* GetInputMap() { return InputManager::Get().GetInputMap(); }
    inline ActionContextManager* GetContextManager() { return InputManager::Get().GetContextManager(); }
}

} // namespace SAGE
