#pragma once

#include "ImGuiLayer.h"
#include "EntityInspector.h"
#include "Profiler.h"
#include "Console.h"
#include "SceneHierarchy.h"
#include <functional>
#include <unordered_map>

/**
 * @file DebugShortcuts.h
 * @brief Keyboard shortcuts for debug windows (F1-F5)
 */

namespace SAGE {

/**
 * @brief Manages keyboard shortcuts for debug windows
 * 
 * F1 = Entity Inspector
 * F2 = Profiler
 * F3 = Console
 * F4 = Scene Hierarchy
 * F5 = Memory Profiler
 * F6 = Toggle All Windows
 */
class DebugShortcuts {
public:
    DebugShortcuts() = default;
    
    /**
     * @brief Register debug window toggles
     */
    void RegisterWindow(int keyCode, const std::string& name, std::function<void(bool)> toggleCallback) {
        m_Shortcuts[keyCode] = {name, toggleCallback, false};
    }
    
    /**
     * @brief Process keyboard input
     * @param keyCode GLFW key code
     * @param pressed true if key was just pressed
     */
    void ProcessInput(int keyCode, bool pressed) {
        if (!pressed) return;
        
        auto it = m_Shortcuts.find(keyCode);
        if (it != m_Shortcuts.end()) {
            auto& shortcut = it->second;
            shortcut.visible = !shortcut.visible;
            shortcut.callback(shortcut.visible);
        }
    }
    
    /**
     * @brief Check if window is visible
     */
    bool IsWindowVisible(int keyCode) const {
        auto it = m_Shortcuts.find(keyCode);
        return (it != m_Shortcuts.end()) ? it->second.visible : false;
    }
    
    /**
     * @brief Set window visibility
     */
    void SetWindowVisible(int keyCode, bool visible) {
        auto it = m_Shortcuts.find(keyCode);
        if (it != m_Shortcuts.end()) {
            it->second.visible = visible;
            it->second.callback(visible);
        }
    }
    
    /**
     * @brief Toggle all windows
     */
    void ToggleAll() {
        m_AllVisible = !m_AllVisible;
        for (auto& [key, shortcut] : m_Shortcuts) {
            shortcut.visible = m_AllVisible;
            shortcut.callback(m_AllVisible);
        }
    }
    
    /**
     * @brief Render shortcuts help overlay
     */
    void RenderHelp() const {
#if __has_include("imgui.h")
        if (!ImGui::BeginPopupModal("Debug Shortcuts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::Separator();
        
        for (const auto& [key, shortcut] : m_Shortcuts) {
            const char* keyName = GetKeyName(key);
            ImGui::Text("%s - %s %s", keyName, shortcut.name.c_str(),
                       shortcut.visible ? "[ON]" : "[OFF]");
        }
        
        ImGui::Separator();
        ImGui::Text("F12 - Toggle This Help");
        
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
#endif
    }
    
    /**
     * @brief Show help popup
     */
    void ShowHelp() {
#if __has_include("imgui.h")
        ImGui::OpenPopup("Debug Shortcuts");
#endif
    }
    
private:
    struct Shortcut {
        std::string name;
        std::function<void(bool)> callback;
        bool visible;
    };
    
    std::unordered_map<int, Shortcut> m_Shortcuts;
    bool m_AllVisible = false;
    
    const char* GetKeyName(int keyCode) const {
        // GLFW key codes
        switch (keyCode) {
            case 290: return "F1";
            case 291: return "F2";
            case 292: return "F3";
            case 293: return "F4";
            case 294: return "F5";
            case 295: return "F6";
            case 301: return "F12";
            default: return "???";
        }
    }
};

} // namespace SAGE
