#pragma once

namespace SAGE {

/**
 * @brief Абстрактный интерфейс окна для независимости от GLFW
 * @note Позволяет портировать движок на SDL, Win32, и другие платформы
 */
class IWindow {
public:
    virtual ~IWindow() noexcept = default;
    
    /**
     * @brief Получить нативный handle окна
     * @return void* указатель на GLFWwindow, SDL_Window, HWND и т.д.
     */
    virtual void* GetNativeHandle() const = 0;
    
    /**
     * @brief Получить размеры окна
     */
    virtual void GetSize(int& width, int& height) const = 0;
    
    /**
     * @brief Проверить, должно ли окно закрыться
     */
    virtual bool ShouldClose() const = 0;
    
    /**
     * @brief Получить тип оконной системы
     */
    enum class Type {
        GLFW,
        SDL,
        Win32,
        X11,
        Wayland
    };
    
    virtual Type GetWindowType() const = 0;
};

/**
 * @brief GLFW реализация IWindow (wrapper)
 */
class GLFWWindowAdapter : public IWindow {
public:
    explicit GLFWWindowAdapter(void* glfwWindow) 
        : m_GLFWWindow(glfwWindow) {}
    
    void* GetNativeHandle() const override { 
        return m_GLFWWindow; 
    }
    
    void GetSize(int& width, int& height) const override;
    bool ShouldClose() const override;
    
    Type GetWindowType() const override { 
        return Type::GLFW; 
    }
    
private:
    void* m_GLFWWindow;
};

} // namespace SAGE
