#pragma once

#include "../Core/ResourceManager.h"
#include <string>
#include <vector>

/**
 * @file MemoryProfiler.h
 * @brief Memory profiling window for ResourceManager
 */

namespace SAGE {

/**
 * @brief Memory profiler for VRAM and system RAM tracking
 * 
 * Displays:
 * - VRAM usage (textures, buffers)
 * - System RAM usage (game objects, components)
 * - Memory budgets and warnings
 * - Per-resource-type breakdown
 */
class MemoryProfiler {
public:
    MemoryProfiler() = default;
    
    /**
     * @brief Update memory statistics
     */
    void Update() {
        // Note: Integration with ResourceManager pending
        m_VRAMUsed = 0; // Will use: ResourceManager::GetVRAMUsage()
        m_RAMUsed = 0;  // Will use: SystemMemory::GetUsage()
    }
    
    /**
     * @brief Render ImGui window
     */
    void Render() {
#if __has_include("imgui.h")
        if (!ImGui::Begin("Memory Profiler")) {
            ImGui::End();
            return;
        }
        
        RenderVRAMSection();
        ImGui::Separator();
        RenderRAMSection();
        ImGui::Separator();
        RenderBudgetSection();
        
        ImGui::End();
#endif
    }
    
    /**
     * @brief Set VRAM budget (bytes)
     */
    void SetVRAMBudget(size_t bytes) { m_VRAMBudget = bytes; }
    
    /**
     * @brief Set RAM budget (bytes)
     */
    void SetRAMBudget(size_t bytes) { m_RAMBudget = bytes; }
    
private:
    void RenderVRAMSection() {
#if __has_include("imgui.h")
        ImGui::Text("VRAM Usage");
        
        float vramPercent = m_VRAMBudget > 0 
            ? (float)m_VRAMUsed / (float)m_VRAMBudget 
            : 0.0f;
        
        ImGui::ProgressBar(vramPercent, ImVec2(-1, 0), 
                          FormatBytes(m_VRAMUsed).c_str());
        
        ImGui::Text("Budget: %s / %s", 
                   FormatBytes(m_VRAMUsed).c_str(),
                   FormatBytes(m_VRAMBudget).c_str());
        
        // Warning if over budget
        if (vramPercent > 0.9f) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            ImGui::Text("WARNING: Approaching VRAM limit!");
            ImGui::PopStyleColor();
        }
        
        // Breakdown by resource type
        ImGui::Spacing();
        ImGui::Text("Breakdown:");
        
        // Note: Real data integration with ResourceManager pending
        RenderResourceBreakdown("Textures", 0, m_VRAMUsed);
        RenderResourceBreakdown("Buffers", 0, m_VRAMUsed);
        RenderResourceBreakdown("Shaders", 0, m_VRAMUsed);
#endif
    }
    
    void RenderRAMSection() {
#if __has_include("imgui.h")
        ImGui::Text("System RAM Usage");
        
        float ramPercent = m_RAMBudget > 0 
            ? (float)m_RAMUsed / (float)m_RAMBudget 
            : 0.0f;
        
        ImGui::ProgressBar(ramPercent, ImVec2(-1, 0), 
                          FormatBytes(m_RAMUsed).c_str());
        
        ImGui::Text("Budget: %s / %s", 
                   FormatBytes(m_RAMUsed).c_str(),
                   FormatBytes(m_RAMBudget).c_str());
        
        // Breakdown
        ImGui::Spacing();
        ImGui::Text("Breakdown:");
        
        // Note: Real data integration pending
        RenderResourceBreakdown("Entities", 0, m_RAMUsed);
        RenderResourceBreakdown("Components", 0, m_RAMUsed);
        RenderResourceBreakdown("Scripts", 0, m_RAMUsed);
#endif
    }
    
    void RenderBudgetSection() {
#if __has_include("imgui.h")
        ImGui::Text("Memory Budgets");
        
        // Allow adjusting budgets
        float vramGB = (float)m_VRAMBudget / (1024.0f * 1024.0f * 1024.0f);
        if (ImGui::SliderFloat("VRAM Budget (GB)", &vramGB, 0.5f, 8.0f)) {
            m_VRAMBudget = (size_t)(vramGB * 1024 * 1024 * 1024);
        }
        
        float ramGB = (float)m_RAMBudget / (1024.0f * 1024.0f * 1024.0f);
        if (ImGui::SliderFloat("RAM Budget (GB)", &ramGB, 1.0f, 16.0f)) {
            m_RAMBudget = (size_t)(ramGB * 1024 * 1024 * 1024);
        }
        
        // Stats
        ImGui::Spacing();
        ImGui::Text("Total Allocations: %d", m_TotalAllocations);
        ImGui::Text("Peak VRAM: %s", FormatBytes(m_PeakVRAM).c_str());
        ImGui::Text("Peak RAM: %s", FormatBytes(m_PeakRAM).c_str());
#endif
    }
    
    void RenderResourceBreakdown(const char* name, size_t used, size_t total) {
#if __has_include("imgui.h")
        float percent = total > 0 ? (float)used / (float)total : 0.0f;
        ImGui::Text("  %s: %s (%.1f%%)", name, FormatBytes(used).c_str(), percent * 100.0f);
#endif
    }
    
    std::string FormatBytes(size_t bytes) const {
        if (bytes < 1024) {
            return std::to_string(bytes) + " B";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else if (bytes < 1024 * 1024 * 1024) {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.2f GB", (float)bytes / (1024.0f * 1024.0f * 1024.0f));
            return std::string(buf);
        }
    }
    
private:
    size_t m_VRAMUsed = 0;
    size_t m_VRAMBudget = 2ULL * 1024 * 1024 * 1024; // 2GB default
    size_t m_RAMUsed = 0;
    size_t m_RAMBudget = 4ULL * 1024 * 1024 * 1024; // 4GB default
    
    size_t m_PeakVRAM = 0;
    size_t m_PeakRAM = 0;
    int m_TotalAllocations = 0;
};

} // namespace SAGE
