#include "EventProfiler.h"
#include <chrono>

namespace SAGE {

static uint64_t NowMicros() {
    auto now = std::chrono::high_resolution_clock::now();
    return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

void EventProfiler::Attach(EventBus& bus) {
    if (m_Bus) return; // already attached
    m_Bus = &bus;
    m_PreId = bus.AddPrePublishHook([this](const Event& e){
        std::lock_guard lk(m_Mutex);
        m_InFlight[e.GetEventId()] = { NowMicros() };
    });
    m_PostId = bus.AddPostPublishHook([this](const Event& e, uint64_t duration){
        std::lock_guard lk(m_Mutex);
        auto name = std::string(e.GetName());
        auto& stats = m_Stats[name];
        stats.count++;
        stats.totalMicros += duration;
        if (duration > stats.maxMicros) stats.maxMicros = duration;
        stats.averageMicros = stats.totalMicros / (double)stats.count;
        // Remove in-flight record if present
        m_InFlight.erase(e.GetEventId());
    });
}

void EventProfiler::Detach() {
    if (!m_Bus) return;
    m_Bus->RemovePrePublishHook(m_PreId);
    m_Bus->RemovePostPublishHook(m_PostId);
    m_Bus = nullptr;
    m_PreId = 0;
    m_PostId = 0;
    {
        std::lock_guard lk(m_Mutex);
        m_InFlight.clear();
    }
}

std::unordered_map<std::string, EventProfileStats> EventProfiler::GetSnapshot() const {
    std::lock_guard lk(m_Mutex);
    return m_Stats; // copy
}

void EventProfiler::Reset() {
    std::lock_guard lk(m_Mutex);
    m_Stats.clear();
    m_InFlight.clear();
}

} // namespace SAGE
