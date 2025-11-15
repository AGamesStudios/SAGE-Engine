// Engine/Debug/Console.cpp

#include "Console.h"

#if __has_include("imgui.h")

#include "imgui.h"
#include <chrono>

namespace SAGE {

Console::Console() {}

void Console::AddLog(const std::string& message, LogLevel level) {
    auto now = std::chrono::high_resolution_clock::now();
    double timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
    
    m_Logs.push_back({ message, level, timestamp });
    
    // Limit log size
    if (m_Logs.size() > 1000) {
        m_Logs.erase(m_Logs.begin());
    }
}

void Console::Render() {
    if (!m_IsOpen) return;

    if (!ImGui::Begin("Console", &m_IsOpen)) {
        ImGui::End();
        return;
    }

    // Toolbar
    if (ImGui::Button("Clear")) {
        Clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &m_FilterInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &m_FilterWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &m_FilterError);

    ImGui::Separator();

    // Log display
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& logEntry : m_Logs) {
        // Filter
        if (logEntry.Level == LogLevel::Info && !m_FilterInfo) continue;
        if (logEntry.Level == LogLevel::Warning && !m_FilterWarning) continue;
        if (logEntry.Level == LogLevel::Error && !m_FilterError) continue;

        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        const char* prefix = "[INFO] ";
        switch (logEntry.Level) {
            case LogLevel::Info:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                prefix = "[INFO] ";
                break;
            case LogLevel::Warning:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                prefix = "[WARN] ";
                break;
            case LogLevel::Error:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                prefix = "[ERROR] ";
                break;
        }

        ImGui::TextColored(color, "%s%s", prefix, logEntry.Message.c_str());
    }

    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

void Console::Clear() {
    m_Logs.clear();
}

} // namespace SAGE

#else

// Stub
namespace SAGE {

Console::Console() {}
void Console::AddLog(const std::string&, LogLevel) {}
void Console::Render() {}
void Console::Clear() {}

} // namespace SAGE

#endif
