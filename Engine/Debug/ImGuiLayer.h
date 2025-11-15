// Engine/Debug/ImGuiLayer.h
#pragma once

#include <functional>
#include <vector>
#include <string>

struct GLFWwindow;

namespace SAGE {

/**
 * @brief ImGui integration layer for SAGE Engine
 * 
 * Handles ImGui initialization, frame management, and rendering.
 * Supports multiple debug windows with automatic layout.
 * 
 * Usage:
 * ```cpp
 * ImGuiLayer layer;
 * layer.Initialize(window);
 * 
 * // Game loop
 * while (!glfwWindowShouldClose(window)) {
 *     layer.BeginFrame();
 *     
 *     // Your ImGui windows here
 *     if (ImGui::Begin("My Window")) {
 *         ImGui::Text("Hello, World!");
 *     }
 *     ImGui::End();
 *     
 *     layer.EndFrame();
 *     layer.Render();
 * }
 * 
 * layer.Shutdown();
 * ```
 */
class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ~ImGuiLayer();

    /**
     * @brief Initialize ImGui with GLFW + OpenGL3 backend
     * @param window GLFW window handle
     * @param glslVersion GLSL version string (e.g., "#version 330")
     */
    void Initialize(GLFWwindow* window, const char* glslVersion = "#version 330");

    /**
     * @brief Shutdown ImGui and cleanup resources
     */
    void Shutdown();

    /**
     * @brief Begin new ImGui frame (call before any ImGui code)
     */
    void BeginFrame();

    /**
     * @brief End ImGui frame (call after all ImGui code)
     */
    void EndFrame();

    /**
     * @brief Render ImGui draw data (call after EndFrame)
     */
    void Render();

    /**
     * @brief Check if ImGui wants to capture mouse input
     * @return true if ImGui is handling mouse (don't process game input)
     */
    bool WantCaptureMouse() const;

    /**
     * @brief Check if ImGui wants to capture keyboard input
     * @return true if ImGui is handling keyboard (don't process game input)
     */
    bool WantCaptureKeyboard() const;

    /**
     * @brief Set dark theme colors (default)
     */
    void SetDarkTheme();

    /**
     * @brief Set light theme colors
     */
    void SetLightTheme();

    /**
     * @brief Set classic ImGui theme colors
     */
    void SetClassicTheme();

private:
    bool m_Initialized = false;
    GLFWwindow* m_Window = nullptr;
};

} // namespace SAGE
