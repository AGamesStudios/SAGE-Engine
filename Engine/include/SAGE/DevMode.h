#pragma once

#include "SAGE/Log.h"

#include <atomic>
#include <string_view>

namespace SAGE::DevMode {

struct Config {
    bool enabled = true;
    bool logInput = true;
    bool logLifecycle = true;
    bool logPerformance = true;
};

class Manager {
public:
    static Manager& Instance();

    void Configure(const Config& config);
    const Config& GetConfig() const { return m_Config; }

    bool Enabled() const { return m_Config.enabled; }

    void LogLifecycle(std::string_view message) const;
    void LogInput(std::string_view message) const;
    void LogPerformance(std::string_view message) const;

private:
    Config m_Config{};
};

inline Manager& Manager::Instance() {
    static Manager instance;
    return instance;
}

inline void Manager::Configure(const Config& config) {
    m_Config = config;
    if (m_Config.enabled) {
        SAGE_INFO("DevMode enabled");
    } else {
        SAGE_INFO("DevMode disabled");
    }
}

inline void Manager::LogLifecycle(std::string_view message) const {
    if (m_Config.enabled && m_Config.logLifecycle) {
        SAGE_TRACE("[Lifecycle] ", message);
    }
}

inline void Manager::LogInput(std::string_view message) const {
    if (m_Config.enabled && m_Config.logInput) {
        SAGE_TRACE("[Input] ", message);
    }
}

inline void Manager::LogPerformance(std::string_view message) const {
    if (m_Config.enabled && m_Config.logPerformance) {
        SAGE_TRACE("[Performance] ", message);
    }
}

} // namespace SAGE::DevMode

#define SAGE_DEV_CONFIGURE(config) ::SAGE::DevMode::Manager::Instance().Configure(config)
#define SAGE_DEV_LOG_LIFECYCLE(msg) ::SAGE::DevMode::Manager::Instance().LogLifecycle(msg)
#define SAGE_DEV_LOG_INPUT(msg) ::SAGE::DevMode::Manager::Instance().LogInput(msg)
#define SAGE_DEV_LOG_PERF(msg) ::SAGE::DevMode::Manager::Instance().LogPerformance(msg)
