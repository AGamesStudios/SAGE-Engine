#include "Window.h"
#include "Events/WindowEvent.h"
#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SAGE {

    static bool s_GLFWInitialized = false;

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
        if (width < MinWidth || width > MaxWidth) {
            SAGE_ERROR("Window width {} out of range [{}, {}]", width, MinWidth, MaxWidth);
            return false;
        }
        if (height < MinHeight || height > MaxHeight) {
            SAGE_ERROR("Window height {} out of range [{}, {}]", height, MinHeight, MaxHeight);
            return false;
        }
        return true;
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
        
        m_Data.Title = props.Title;
        
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
        
        // Настройка OpenGL контекста
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif
        
        m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, 
                                   m_Data.Title.c_str(), nullptr, nullptr);
        
        if (!m_Window) {
            SAGE_ERROR("Failed to create GLFW window!");
            glfwTerminate();
            s_GLFWInitialized = false;
            return;
        }
        
        glfwMakeContextCurrent(m_Window);
        
        // Инициализация GLAD
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!status) {
            SAGE_ERROR("Failed to initialize GLAD!");
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            glfwTerminate();
            s_GLFWInitialized = false;
            return;
        }
        
        SAGE_INFO("OpenGL context created successfully");
        SAGE_INFO("OpenGL Info:");
        SAGE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
        SAGE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
        SAGE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));
        
        // Установка VSync
        glfwSwapInterval(1);

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Установка начального viewport
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Установка колбэков
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->EventCallback) {
                return;
            }
            WindowCloseEvent event;
            data->EventCallback(event);
        });

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data) {
                return;
            }
            
            // Валидация размеров
            width = std::max((int)MinWidth, std::min(width, (int)MaxWidth));
            height = std::max((int)MinHeight, std::min(height, (int)MaxHeight));
            
            data->Width = static_cast<unsigned int>(width);
            data->Height = static_cast<unsigned int>(height);
            
            if (data->EventCallback) {
                WindowResizeEvent event(data->Width, data->Height);
                data->EventCallback(event);
            }
        });

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data) {
                // Валидация размеров framebuffer
                width = std::max(1, width);
                height = std::max(1, height);
                
                data->Width = static_cast<unsigned int>(width);
                data->Height = static_cast<unsigned int>(height);
                glViewport(0, 0, width, height);
                
                if (data->EventCallback) {
                    WindowResizeEvent event(data->Width, data->Height);
                    data->EventCallback(event);
                }
            }
        });
        
        m_Initialized = true;
        SAGE_INFO("Window initialized successfully");
    }

    void Window::Shutdown() {
        if (!m_Initialized) {
            return;
        }
        
        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
        
        // Не терминируем GLFW здесь, так как могут быть другие окна
        m_Initialized = false;
        SAGE_INFO("Window shut down");
    }

    void Window::OnUpdate() {
        if (!m_Window) {
            SAGE_WARNING("Window::OnUpdate() called on uninitialized window!");
            return;
        }
        glfwPollEvents();
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

    bool Window::ShouldClose() const {
        if (!m_Window) {
            return true;
        }
        return glfwWindowShouldClose(m_Window);
    }

}
