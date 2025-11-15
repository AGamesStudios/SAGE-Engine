// ImGui Placeholder - Download from https://github.com/ocornut/imgui
// This file should be replaced with the actual imgui.h from ImGui repository

#pragma once

// TODO: Replace this placeholder with actual ImGui files
// 1. Download ImGui from: https://github.com/ocornut/imgui
// 2. Copy the following files to ThirdParty/imgui/:
//    - imgui.h
//    - imgui.cpp
//    - imgui_draw.cpp
//    - imgui_widgets.cpp
//    - imgui_tables.cpp
//    - imgui_demo.cpp
//    - imgui_internal.h
//    - imconfig.h
//    - imstb_rectpack.h
//    - imstb_textedit.h
//    - imstb_truetype.h
// 3. Copy backends/ folder with:
//    - backends/imgui_impl_glfw.h
//    - backends/imgui_impl_glfw.cpp
//    - backends/imgui_impl_opengl3.h
//    - backends/imgui_impl_opengl3.cpp

#ifndef IMGUI_PLACEHOLDER
#define IMGUI_PLACEHOLDER

// Minimal stub to allow compilation without actual ImGui
namespace ImGui {
    inline void NewFrame() {}
    inline void Render() {}
    inline void EndFrame() {}
    inline bool Begin(const char*, bool* = nullptr, int = 0) { return false; }
    inline void End() {}
    inline void Text(const char*, ...) {}
}

struct ImGuiContext {};
struct ImGuiIO {
    float DeltaTime = 0.016f;
    bool WantCaptureMouse = false;
    bool WantCaptureKeyboard = false;
};

#endif // IMGUI_PLACEHOLDER
