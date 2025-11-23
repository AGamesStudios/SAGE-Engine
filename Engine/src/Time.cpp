#include "SAGE/Time.h"

namespace SAGE {

Time::Clock::time_point& Time::LastTime() {
    static Clock::time_point last = Clock::now();
    return last;
}

Time::Clock::time_point& Time::StartTime() {
    static Clock::time_point start = Clock::now();
    return start;
}

double& Time::DeltaSeconds() {
    static double delta = 0.0;
    return delta;
}

double& Time::UnscaledDeltaSeconds() {
    static double delta = 0.0;
    return delta;
}

double& Time::TimeScale() {
    static double scale = 1.0;
    return scale;
}

double& Time::FixedDeltaSeconds() {
    static double fixed = 1.0 / 60.0;
    return fixed;
}

void Time::Reset() {
    StartTime() = Clock::now();
    LastTime() = StartTime();
    DeltaSeconds() = 0.0;
    UnscaledDeltaSeconds() = 0.0;
    TimeScale() = 1.0;
}

void Time::Tick() {
    const auto now = Clock::now();
    double dt = std::chrono::duration<double>(now - LastTime()).count();
    LastTime() = now;
    
    UnscaledDeltaSeconds() = dt;
    
    // Cap delta time to prevent spiral of death (max 0.25s)
    if (dt > 0.25) dt = 0.25;
    
    DeltaSeconds() = dt * TimeScale();
}

double Time::Delta() {
    return DeltaSeconds();
}

double Time::UnscaledDelta() {
    return UnscaledDeltaSeconds();
}

double Time::FixedDelta() {
    return FixedDeltaSeconds();
}

double Time::Elapsed() {
    return std::chrono::duration<double>(Clock::now() - StartTime()).count();
}

void Time::SetTimeScale(double scale) {
    if (scale >= 0.0) {
        TimeScale() = scale;
    }
}

double Time::GetTimeScale() {
    return TimeScale();
}

} // namespace SAGE
