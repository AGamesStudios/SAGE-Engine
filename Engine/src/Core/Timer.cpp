#include "SAGE/Core/Timer.h"
#include <algorithm>

namespace SAGE {

// Timer implementation
void Timer::ScheduleOnce(float delay, Callback callback, const std::string& name) {
    if (callback) {
        m_Timers.push_back({name, delay, 0.0f, false, std::move(callback)});
    }
}

void Timer::ScheduleRepeating(float interval, Callback callback, const std::string& name) {
    if (callback) {
        m_Timers.push_back({name, interval, 0.0f, true, std::move(callback)});
    }
}

void Timer::Cancel(const std::string& name) {
    if (name.empty()) return;
    
    m_Timers.erase(
        std::remove_if(m_Timers.begin(), m_Timers.end(),
            [&name](const TimerData& timer) { return timer.name == name; }),
        m_Timers.end()
    );
}

void Timer::CancelAll() {
    m_Timers.clear();
}

void Timer::Update(float deltaTime) {
    if (m_Timers.empty()) return;
    
    // Update timers and collect completed ones
    std::vector<size_t> completedIndices;
    
    for (size_t i = 0; i < m_Timers.size(); ++i) {
        if (i >= m_Timers.size()) {
            break;
        }
        auto& timer = m_Timers[i];
        timer.elapsed += deltaTime;
        
        if (timer.elapsed >= timer.delay) {
            // Execute callback
            if (timer.callback) {
                timer.callback();
            }
            
            if (timer.repeat) {
                // Reset for next iteration
                timer.elapsed -= timer.delay;
            } else {
                // Mark for removal
                completedIndices.push_back(i);
            }
        }
    }
    
    // Remove completed one-time timers (in reverse order to maintain indices)
    for (auto it = completedIndices.rbegin(); it != completedIndices.rend(); ++it) {
        m_Timers.erase(m_Timers.begin() + *it);
    }
}

// TimeTracker implementation
TimeTracker& TimeTracker::Get() {
    static TimeTracker instance;
    return instance;
}

void TimeTracker::Update(float deltaTime) {
    m_DeltaTime = deltaTime;
    m_TotalTime += deltaTime;
    m_FrameCount++;
    
    // Calculate FPS
    if (deltaTime > 0.0f) {
        float currentFPS = 1.0f / deltaTime;
        
        // Initialize samples array if needed
        if (m_FPSSamples.empty()) {
            m_FPSSamples.resize(FPS_SAMPLE_COUNT, currentFPS);
        }
        
        // Update rolling average
        m_FPSSamples[m_FPSSampleIndex] = currentFPS;
        m_FPSSampleIndex = (m_FPSSampleIndex + 1) % FPS_SAMPLE_COUNT;
        
        m_FPS = currentFPS;
    }
}

void TimeTracker::Reset() {
    m_FrameCount = 0;
    m_TotalTime = 0.0f;
    m_FPSSamples.clear();
    m_FPSSampleIndex = 0;
}

float TimeTracker::GetAverageFPS() const {
    if (m_FPSSamples.empty()) return 0.0f;
    
    float sum = 0.0f;
    size_t validSamples = 0;
    for (float fps : m_FPSSamples) {
        if (fps > 0.0f) {
            sum += fps;
            validSamples++;
        }
    }
    return validSamples > 0 ? sum / validSamples : 0.0f;
}

} // namespace SAGE
