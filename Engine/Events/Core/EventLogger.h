#pragma once

#include "EventBus.h"
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace SAGE {

struct LoggedEventRecord {
    std::string name;
    uint64_t eventId = 0;
    int priority = 0;
    uint64_t durationMicros = 0;
    double ageMillis = 0.0;
};

class EventLogger {
public:
    explicit EventLogger(size_t maxRecords = 1024)
        : m_MaxRecords(maxRecords) {}

    void Attach(EventBus& bus);
    void Detach();

    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }

    // Filtering options
    void SetMinPriority(int p) { m_MinPriority = p; }

    std::vector<LoggedEventRecord> GetSnapshot() const;
    void Reset();

private:
    EventBus* m_Bus = nullptr;
    EventBus::HookId m_PostId = 0;
    bool m_Enabled = true;
    int m_MinPriority = std::numeric_limits<int>::min();
    size_t m_MaxRecords;

    mutable std::mutex m_Mutex;
    std::deque<LoggedEventRecord> m_Records;
};

} // namespace SAGE
