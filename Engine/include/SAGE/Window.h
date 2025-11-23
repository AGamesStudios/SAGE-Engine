#pragma once

#include "SAGE/WindowConfig.h"
#include "SAGE/Core/Event.h"

#include <functional>
#include <memory>

namespace SAGE {

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    // Factory method
    static std::unique_ptr<Window> Create(const WindowConfig& config);

    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;

    virtual bool ShouldClose() const = 0;
    virtual void RequestClose() = 0;

    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    virtual void* GetNativeHandle() const = 0;
    virtual const WindowConfig& GetConfig() const = 0;
    
    virtual void SetResizeCallback(std::function<void(int, int)> callback) = 0;
    virtual void GetFramebufferSize(int& width, int& height) const = 0;
    virtual void SetFocusCallback(std::function<void(bool)> callback) = 0;
    virtual void SetCloseCallback(std::function<void()> callback) = 0;
    
    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

    // Fullscreen control
    virtual void SetFullscreen(bool enable) = 0;
    virtual void ToggleFullscreen() = 0;
    virtual bool IsFullscreen() const = 0;

    enum class WindowMode {
        Windowed,
        Fullscreen,
        Borderless
    };

    // Window mode control
    virtual void SetWindowMode(WindowMode mode) = 0;
    virtual WindowMode GetWindowMode() const = 0;

    // Aspect ratio control
    virtual void SetAspectRatio(int numerator, int denominator) = 0;

protected:
    Window() = default;
};

} // namespace SAGE
