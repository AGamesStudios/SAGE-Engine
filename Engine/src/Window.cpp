#include "SAGE/Window.h"

#include "SAGE/Log.h"
#include "SAGE/Core/KeyEvents.h"
#include "SAGE/Core/MouseEvents.h"
#include "SAGE/Core/ApplicationEvents.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <stdexcept>
#include <utility>

namespace SAGE {
namespace {
    std::atomic<int>& WindowCount() {
        static std::atomic<int> count{0};
        return count;
    }

    void GLFWErrorCallback(int errorCode, const char* description) {
        SAGE_ERROR("[GLFW] (", errorCode, ") ", description);
    }

    void EnsureGLFWInitialised() {
        if (WindowCount().load() > 0) {
            return;
        }

        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("Failed to initialise GLFW");
        }

        glfwSetErrorCallback(GLFWErrorCallback);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif
    }

    void ShutdownGLFW() {
        if (WindowCount().load() == 0) {
            glfwTerminate();
        }
    }
}

class GlfwWindow : public Window {
public:
    GlfwWindow(const WindowConfig& config);
    ~GlfwWindow() override;

    void PollEvents() override;
    void SwapBuffers() override;

    bool ShouldClose() const override;
    void RequestClose() override;

    void SetVSync(bool enabled) override;
    bool IsVSync() const override { return m_Config.vsync; }

    void* GetNativeHandle() const override { return m_Handle; }
    const WindowConfig& GetConfig() const override { return m_Config; }
    
    void SetResizeCallback(std::function<void(int, int)> callback) override;
    void GetFramebufferSize(int& width, int& height) const override;
    void SetFocusCallback(std::function<void(bool)> callback) override;
    void SetCloseCallback(std::function<void()> callback) override;
    
    void SetEventCallback(const EventCallbackFn& callback) override { m_EventCallback = callback; }

    void SetFullscreen(bool enable) override;
    void ToggleFullscreen() override;
    bool IsFullscreen() const override { return m_Fullscreen; }

    void SetWindowMode(WindowMode mode) override;
    WindowMode GetWindowMode() const override { return m_WindowMode; }

    void SetAspectRatio(int numerator, int denominator) override;

private:
    void Destroy();
    void HandleFramebufferResize(int width, int height);
    void HandleFocusChanged(bool focused);
    void HandleCloseRequested();

    static void FramebufferSizeThunk(GLFWwindow* window, int width, int height);
    static void FocusThunk(GLFWwindow* window, int focused);
    static void CloseThunk(GLFWwindow* window);
    static void KeyThunk(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonThunk(GLFWwindow* window, int button, int action, int mods);
    static void ScrollThunk(GLFWwindow* window, double xoffset, double yoffset);
    static void CursorPosThunk(GLFWwindow* window, double xpos, double ypos);
    static void CharThunk(GLFWwindow* window, unsigned int keycode);

    WindowConfig m_Config;
    GLFWwindow* m_Handle = nullptr;
    
    std::function<void(int, int)> m_ResizeCallback;
    std::function<void(bool)> m_FocusCallback;
    std::function<void()> m_CloseCallback;
    EventCallbackFn m_EventCallback;

    int m_FramebufferWidth = 0;
    int m_FramebufferHeight = 0;
    bool m_Fullscreen = false;
    int m_WindowedX = 0;
    int m_WindowedY = 0;
    int m_WindowedWidth = 0;
    int m_WindowedHeight = 0;
    WindowMode m_WindowMode = WindowMode::Windowed;
    int m_AspectRatioNumerator = 0;
    int m_AspectRatioDenominator = 0;
};

std::unique_ptr<Window> Window::Create(const WindowConfig& config) {
    return std::make_unique<GlfwWindow>(config);
}

GlfwWindow::GlfwWindow(const WindowConfig& config)
    : m_Config(config) {
    EnsureGLFWInitialised();

    glfwWindowHint(GLFW_SAMPLES, config.samples);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* monitor = nullptr;
    if (config.fullscreen) {
        monitor = glfwGetPrimaryMonitor();
    }

    m_Handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
    if (!m_Handle) {
        ShutdownGLFW();
        throw std::runtime_error("Failed to create GLFW window");
    }

    WindowCount()++;

    glfwMakeContextCurrent(m_Handle);
    glfwSetWindowUserPointer(m_Handle, this);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
        WindowCount()--;
        ShutdownGLFW();
        throw std::runtime_error("Failed to initialise GLAD");
    }

    if (config.samples > 0) {
        glEnable(GL_MULTISAMPLE);
    }

    SetVSync(config.vsync);
    glfwSetFramebufferSizeCallback(m_Handle, FramebufferSizeThunk);
    glfwSetWindowFocusCallback(m_Handle, FocusThunk);
    glfwSetWindowCloseCallback(m_Handle, CloseThunk);
    
    glfwSetKeyCallback(m_Handle, KeyThunk);
    glfwSetMouseButtonCallback(m_Handle, MouseButtonThunk);
    glfwSetScrollCallback(m_Handle, ScrollThunk);
    glfwSetCursorPosCallback(m_Handle, CursorPosThunk);
    glfwSetCharCallback(m_Handle, CharThunk);

    int fbWidth = 0;
    int fbHeight = 0;
    glfwGetFramebufferSize(m_Handle, &fbWidth, &fbHeight);
    HandleFramebufferResize(fbWidth, fbHeight);
}

GlfwWindow::~GlfwWindow() {
    Destroy();
}

void GlfwWindow::Destroy() {
    if (!m_Handle) {
        return;
    }

    glfwSetWindowUserPointer(m_Handle, nullptr);
    glfwDestroyWindow(m_Handle);
    m_Handle = nullptr;

    WindowCount()--;
    ShutdownGLFW();

    m_FramebufferWidth = 0;
    m_FramebufferHeight = 0;
}

void GlfwWindow::PollEvents() {
    glfwPollEvents();
}

void GlfwWindow::SwapBuffers() {
    if (m_Handle) {
        glfwSwapBuffers(m_Handle);
    }
}

bool GlfwWindow::ShouldClose() const {
    return m_Handle ? glfwWindowShouldClose(m_Handle) == GLFW_TRUE : true;
}

void GlfwWindow::RequestClose() {
    if (m_Handle) {
        glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);
    }
}

void GlfwWindow::SetVSync(bool enabled) {
    if (!m_Handle) {
        return;
    }

    glfwSwapInterval(enabled ? 1 : 0);
    m_Config.vsync = enabled;
}

void GlfwWindow::SetResizeCallback(std::function<void(int, int)> callback) {
    m_ResizeCallback = std::move(callback);
    if (m_ResizeCallback && m_FramebufferWidth > 0 && m_FramebufferHeight > 0) {
        m_ResizeCallback(m_FramebufferWidth, m_FramebufferHeight);
    }
}

void GlfwWindow::SetFocusCallback(std::function<void(bool)> callback) {
    m_FocusCallback = std::move(callback);
}

void GlfwWindow::SetCloseCallback(std::function<void()> callback) {
    m_CloseCallback = std::move(callback);
}

void GlfwWindow::GetFramebufferSize(int& width, int& height) const {
    width = m_FramebufferWidth;
    height = m_FramebufferHeight;
}

void GlfwWindow::HandleFramebufferResize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    m_FramebufferWidth = width;
    m_FramebufferHeight = height;
    m_Config.width = width;
    m_Config.height = height;

    if (m_ResizeCallback) {
        m_ResizeCallback(width, height);
    }
    
    if (m_EventCallback) {
        WindowResizeEvent event(width, height);
        m_EventCallback(event);
    }
}

void GlfwWindow::HandleFocusChanged(bool focused) {
    if (m_FocusCallback) {
        m_FocusCallback(focused);
    }
    
    if (m_EventCallback) {
        if (focused) {
            WindowFocusEvent event;
            m_EventCallback(event);
        } else {
            WindowLostFocusEvent event;
            m_EventCallback(event);
        }
    }
}

void GlfwWindow::HandleCloseRequested() {
    if (m_CloseCallback) {
        m_CloseCallback();
    } else {
        RequestClose();
    }
    
    if (m_EventCallback) {
        WindowCloseEvent event;
        m_EventCallback(event);
    }
}

void GlfwWindow::FramebufferSizeThunk(GLFWwindow* window, int width, int height) {
    if (!window) return;
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (self) self->HandleFramebufferResize(width, height);
}

void GlfwWindow::FocusThunk(GLFWwindow* window, int focused) {
    if (!window) return;
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (self) self->HandleFocusChanged(focused == GLFW_TRUE);
}

void GlfwWindow::CloseThunk(GLFWwindow* window) {
    if (!window) return;
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (self) self->HandleCloseRequested();
}

void GlfwWindow::KeyThunk(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_EventCallback) return;

    switch (action) {
        case GLFW_PRESS: {
            KeyPressedEvent event(static_cast<KeyCode>(key), 0);
            self->m_EventCallback(event);
            break;
        }
        case GLFW_RELEASE: {
            KeyReleasedEvent event(static_cast<KeyCode>(key));
            self->m_EventCallback(event);
            break;
        }
        case GLFW_REPEAT: {
            KeyPressedEvent event(static_cast<KeyCode>(key), 1);
            self->m_EventCallback(event);
            break;
        }
    }
}

void GlfwWindow::MouseButtonThunk(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_EventCallback) return;

    switch (action) {
        case GLFW_PRESS: {
            MouseButtonPressedEvent event(static_cast<MouseButton>(button));
            self->m_EventCallback(event);
            break;
        }
        case GLFW_RELEASE: {
            MouseButtonReleasedEvent event(static_cast<MouseButton>(button));
            self->m_EventCallback(event);
            break;
        }
    }
}

void GlfwWindow::ScrollThunk(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_EventCallback) return;

    MouseScrolledEvent event((float)xoffset, (float)yoffset);
    self->m_EventCallback(event);
}

void GlfwWindow::CursorPosThunk(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_EventCallback) return;

    MouseMovedEvent event((float)xpos, (float)ypos);
    self->m_EventCallback(event);
}

void GlfwWindow::CharThunk(GLFWwindow* window, unsigned int keycode) {
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (!self || !self->m_EventCallback) return;

    KeyTypedEvent event(static_cast<KeyCode>(keycode));
    self->m_EventCallback(event);
}

void GlfwWindow::SetFullscreen(bool enable) {
    if (!m_Handle) return;
    if (enable == m_Fullscreen) return;

    if (enable) {
        glfwGetWindowPos(m_Handle, &m_WindowedX, &m_WindowedY);
        glfwGetWindowSize(m_Handle, &m_WindowedWidth, &m_WindowedHeight);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_Handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_Handle, nullptr, m_WindowedX, m_WindowedY, m_WindowedWidth, m_WindowedHeight, 0);
    }
    m_Fullscreen = enable;
}

void GlfwWindow::ToggleFullscreen() {
    SetFullscreen(!m_Fullscreen);
}

void GlfwWindow::SetWindowMode(WindowMode mode) {
    if (!m_Handle || mode == m_WindowMode) return;

    if (m_WindowMode == WindowMode::Windowed) {
        glfwGetWindowPos(m_Handle, &m_WindowedX, &m_WindowedY);
        glfwGetWindowSize(m_Handle, &m_WindowedWidth, &m_WindowedHeight);
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

    switch (mode) {
        case WindowMode::Windowed:
            glfwSetWindowMonitor(m_Handle, nullptr, m_WindowedX, m_WindowedY, m_WindowedWidth, m_WindowedHeight, 0);
            break;
        case WindowMode::Fullscreen:
            glfwSetWindowMonitor(m_Handle, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
            break;
        case WindowMode::Borderless:
            glfwSetWindowMonitor(m_Handle, nullptr, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
            glfwSetWindowAttrib(m_Handle, GLFW_DECORATED, GLFW_FALSE);
            break;
    }

    if (mode != WindowMode::Borderless) {
        glfwSetWindowAttrib(m_Handle, GLFW_DECORATED, GLFW_TRUE);
    }

    m_WindowMode = mode;
    m_Fullscreen = (mode != WindowMode::Windowed);
}

void GlfwWindow::SetAspectRatio(int numerator, int denominator) {
    if (!m_Handle) return;
    m_AspectRatioNumerator = numerator;
    m_AspectRatioDenominator = denominator;
    if (numerator > 0 && denominator > 0) {
        glfwSetWindowAspectRatio(m_Handle, numerator, denominator);
    } else {
        glfwSetWindowAspectRatio(m_Handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
}

} // namespace SAGE
