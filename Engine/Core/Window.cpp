#include "Window.h"
#include "Events/WindowEvent.h"
#include "Logger.h"
#include "Graphics/API/Renderer.h"
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

namespace SAGE {

    static bool s_GLFWInitialized = false;
    static int s_WindowCount = 0;

    namespace {
        constexpr float kMinContentScale = 0.01f;
    }

    void Window::UpdateFramebufferData(GLFWwindow* window, WindowData& data) {
        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        fbWidth = std::max(1, fbWidth);
        fbHeight = std::max(1, fbHeight);
        data.FramebufferWidth = static_cast<unsigned int>(fbWidth);
        data.FramebufferHeight = static_cast<unsigned int>(fbHeight);
    }

    void Window::UpdateContentScale(GLFWwindow* window, WindowData& data) {
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        glfwGetWindowContentScale(window, &scaleX, &scaleY);
        data.ContentScaleX = std::max(scaleX, kMinContentScale);
        data.ContentScaleY = std::max(scaleY, kMinContentScale);
    }

    static void GLFWErrorCallback(int error, const char* description) {
        SAGE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window::Window(const WindowProps& props) {
        Init(props);
    }

    Window::~Window() {
        Shutdown();
    }

    Window::Window(Window&& other) noexcept
        : m_Window(other.m_Window)
        , m_DeltaTime(other.m_DeltaTime)
        , m_Data(std::move(other.m_Data))
        , m_Initialized(other.m_Initialized) {
        other.m_Window = nullptr;
        other.m_Initialized = false;
    }

    Window& Window::operator=(Window&& other) noexcept {
        if (this != &other) {
            Shutdown(); // Освободить текущие ресурсы
            
            m_Window = other.m_Window;
            m_DeltaTime = other.m_DeltaTime;
            m_Data = std::move(other.m_Data);
            m_Initialized = other.m_Initialized;
            
            other.m_Window = nullptr;
            other.m_Initialized = false;
        }
        return *this;
    }

    bool Window::ValidateWindowSize(unsigned int width, unsigned int height) const {
        // Use custom limits or defaults
        unsigned int minW = (m_Data.WindowedWidth > 0) ? MinWidth : 1;
        unsigned int minH = (m_Data.WindowedHeight > 0) ? MinHeight : 1;
        unsigned int maxW = MaxWidth;
        unsigned int maxH = MaxHeight;
        
        if (width < minW || (maxW > 0 && width > maxW)) {
            SAGE_ERROR("Window width {} out of range [{}, {}]", width, minW, maxW);
            return false;
        }
        if (height < minH || (maxH > 0 && height > maxH)) {
            SAGE_ERROR("Window height {} out of range [{}, {}]", height, minH, maxH);
            return false;
        }
        return true;
    }

    void Window::ApplyWindowHints(const WindowProps& props) {
        // Reset to GLFW defaults to avoid inheriting stale hints (e.g., undecorated windows)
        glfwDefaultWindowHints();

        // OpenGL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif
        
        // Window properties
        glfwWindowHint(GLFW_RESIZABLE, props.Resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, props.Decorated ? GLFW_TRUE : GLFW_FALSE);
        SAGE_INFO("Window: Setting GLFW_DECORATED = {}", props.Decorated ? "TRUE" : "FALSE");
        glfwWindowHint(GLFW_FLOATING, props.Floating ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, props.Maximized ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, props.Visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, props.Focused ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, props.FocusOnShow ? GLFW_TRUE : GLFW_FALSE);
        
        // Framebuffer
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, props.TransparentFramebuffer ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES, props.Samples);
    }

    void Window::Init(const WindowProps& props) {
        // Защита от повторной инициализации
        if (m_Initialized) {
            SAGE_WARNING("Window::Init() called on already initialized window!");
            return;
        }

        // Валидация размеров
        if (!ValidateWindowSize(props.Width, props.Height)) {
            SAGE_ERROR("Invalid window dimensions, using defaults");
            m_Data.Width = 1280;
            m_Data.Height = 720;
        } else {
            m_Data.Width = props.Width;
            m_Data.Height = props.Height;
        }

        m_Data.FramebufferWidth = m_Data.Width;
        m_Data.FramebufferHeight = m_Data.Height;
        m_Data.ContentScaleX = 1.0f;
        m_Data.ContentScaleY = 1.0f;
        m_Data.Title = props.Title;
        m_Data.VSyncEnabled = props.VSync;
        m_Data.Mode = props.Mode;
        m_Data.CursorMode = props.Cursor;
        m_Data.WindowedWidth = props.Width;
        m_Data.WindowedHeight = props.Height;
        
        SAGE_INFO("Creating window {0} ({1}, {2})", props.Title, m_Data.Width, m_Data.Height);
        
        // Инициализация GLFW
        if (!s_GLFWInitialized) {
            int success = glfwInit();
            if (!success) {
                SAGE_ERROR("Failed to initialize GLFW!");
                return;
            }
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
            SAGE_INFO("GLFW initialized successfully");
        }
        
        // Применить window hints
        ApplyWindowHints(props);
        
        // Получить монитор для fullscreen
        GLFWmonitor* monitor = nullptr;
        if (props.Mode == WindowMode::Fullscreen || props.Mode == WindowMode::WindowedFullscreen) {
            int monitorCount;
            GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
            if (monitors && monitorCount > 0) {
                int targetMonitor = std::max(0, std::min(props.MonitorIndex, monitorCount - 1));
                monitor = monitors[targetMonitor];
                
                if (props.Mode == WindowMode::Fullscreen) {
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    m_Data.Width = mode->width;
                    m_Data.Height = mode->height;
                } else {
                    // Windowed fullscreen
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    m_Data.Width = mode->width;
                    m_Data.Height = mode->height;
                    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
                    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
                }
            }
        }
        
        // Re-enforce decoration hint after any fullscreen hints (critical for windowed mode)
        if (props.Mode == WindowMode::Windowed) {
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            SAGE_INFO("Window: Re-enforcing GLFW_DECORATED = TRUE for windowed mode");
        }
        
        m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, 
                                   m_Data.Title.c_str(), 
                                   (props.Mode == WindowMode::Fullscreen) ? monitor : nullptr,
                                   nullptr);
        
        if (!m_Window) {
            SAGE_ERROR("Failed to create GLFW window!");
            if (s_WindowCount == 0) {
                glfwTerminate();
                s_GLFWInitialized = false;
            }
            return;
        }
        
        // Verify decoration attribute after window creation
        int decorated = glfwGetWindowAttrib(m_Window, GLFW_DECORATED);
        SAGE_INFO("Window created: GLFW_DECORATED attribute = {}", decorated ? "TRUE" : "FALSE");
        
        s_WindowCount++;
        glfwMakeContextCurrent(m_Window);
        
        // Инициализация GLAD
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!status) {
            SAGE_ERROR("Failed to initialize GLAD!");
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            s_WindowCount--;
            if (s_WindowCount == 0) {
                glfwTerminate();
                s_GLFWInitialized = false;
            }
            return;
        }
        
        SAGE_INFO("OpenGL context created successfully");
        SAGE_INFO("OpenGL Info:");
        SAGE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
        SAGE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
        SAGE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));
        
        // Check OpenGL version
        GLint majorVersion = 0, minorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
        
        const int requiredMajor = 3;
        const int requiredMinor = 3;
        
        if (majorVersion < requiredMajor || (majorVersion == requiredMajor && minorVersion < requiredMinor)) {
            SAGE_ERROR("OpenGL version {}.{} is too old! Minimum required: {}.{}", 
                majorVersion, minorVersion, requiredMajor, requiredMinor);
            SAGE_ERROR("Please update your graphics drivers!");
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            s_WindowCount--;
            if (s_WindowCount == 0) {
                glfwTerminate();
                s_GLFWInitialized = false;
            }
            return;
        }
        
        SAGE_INFO("OpenGL version check passed: {}.{}", majorVersion, minorVersion);
        
        // VSync
        glfwSwapInterval(props.VSync ? 1 : 0);

        // Позиционирование окна
        if (props.Mode == WindowMode::Windowed) {
            if (props.PosX == -1 || props.PosY == -1) {
                Center();
            } else {
                glfwSetWindowPos(m_Window, props.PosX, props.PosY);
            }
            
            // Сохранить позицию для восстановления из fullscreen
            glfwGetWindowPos(m_Window, &m_Data.WindowedPosX, &m_Data.WindowedPosY);
        }

        // Ограничения размеров окна
        if (props.Mode == WindowMode::Windowed) {
            unsigned int minW = props.MinWidth;
            unsigned int minH = props.MinHeight;
            unsigned int maxW = (props.MaxWidth > 0) ? props.MaxWidth : GLFW_DONT_CARE;
            unsigned int maxH = (props.MaxHeight > 0) ? props.MaxHeight : GLFW_DONT_CARE;
            glfwSetWindowSizeLimits(m_Window, minW, minH, maxW, maxH);
        }
        
        // Aspect ratio
        if (props.AspectRatioNumerator > 0 && props.AspectRatioDenominator > 0) {
            glfwSetWindowAspectRatio(m_Window, props.AspectRatioNumerator, props.AspectRatioDenominator);
        }

        // Cursor mode
        SetCursorMode(props.Cursor);

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Установка начального viewport
        UpdateFramebufferData(m_Window, m_Data);
        UpdateContentScale(m_Window, m_Data);
        glViewport(0, 0,
            static_cast<GLsizei>(m_Data.FramebufferWidth),
            static_cast<GLsizei>(m_Data.FramebufferHeight));

        // Установка колбэков
        SetupCallbacks();
        
        m_Initialized = true;
        SAGE_INFO("Window initialized successfully");
    }
    void Window::SetupCallbacks() {
        // Window close
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) {
                return;
            }
            WindowCloseEvent event;
            data->EventCallback(event);
        });

        // Framebuffer resize
        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data) {
                return;
            }
            
            // Валидация размеров framebuffer
            width = std::max(1, width);
            height = std::max(1, height);
            
            data->FramebufferWidth = static_cast<unsigned int>(width);
            data->FramebufferHeight = static_cast<unsigned int>(height);
            
            // Обновить window size пропорционально
            int wWidth, wHeight;
            glfwGetWindowSize(window, &wWidth, &wHeight);
            data->Width = static_cast<unsigned int>(wWidth);
            data->Height = static_cast<unsigned int>(wHeight);
            
            glViewport(0, 0, width, height);
            
            if (Renderer::IsInitialized()) {
                Renderer::OnWindowResize(width, height);
            }
            
            if (data->EventCallback) {
                WindowResizeEvent event(data->FramebufferWidth, data->FramebufferHeight);
                data->EventCallback(event);
            }
        });

        // Window position
        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xpos, int ypos) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data) return;
            
            // Update windowed position if not fullscreen
            if (data->Mode == WindowMode::Windowed) {
                data->WindowedPosX = xpos;
                data->WindowedPosY = ypos;
            }
            
            if (data->EventCallback) {
                WindowMoveEvent event(xpos, ypos);
                data->EventCallback(event);
            }
        });

        // Window focus
        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) return;
            
            data->Focused = (focused == GLFW_TRUE);
            
            WindowFocusEvent event(data->Focused);
            data->EventCallback(event);
        });

        // Window iconify (minimize)
        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) return;
            
            data->Minimized = (iconified == GLFW_TRUE);
            
            WindowMinimizeEvent event(data->Minimized);
            data->EventCallback(event);
        });

        // Window maximize
        glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* window, int maximized) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) return;
            
            data->Maximized = (maximized == GLFW_TRUE);
            
            WindowMaximizeEvent event(data->Maximized);
            data->EventCallback(event);
        });

        // Content scale
        glfwSetWindowContentScaleCallback(m_Window, [](GLFWwindow* window, float xscale, float yscale) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data) {
                return;
            }

            data->ContentScaleX = std::max(xscale, kMinContentScale);
            data->ContentScaleY = std::max(yscale, kMinContentScale);
            
            if (Renderer::IsInitialized()) {
                Renderer::SetContentScale(data->ContentScaleX, data->ContentScaleY);
            }
            
            if (data->EventCallback) {
                WindowContentScaleEvent event(data->ContentScaleX, data->ContentScaleY);
                data->EventCallback(event);
            }
        });

        // Drop files
        glfwSetDropCallback(m_Window, [](GLFWwindow* window, int count, const char** paths) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) return;
            
            std::vector<std::string> filePaths;
            filePaths.reserve(count);
            for (int i = 0; i < count; i++) {
                filePaths.emplace_back(paths[i]);
                SAGE_INFO("File dropped: {}", paths[i]);
            }
            
            WindowFileDropEvent event(filePaths);
            data->EventCallback(event);
        });
    }

    void Window::Shutdown() {
        if (!m_Initialized) {
            return;
        }
        
        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            s_WindowCount--;
        }
        
        // Терминируем GLFW только когда все окна закрыты
        if (s_WindowCount == 0 && s_GLFWInitialized) {
            glfwTerminate();
            s_GLFWInitialized = false;
            SAGE_INFO("GLFW terminated (all windows closed)");
        }
        
        m_Initialized = false;
        SAGE_INFO("Window shut down");
    }

    void Window::OnUpdate() {
        if (!m_Window) {
            SAGE_WARNING("Window::OnUpdate() called on uninitialized window!");
            return;
        }
        PollEvents();  // Use PollEvents() instead of duplicating glfwPollEvents()
        glfwSwapBuffers(m_Window);
    }

    void Window::PollEvents() {
        if (!m_Window) {
            return;
        }
        glfwPollEvents();
    }

    void Window::SwapBuffers() {
        if (!m_Window) {
            return;
        }
        glfwSwapBuffers(m_Window);
    }

    void Window::SetTitle(const std::string& title) {
        if (title.empty()) {
            SAGE_WARNING("Window::SetTitle called with empty string, ignoring.");
            return;
        }
        m_Data.Title = title;
        if (m_Window) {
            glfwSetWindowTitle(m_Window, m_Data.Title.c_str());
        }
    }

    void Window::SetVSync(bool enabled) {
        if (m_Window) {
            glfwSwapInterval(enabled ? 1 : 0);
            m_Data.VSyncEnabled = enabled;
        }
    }

    bool Window::ShouldClose() const {
        if (!m_Window) {
            return true;
        }
        return glfwWindowShouldClose(m_Window);
    }

    void Window::Close() {
        if (m_Window) {
            glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
        }
    }

    // ========== New Methods ==========

    void Window::SetSize(unsigned int width, unsigned int height) {
        if (!m_Window || m_Data.Mode != WindowMode::Windowed) return;
        
        if (ValidateWindowSize(width, height)) {
            glfwSetWindowSize(m_Window, width, height);
            m_Data.Width = width;
            m_Data.Height = height;
            m_Data.WindowedWidth = width;
            m_Data.WindowedHeight = height;
        }
    }

    void Window::SetPosition(int x, int y) {
        if (!m_Window || m_Data.Mode != WindowMode::Windowed) return;
        
        glfwSetWindowPos(m_Window, x, y);
        m_Data.WindowedPosX = x;
        m_Data.WindowedPosY = y;
    }

    void Window::GetPosition(int& x, int& y) const {
        if (!m_Window) {
            x = 0;
            y = 0;
            return;
        }
        glfwGetWindowPos(m_Window, &x, &y);
    }

    void Window::Center() {
        if (!m_Window) return;
        
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) return;
        
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode) return;
        
        int x = (mode->width - static_cast<int>(m_Data.Width)) / 2;
        int y = (mode->height - static_cast<int>(m_Data.Height)) / 2;
        
        SetPosition(x, y);
    }

    bool Window::IsMaximized() const {
        if (!m_Window) return false;
        return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED) == GLFW_TRUE;
    }

    bool Window::IsMinimized() const {
        if (!m_Window) return false;
        return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    bool Window::IsFocused() const {
        if (!m_Window) return false;
        return glfwGetWindowAttrib(m_Window, GLFW_FOCUSED) == GLFW_TRUE;
    }

    bool Window::IsVisible() const {
        if (!m_Window) return false;
        return glfwGetWindowAttrib(m_Window, GLFW_VISIBLE) == GLFW_TRUE;
    }

    void Window::Maximize() {
        if (m_Window && m_Data.Mode == WindowMode::Windowed) {
            glfwMaximizeWindow(m_Window);
        }
    }

    void Window::Minimize() {
        if (m_Window) {
            glfwIconifyWindow(m_Window);
        }
    }

    void Window::Restore() {
        if (m_Window) {
            glfwRestoreWindow(m_Window);
        }
    }

    void Window::Show() {
        if (m_Window) {
            glfwShowWindow(m_Window);
        }
    }

    void Window::Hide() {
        if (m_Window) {
            glfwHideWindow(m_Window);
        }
    }

    void Window::Focus() {
        if (m_Window) {
            glfwFocusWindow(m_Window);
        }
    }

    void Window::SetFullscreen(bool enabled) {
        if (!m_Window) return;
        
        if (enabled) {
            SetWindowMode(WindowMode::Fullscreen);
        } else {
            SetWindowMode(WindowMode::Windowed);
        }
    }

    void Window::SetFullscreenMonitor(int monitorIndex) {
        m_Data.FullscreenMonitorIndex = monitorIndex;
        // If already in fullscreen, switch to new monitor
        if (m_Data.Mode != WindowMode::Windowed) {
            WindowMode currentMode = m_Data.Mode;
            SetWindowMode(WindowMode::Windowed);
            SetWindowMode(currentMode);
        }
    }

    void Window::SetWindowMode(WindowMode mode) {
        if (!m_Window || m_Data.Mode == mode) return;
        
        if (mode == WindowMode::Windowed) {
            // Restore windowed mode
            glfwSetWindowMonitor(m_Window, nullptr,
                m_Data.WindowedPosX, m_Data.WindowedPosY,
                m_Data.WindowedWidth, m_Data.WindowedHeight,
                GLFW_DONT_CARE);
            
            m_Data.Width = m_Data.WindowedWidth;
            m_Data.Height = m_Data.WindowedHeight;
        } else {
            // Save current windowed position/size
            if (m_Data.Mode == WindowMode::Windowed) {
                glfwGetWindowPos(m_Window, &m_Data.WindowedPosX, &m_Data.WindowedPosY);
                glfwGetWindowSize(m_Window, &m_Data.WindowedWidth, &m_Data.WindowedHeight);
            }
            
            // Get monitor for fullscreen
            GLFWmonitor* monitor = nullptr;
            if (m_Data.FullscreenMonitorIndex >= 0) {
                int monitorCount;
                GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
                if (m_Data.FullscreenMonitorIndex < monitorCount) {
                    monitor = monitors[m_Data.FullscreenMonitorIndex];
                }
            }
            if (!monitor) {
                monitor = glfwGetPrimaryMonitor();
            }
            
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
            
            if (mode == WindowMode::Fullscreen) {
                // True fullscreen with mode switch
                glfwSetWindowMonitor(m_Window, monitor, 0, 0,
                    videoMode->width, videoMode->height, videoMode->refreshRate);
            } else { // WindowedFullscreen
                // Borderless fullscreen without mode switch
                glfwSetWindowMonitor(m_Window, monitor, 0, 0,
                    videoMode->width, videoMode->height, GLFW_DONT_CARE);
            }
            
            m_Data.Width = videoMode->width;
            m_Data.Height = videoMode->height;
        }
        
        m_Data.Mode = mode;
        SAGE_INFO("Window mode changed to: {}", static_cast<int>(mode));
    }

    void Window::SetCursorMode(WindowCursorMode mode) {
        if (!m_Window) return;
        
        int glfwMode = GLFW_CURSOR_NORMAL;
        switch (mode) {
            case WindowCursorMode::Normal:
                glfwMode = GLFW_CURSOR_NORMAL;
                break;
            case WindowCursorMode::Hidden:
                glfwMode = GLFW_CURSOR_HIDDEN;
                break;
            case WindowCursorMode::Disabled:
                glfwMode = GLFW_CURSOR_DISABLED;
                break;
        }
        
        glfwSetInputMode(m_Window, GLFW_CURSOR, glfwMode);
        m_Data.CursorMode = mode;
    }

    void Window::GetCursorPos(double& x, double& y) const {
        if (!m_Window) {
            x = 0.0;
            y = 0.0;
            return;
        }
        glfwGetCursorPos(m_Window, &x, &y);
    }

    void Window::SetCursorPos(double x, double y) {
        if (m_Window) {
            glfwSetCursorPos(m_Window, x, y);
        }
    }

    void Window::SetOpacity(float opacity) {
        if (m_Window) {
            opacity = std::max(0.0f, std::min(1.0f, opacity));
            glfwSetWindowOpacity(m_Window, opacity);
        }
    }

    float Window::GetOpacity() const {
        if (!m_Window) return 1.0f;
        return glfwGetWindowOpacity(m_Window);
    }

    void Window::SetSizeLimits(unsigned int minW, unsigned int minH, unsigned int maxW, unsigned int maxH) {
        if (!m_Window) return;
        
        int glfwMinW = (minW > 0) ? minW : GLFW_DONT_CARE;
        int glfwMinH = (minH > 0) ? minH : GLFW_DONT_CARE;
        int glfwMaxW = (maxW > 0) ? maxW : GLFW_DONT_CARE;
        int glfwMaxH = (maxH > 0) ? maxH : GLFW_DONT_CARE;
        
        glfwSetWindowSizeLimits(m_Window, glfwMinW, glfwMinH, glfwMaxW, glfwMaxH);
    }

    void Window::SetAspectRatio(int numerator, int denominator) {
        if (m_Window) {
            if (numerator == 0 || denominator == 0) {
                glfwSetWindowAspectRatio(m_Window, GLFW_DONT_CARE, GLFW_DONT_CARE);
            } else {
                glfwSetWindowAspectRatio(m_Window, numerator, denominator);
            }
        }
    }

    void Window::SetIcon(const std::string& path) {
        if (!m_Window) return;
        
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
        
        if (data) {
            SetIcon(data, width, height);
            stbi_image_free(data);
        } else {
            SAGE_ERROR("Failed to load window icon: {}", path);
        }
    }

    void Window::SetIcon(const unsigned char* pixels, int width, int height) {
        if (!m_Window || !pixels) return;
        
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = const_cast<unsigned char*>(pixels);
        
        glfwSetWindowIcon(m_Window, 1, &icon);
    }

    std::string Window::GetClipboardString() const {
        if (!m_Window) return "";
        
        const char* text = glfwGetClipboardString(m_Window);
        return text ? std::string(text) : "";
    }

    void Window::SetClipboardString(const std::string& text) {
        if (m_Window) {
            glfwSetClipboardString(m_Window, text.c_str());
        }
    }

}
