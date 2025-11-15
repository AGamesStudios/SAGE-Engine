#pragma once

#include <string>

namespace SAGE {

    enum class WindowCursorMode {
        Normal = 0,   // Cursor visible and behaves normally
        Hidden = 1,   // Cursor invisible when over window
        Disabled = 2  // Cursor hidden and locked (for FPS games)
    };

    enum class WindowMode {
        Windowed = 0,
        Fullscreen = 1,
        WindowedFullscreen = 2  // Borderless fullscreen
    };

    struct WindowProps {
        std::string Title = "SAGE Window";
        unsigned int Width = 1280;
        unsigned int Height = 720;
        
        // Window behavior
        bool Resizable = true;
        bool Decorated = true;      // Window has title bar and border
        bool Floating = false;       // Always on top
        bool Maximized = false;
        bool Visible = true;
        bool Focused = true;        // Request focus on creation
        bool FocusOnShow = true;
        
        // Graphics
        bool VSync = true;
        bool TransparentFramebuffer = false;
        int Samples = 0;  // MSAA samples (0 = disabled)
        
        // Position (-1 = centered)
        int PosX = -1;
        int PosY = -1;
        
        // Monitor (0 = primary, -1 = current)
        int MonitorIndex = 0;
        
        WindowMode Mode = WindowMode::Windowed;
        WindowCursorMode Cursor = WindowCursorMode::Normal;
        
        // Limits (0 = no limit)
        unsigned int MinWidth = 320;
        unsigned int MinHeight = 240;
        unsigned int MaxWidth = 0;
        unsigned int MaxHeight = 0;
        
        // Aspect ratio (0 = no constraint)
        int AspectRatioNumerator = 0;
        int AspectRatioDenominator = 0;
    };

} // namespace SAGE
