#pragma once

#include "Core/Core.h"
#include <functional>
#include <string>

namespace SAGE {

/// @brief Система пользовательского интерфейса
/// @note Базовая реализация с поддержкой ImGui (если доступен)
class UISystem {
public:
    UISystem() = default;
    ~UISystem() = default;

    /// @brief Инициализация UI системы
    /// @param window GLFW window для ImGui backend (опционально)
    void Init(void* window = nullptr);

    /// @brief Обновление UI (вызывается в начале кадра)
    /// @param deltaTime Время с последнего кадра
    void Update(float deltaTime);

    /// @brief Рендеринг UI (вызывается после рендера сцены)
    void Render();

    /// @brief Shutdown UI системы
    void Shutdown();
    
    /// @brief Проверка инициализации
    bool IsInitialized() const { return m_Initialized; }
    
    /// @brief Установить callback для пользовательского UI
    /// @param callback Функция которая вызывается каждый кадр для рисования UI
    using UIDrawCallback = std::function<void()>;
    void SetDrawCallback(UIDrawCallback callback) { m_DrawCallback = callback; }
    
    /// @brief Начать новый UI кадр
    void BeginFrame();
    
    /// @brief Завершить UI кадр
    void EndFrame();
    
    /// @brief Проверить захватывает ли UI мышь
    bool IsCapturingMouse() const;
    
    /// @brief Проверить захватывает ли UI клавиатуру
    bool IsCapturingKeyboard() const;

private:
    bool m_Initialized = false;
    UIDrawCallback m_DrawCallback = nullptr;
    
    #if __has_include("imgui.h")
    bool m_ImGuiAvailable = true;
    #else
    bool m_ImGuiAvailable = false;
    #endif
};

} // namespace SAGE
