// Engine/Debug/Profiler.h
#pragma once

#include <array>
#include <string>

namespace SAGE {

/**
 * @brief Performance Profiler Window
 * 
 * Displays real-time performance metrics:
 * - Frame time graph (last 100 frames)
 * - CPU usage percentage
 * - GPU memory usage
 * - Draw call count
 * - Entity count
 * - FPS counter
 * 
 * Usage:
 * ```cpp
 * Profiler profiler;
 * 
 * // Game loop
 * while (running) {
 *     float deltaTime = GetDeltaTime();
 *     profiler.Update(deltaTime);
 *     profiler.Render();
 * }
 * ```
 */
class Profiler {
public:
    Profiler();

    /**
     * @brief Update profiler metrics
     * @param deltaTime Frame time in seconds
     */
    void Update(float deltaTime);

    /**
     * @brief Render profiler window
     */
    void Render();

    /**
     * @brief Set draw call count
     */
    void SetDrawCalls(int count) { m_DrawCalls = count; }

    /**
     * @brief Set entity count
     */
    void SetEntityCount(int count) { m_EntityCount = count; }

    /**
     * @brief Set GPU memory usage (bytes)
     */
    void SetGPUMemory(size_t bytes) { m_GPUMemory = bytes; }

    /**
     * @brief Check if window is open
     */
    bool IsOpen() const { return m_IsOpen; }

    /**
     * @brief Set window open state
     */
    void SetOpen(bool open) { m_IsOpen = open; }

private:
    static constexpr int HISTORY_SIZE = 100;

    std::array<float, HISTORY_SIZE> m_FrameTimes{};
    int m_FrameIndex = 0;
    float m_FPS = 0.0f;
    float m_AverageFrameTime = 0.0f;
    int m_DrawCalls = 0;
    int m_EntityCount = 0;
    size_t m_GPUMemory = 0;
    bool m_IsOpen = true;
};

} // namespace SAGE
