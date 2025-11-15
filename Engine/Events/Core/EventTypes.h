#pragma once

namespace SAGE {

/// @brief Типы событий в системе
enum class EventType {
    None = 0,
    
    // ===== Window Events =====
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    WindowMinimized,
    WindowMaximized,
    WindowRestored,
    
    // ===== Application Events =====
    AppTick,
    AppUpdate,
    AppRender,
    AppInit,
    AppShutdown,
    
    // ===== Keyboard Events =====
    KeyPressed,
    KeyReleased,
    KeyRepeat,
    CharInput,
    
    // ===== Mouse Events =====
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    MouseEnter,
    MouseLeave,
    
    // ===== Gamepad Events =====
    GamepadConnected,
    GamepadDisconnected,
    GamepadButtonPressed,
    GamepadButtonReleased,
    GamepadAxis,
    GamepadTrigger,
    
    // ===== UI Events =====
    UIMousePressed,
    UIMouseReleased,
    UIMouseEnter,
    UIMouseLeave,
    UIMouseMoved,
    UIFocus,
    UIBlur,
    UITextInput,
    UITextChanged,
    UIDragStart,
    UIDragMove,
    UIDragEnd,
    UIDrop,
    UIClick,
    UIDoubleClick,
    
    // ===== Physics Events =====
    CollisionBegin,
    CollisionEnd,
    CollisionPreSolve,
    CollisionPostSolve,
    TriggerEnter,
    TriggerExit,
    PhysicsStep,
    PhysicsTransformUpdated,
    
    // ===== Camera Events =====
    CameraMoved,
    CameraZoomed,
    CameraRotated,
    CameraShake,
    
    // ===== Cursor Events =====
    CursorModeChanged,
    
    // ===== Audio Events =====
    AudioPlay,
    AudioStop,
    AudioPause,
    AudioResume,
    
    // ===== Resource Events =====
    AssetLoaded,
    AssetUnloaded,
    AssetError,
    
    // ===== Network Events (Future) =====
    NetworkConnected,
    NetworkDisconnected,
    NetworkData,
    
    // ===== Custom Events =====
    Custom = 1000
};

/// @brief Категории событий (битовые флаги)
enum EventCategory {
    EventCategoryNone           = 0,
    EventCategoryApplication    = 1 << 0,
    EventCategoryInput          = 1 << 1,
    EventCategoryKeyboard       = 1 << 2,
    EventCategoryMouse          = 1 << 3,
    EventCategoryMouseButton    = 1 << 4,
    EventCategoryGamepad        = 1 << 5,
    EventCategoryCursor         = 1 << 6,
    EventCategoryPhysics        = 1 << 7,
    EventCategoryCamera         = 1 << 8,
    EventCategoryUI             = 1 << 9,
    EventCategoryAudio          = 1 << 10,
    EventCategoryResource       = 1 << 11,
    EventCategoryNetwork        = 1 << 12,
    EventCategoryWindow         = 1 << 13
};

/// @brief Приоритеты событий
enum class EventPriority : int {
    Lowest      = -100,
    Low         = -50,
    Normal      = 0,
    High        = 50,
    Highest     = 100,
    Critical    = 1000
};

} // namespace SAGE
