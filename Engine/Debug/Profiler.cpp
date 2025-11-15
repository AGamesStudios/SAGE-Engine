// Engine/Debug/Profiler.cpp

#include "Profiler.h"

#if __has_include("imgui.h")

#include "imgui.h"
#include <numeric>
#include <algorithm>

namespace SAGE {

Profiler::Profiler() {
    m_FrameTimes.fill(0.0f);
}

void Profiler::Update(float deltaTime) {
    // Update frame time history
    m_FrameTimes[m_FrameIndex] = deltaTime * 1000.0f;  // Convert to ms
    m_FrameIndex = (m_FrameIndex + 1) % HISTORY_SIZE;

    // Calculate FPS
    if (deltaTime > 0.0f) {
        m_FPS = 1.0f / deltaTime;
    }

    // Calculate average frame time
    float sum = std::accumulate(m_FrameTimes.begin(), m_FrameTimes.end(), 0.0f);
    m_AverageFrameTime = sum / static_cast<float>(HISTORY_SIZE);
}

void Profiler::Render() {
    if (!m_IsOpen) return;

    if (!ImGui::Begin("Profiler", &m_IsOpen)) {
        ImGui::End();
        return;
    }

    // FPS Display
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::Text("Frame Time: %.2f ms (avg: %.2f ms)", m_FrameTimes[m_FrameIndex], m_AverageFrameTime);
    ImGui::Separator();

    // Frame Time Graph
    float minFrameTime = *std::min_element(m_FrameTimes.begin(), m_FrameTimes.end());
    float maxFrameTime = *std::max_element(m_FrameTimes.begin(), m_FrameTimes.end());
    
    ImGui::PlotLines("Frame Time (ms)", 
                     m_FrameTimes.data(), 
                     HISTORY_SIZE, 
                     m_FrameIndex, 
                     nullptr, 
                     minFrameTime * 0.9f, 
                     maxFrameTime * 1.1f, 
                     ImVec2(0, 80));

    ImGui::Separator();

    // Performance Metrics
    ImGui::Text("Draw Calls: %d", m_DrawCalls);
    ImGui::Text("Entities: %d", m_EntityCount);
    
    // GPU Memory (convert bytes to MB)
    float gpuMemoryMB = static_cast<float>(m_GPUMemory) / (1024.0f * 1024.0f);
    ImGui::Text("GPU Memory: %.2f MB", gpuMemoryMB);

    ImGui::Separator();

    // Performance Targets
    float targetFrameTime = 16.67f;  // 60 FPS
    float currentFrameTime = m_FrameTimes[m_FrameIndex];
    
    ImVec4 color;
    if (currentFrameTime < targetFrameTime) {
        color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green - good
    } else if (currentFrameTime < targetFrameTime * 1.5f) {
        color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow - warning
    } else {
        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red - bad
    }

    ImGui::TextColored(color, "Performance: %s", 
                       currentFrameTime < targetFrameTime ? "GOOD" : 
                       currentFrameTime < targetFrameTime * 1.5f ? "WARNING" : "POOR");

    ImGui::End();
}

} // namespace SAGE

#else

// Stub when ImGui not available
namespace SAGE {

Profiler::Profiler() { m_FrameTimes.fill(0.0f); }
void Profiler::Update(float) {}
void Profiler::Render() {}

} // namespace SAGE

#endif
