#pragma once

#include "EventBus.h"
#include <unordered_map>
#include <mutex>
#include <string>

namespace SAGE {

struct EventProfileStats {
    uint64_t count = 0;
    uint64_t totalMicros = 0;
    uint64_t maxMicros = 0;
    double averageMicros = 0.0;
};

/// Simple profiler that hooks into EventBus pre/post publish hooks.
class EventProfiler {
public:
    EventProfiler() = default;

    void Attach(EventBus& bus);
    void Detach();

    bool IsAttached() const { return m_Bus != nullptr; }

    // Thread-safe snapshot
    std::unordered_map<std::string, EventProfileStats> GetSnapshot() const;

    void Reset();

private:
    EventBus* m_Bus = nullptr;
    EventBus::HookId m_PreId = 0;
    EventBus::HookId m_PostId = 0;

    mutable std::mutex m_Mutex;
    struct InFlightRecord { uint64_t startMicros; };
    // Key: event id unique number -> start time
    std::unordered_map<uint64_t, InFlightRecord> m_InFlight;
    // Aggregated by event name
    std::unordered_map<std::string, EventProfileStats> m_Stats;
};

} // namespace SAGE
