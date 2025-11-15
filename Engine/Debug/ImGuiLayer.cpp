// Engine/Debug/ImGuiLayer.cpp

#include "ImGuiLayer.h"

// Only compile if ImGui is available
#if __has_include("imgui.h")

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

namespace SAGE {

ImGuiLayer::~ImGuiLayer() {
    if (m_Initialized) {
        Shutdown();
    }
}

void ImGuiLayer::Initialize(GLFWwindow* window, const char* glslVersion) {
    if (m_Initialized) return;

    m_Window = window;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable multi-viewport

    // Setup style
    SetDarkTheme();

    // When viewports are enabled, tweak WindowRounding to avoid ImGui UI artifacts
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    m_Initialized = true;
}

void ImGuiLayer::Shutdown() {
    if (!m_Initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_Initialized = false;
    m_Window = nullptr;
}

void ImGuiLayer::BeginFrame() {
    if (!m_Initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::EndFrame() {
    if (!m_Initialized) return;

    ImGui::Render();
}

void ImGuiLayer::Render() {
    if (!m_Initialized) return;

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

bool ImGuiLayer::WantCaptureMouse() const {
    if (!m_Initialized) return false;
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiLayer::WantCaptureKeyboard() const {
    if (!m_Initialized) return false;
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiLayer::SetDarkTheme() {
    ImGui::StyleColorsDark();

    // Customize colors
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
}

void ImGuiLayer::SetLightTheme() {
    ImGui::StyleColorsLight();
}

void ImGuiLayer::SetClassicTheme() {
    ImGui::StyleColorsClassic();
}

} // namespace SAGE

#else

// Stub implementation when ImGui is not available
namespace SAGE {

ImGuiLayer::~ImGuiLayer() {}
void ImGuiLayer::Initialize(GLFWwindow*, const char*) {}
void ImGuiLayer::Shutdown() {}
void ImGuiLayer::BeginFrame() {}
void ImGuiLayer::EndFrame() {}
void ImGuiLayer::Render() {}
bool ImGuiLayer::WantCaptureMouse() const { return false; }
bool ImGuiLayer::WantCaptureKeyboard() const { return false; }
void ImGuiLayer::SetDarkTheme() {}
void ImGuiLayer::SetLightTheme() {}
void ImGuiLayer::SetClassicTheme() {}

} // namespace SAGE

#endif
