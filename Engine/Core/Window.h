#pragma once

#include "Core.h"
#include "WindowProps.h"
#include <functional>
#include <memory>
#include <string>

struct GLFWwindow;

namespace SAGE {

    class Event;

    using EventCallbackFn = std::function<void(Event&)>;

    class Window {
    public:
        Window(const WindowProps& props);
        ~Window();

        // Запретить копирование (управляет GLFWwindow*)
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Разрешить перемещение
        Window(Window&& other) noexcept;
        Window& operator=(Window&& other) noexcept;

        void OnUpdate();
        void PollEvents();
        void SwapBuffers();

        unsigned int GetWidth() const { return m_Data.Width; }
        unsigned int GetHeight() const { return m_Data.Height; }
        GLFWwindow* GetNativeWindow() const { return m_Window; }

        bool ShouldClose() const;
        bool IsInitialized() const { return m_Initialized; }

        float GetDeltaTime() const { return m_DeltaTime; }
        void SetDeltaTime(float dt) { m_DeltaTime = dt; }

        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

        // Размеры окна с валидацией
        static constexpr unsigned int MinWidth = 320;
        static constexpr unsigned int MinHeight = 240;
        static constexpr unsigned int MaxWidth = 7680;  // 8K
        static constexpr unsigned int MaxHeight = 4320; // 8K

    private:
        void Init(const WindowProps& props);
        void Shutdown();
        bool ValidateWindowSize(unsigned int width, unsigned int height) const;

        GLFWwindow* m_Window = nullptr;
        float m_DeltaTime = 0.0f;
        bool m_Initialized = false;

        struct WindowData {
            std::string Title;
            unsigned int Width = 0;
            unsigned int Height = 0;
            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

} // namespace SAGE
