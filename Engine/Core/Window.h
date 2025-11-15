#pragma once

#include "Core.h"
#include "WindowProps.h"
#include <functional>
#include <memory>
#include <string>

struct GLFWwindow;

namespace SAGE {

    class Event;
    class InputBridge;

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

        // Size and position
        unsigned int GetWidth() const { return m_Data.Width; }
        unsigned int GetHeight() const { return m_Data.Height; }
        unsigned int GetFramebufferWidth() const { return m_Data.FramebufferWidth; }
        unsigned int GetFramebufferHeight() const { return m_Data.FramebufferHeight; }
        float GetContentScaleX() const { return m_Data.ContentScaleX; }
        float GetContentScaleY() const { return m_Data.ContentScaleY; }
        
        void SetSize(unsigned int width, unsigned int height);
        void SetPosition(int x, int y);
        void GetPosition(int& x, int& y) const;
        void Center();
        
        // Window state
        GLFWwindow* GetNativeWindow() const { return m_Window; }
        bool ShouldClose() const;
        void Close(); // Request window to close
        bool IsInitialized() const { return m_Initialized; }
        bool IsMaximized() const;
        bool IsMinimized() const;
        bool IsFocused() const;
        bool IsVisible() const;
        
        void Maximize();
        void Minimize();
        void Restore();
        void Show();
        void Hide();
        void Focus();

        float GetDeltaTime() const { return m_DeltaTime; }
        void SetDeltaTime(float dt) { m_DeltaTime = dt; }

        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
        void SetTitle(const std::string& title);
        
        // VSync control
        void SetVSync(bool enabled);
        bool IsVSyncEnabled() const { return m_Data.VSyncEnabled; }
        
        // Fullscreen
        void SetFullscreen(bool enabled);
        void SetWindowMode(WindowMode mode);
        void SetFullscreenMonitor(int monitorIndex); // Select monitor for fullscreen
        bool IsFullscreen() const { return m_Data.Mode == WindowMode::Fullscreen; }
        WindowMode GetWindowMode() const { return m_Data.Mode; }
        
        // Cursor control
        void SetCursorMode(WindowCursorMode mode);
        WindowCursorMode GetCursorMode() const { return m_Data.CursorMode; }
        void GetCursorPos(double& x, double& y) const;
        void SetCursorPos(double x, double y);
        
        // Window attributes
        void SetOpacity(float opacity);
        float GetOpacity() const;
        void SetSizeLimits(unsigned int minW, unsigned int minH, unsigned int maxW, unsigned int maxH);
        void SetAspectRatio(int numerator, int denominator);
        
        // Icon
        void SetIcon(const std::string& path);
        void SetIcon(const unsigned char* pixels, int width, int height);
        
        // Clipboard
        std::string GetClipboardString() const;
        void SetClipboardString(const std::string& text);
        
        // InputBridge integration
        void SetInputBridge(InputBridge* bridge) { m_Data.InputBridge = bridge; }
        InputBridge* GetInputBridge() const { return static_cast<InputBridge*>(m_Data.InputBridge); }

        // Размеры окна с валидацией (legacy defaults)
        static constexpr unsigned int MinWidth = 320;
        static constexpr unsigned int MinHeight = 240;
        static constexpr unsigned int MaxWidth = 7680;  // 8K
        static constexpr unsigned int MaxHeight = 4320; // 8K

        // WindowData needs to be accessible for InputBridge callbacks
        struct WindowData {
            std::string Title;
            unsigned int Width = 0;
            unsigned int Height = 0;
            unsigned int FramebufferWidth = 0;
            unsigned int FramebufferHeight = 0;
            float ContentScaleX = 1.0f;
            float ContentScaleY = 1.0f;
            bool VSyncEnabled = true;
            WindowMode Mode = WindowMode::Windowed;
            WindowCursorMode CursorMode = SAGE::WindowCursorMode::Normal;
            EventCallbackFn EventCallback;
            void* InputBridge = nullptr;
            
            // State tracking
            bool Focused = true;
            bool Minimized = false;
            bool Maximized = false;
            double LastCursorX = 0.0;
            double LastCursorY = 0.0;
            
            // For fullscreen restore
            int WindowedPosX = 0;
            int WindowedPosY = 0;
            int WindowedWidth = 1280;
            int WindowedHeight = 720;
            
            // For multi-monitor fullscreen
            int FullscreenMonitorIndex = 0;
        };

    private:

        void Init(const WindowProps& props);
        void Shutdown();
        bool ValidateWindowSize(unsigned int width, unsigned int height) const;
        void ApplyWindowHints(const WindowProps& props);
        void SetupCallbacks();
        static void UpdateFramebufferData(GLFWwindow* window, WindowData& data);
        static void UpdateContentScale(GLFWwindow* window, WindowData& data);

        GLFWwindow* m_Window = nullptr;
        float m_DeltaTime = 0.0f;
        bool m_Initialized = false;

        WindowData m_Data;
    };

} // namespace SAGE
