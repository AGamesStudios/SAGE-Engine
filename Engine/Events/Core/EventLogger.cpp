#include "EventLogger.h"
#include <limits>

namespace SAGE {

void EventLogger::Attach(EventBus& bus) {
    if (m_Bus) return;
    m_Bus = &bus;
    m_PostId = bus.AddPostPublishHook([this](const Event& e, uint64_t duration){
        if (!m_Enabled) return;
        if (e.GetPriority() < m_MinPriority) return;
        LoggedEventRecord rec;
        rec.name = e.GetName();
        rec.eventId = e.GetEventId();
        rec.priority = e.GetPriority();
        rec.durationMicros = duration;
        rec.ageMillis = e.GetAge() * 1000.0;
        std::lock_guard lk(m_Mutex);
        if (m_Records.size() == m_MaxRecords) {
            m_Records.pop_front();
        }
        m_Records.push_back(std::move(rec));
    });
}

void EventLogger::Detach() {
    if (!m_Bus) return;
    m_Bus->RemovePostPublishHook(m_PostId);
    m_Bus = nullptr;
    m_PostId = 0;
    Reset();
}

std::vector<LoggedEventRecord> EventLogger::GetSnapshot() const {
    std::lock_guard lk(m_Mutex);
    return { m_Records.begin(), m_Records.end() };
}

void EventLogger::Reset() {
    std::lock_guard lk(m_Mutex);
    m_Records.clear();
}

} // namespace SAGE
