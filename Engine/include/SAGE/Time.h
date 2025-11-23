#pragma once

#include <chrono>

namespace SAGE {

class Time {
public:
    using Clock = std::chrono::high_resolution_clock;

    static void Reset();
    static void Tick();

    static double Delta();      // Scaled delta time
    static double UnscaledDelta(); // Raw delta time
    static double FixedDelta(); // Fixed delta time for physics
    static double Elapsed();    // Seconds since Reset
    
    static void SetTimeScale(double scale);
    static double GetTimeScale();

private:
    static Clock::time_point& LastTime();
    static Clock::time_point& StartTime();
    static double& DeltaSeconds();
    static double& UnscaledDeltaSeconds();
    static double& TimeScale();
    static double& FixedDeltaSeconds();
};

} // namespace SAGE
