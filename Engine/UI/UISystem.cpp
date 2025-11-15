#include "UISystem.h"

#if __has_include("imgui.h")
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define IMGUI_AVAILABLE
#endif

namespace SAGE {

void UISystem::Init(void* window) {
    if (m_Initialized) return;
    
#ifdef IMGUI_AVAILABLE
    if (window && m_ImGuiAvailable) {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window);
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init("#version 410");
        
        m_Initialized = true;
    } else {
        SAGE_WARNING("[UISystem] ImGui not available or no window provided. UI system disabled.");
    }
#else
    (void)window; // Suppress unused warning
    SAGE_WARNING("[UISystem] ImGui headers not found. UI system disabled.");
#endif
}

void UISystem::Update(float deltaTime) {
    (void)deltaTime; // May be used for animations in the future
}

void UISystem::Render() {
    if (!m_Initialized) return;
    
#ifdef IMGUI_AVAILABLE
    // Call user callback for custom UI
    if (m_DrawCallback) {
        m_DrawCallback();
    }
#endif
}

void UISystem::Shutdown() {
    if (!m_Initialized) return;
    
#ifdef IMGUI_AVAILABLE
    if (m_ImGuiAvailable) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
#endif
    
    m_Initialized = false;
}

void UISystem::BeginFrame() {
    if (!m_Initialized) return;
    
#ifdef IMGUI_AVAILABLE
    if (m_ImGuiAvailable) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
#endif
}

void UISystem::EndFrame() {
    if (!m_Initialized) return;
    
#ifdef IMGUI_AVAILABLE
    if (m_ImGuiAvailable) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
#endif
}

bool UISystem::IsCapturingMouse() const {
    if (!m_Initialized) return false;
    
#ifdef IMGUI_AVAILABLE
    if (m_ImGuiAvailable) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }
#endif
    
    return false;
}

bool UISystem::IsCapturingKeyboard() const {
    if (!m_Initialized) return false;
    
#ifdef IMGUI_AVAILABLE
    if (m_ImGuiAvailable) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }
#endif
    
    return false;
}

} // namespace SAGE
