#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <string>

namespace SAGE {

// Timer class for scheduling callbacks
class Timer {
public:
    using Callback = std::function<void()>;
    
    Timer() = default;
    
    // Schedule a one-time callback after delay (in seconds)
    void ScheduleOnce(float delay, Callback callback, const std::string& name = "");
    
    // Schedule a repeating callback with interval (in seconds)
    void ScheduleRepeating(float interval, Callback callback, const std::string& name = "");
    
    // Cancel timer by name
    void Cancel(const std::string& name);
    
    // Cancel all timers
    void CancelAll();
    
    // Update timers (call every frame with deltaTime)
    void Update(float deltaTime);
    
    // Get number of active timers
    size_t GetActiveTimerCount() const { return m_Timers.size(); }

private:
    struct TimerData {
        std::string name;
        float delay;
        float elapsed;
        bool repeat;
        Callback callback;
    };
    
    std::vector<TimerData> m_Timers;
};

// Global frame counter and time tracking
class TimeTracker {
public:
    static TimeTracker& Get();
    
    void Update(float deltaTime);
    void Reset();
    
    // Frame information
    uint64_t GetFrameCount() const { return m_FrameCount; }
    float GetDeltaTime() const { return m_DeltaTime; }
    float GetTotalTime() const { return m_TotalTime; }
    
    // FPS calculation
    float GetFPS() const { return m_FPS; }
    float GetAverageFPS() const;
    
    // Time scale (for slow-motion/fast-forward effects)
    void SetTimeScale(float scale) { m_TimeScale = scale; }
    float GetTimeScale() const { return m_TimeScale; }
    float GetScaledDeltaTime() const { return m_DeltaTime * m_TimeScale; }

private:
    TimeTracker() = default;
    
    uint64_t m_FrameCount = 0;
    float m_DeltaTime = 0.0f;
    float m_TotalTime = 0.0f;
    float m_TimeScale = 1.0f;
    float m_FPS = 0.0f;
    
    // For FPS averaging
    static constexpr size_t FPS_SAMPLE_COUNT = 60;
    std::vector<float> m_FPSSamples;
    size_t m_FPSSampleIndex = 0;
};

} // namespace SAGE
