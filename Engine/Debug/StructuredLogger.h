#pragma once

#include "Core/Logger.h"
#include <string>
#include <vector>
#include <chrono>

namespace SAGE {
namespace Debug {

/**
 * @brief Logger with structured logging, levels, and filtering
 */
class StructuredLogger {
public:
    enum class Level {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };
    
    struct LogEntry {
        Level level;
        std::string message;
        std::string category;
        std::chrono::system_clock::time_point timestamp;
    };
    
    static StructuredLogger& Get();
    
    void Log(Level level, const std::string& category, const std::string& message);
    
    void SetMinLevel(Level level) { m_MinLevel = level; }
    void EnableCategory(const std::string& category);
    void DisableCategory(const std::string& category);
    
    const std::vector<LogEntry>& GetHistory() const { return m_History; }
    void ClearHistory() { m_History.clear(); }
    
    // Convenience methods
    void Trace(const std::string& category, const std::string& msg) { Log(Level::Trace, category, msg); }
    void Debug(const std::string& category, const std::string& msg) { Log(Level::Debug, category, msg); }
    void Info(const std::string& category, const std::string& msg) { Log(Level::Info, category, msg); }
    void Warning(const std::string& category, const std::string& msg) { Log(Level::Warning, category, msg); }
    void Error(const std::string& category, const std::string& msg) { Log(Level::Error, category, msg); }
    void Critical(const std::string& category, const std::string& msg) { Log(Level::Critical, category, msg); }
    
private:
    StructuredLogger() = default;
    
    Level m_MinLevel = Level::Info;
    std::vector<std::string> m_EnabledCategories;
    std::vector<LogEntry> m_History;
    const size_t m_MaxHistory = 1000;
    
    bool IsCategoryEnabled(const std::string& category) const;
    std::string LevelToString(Level level) const;
};

// Macros for easy logging
#define SAGE_LOG_TRACE(category, msg) SAGE::Debug::StructuredLogger::Get().Trace(category, msg)
#define SAGE_LOG_DEBUG(category, msg) SAGE::Debug::StructuredLogger::Get().Debug(category, msg)
#define SAGE_LOG_INFO(category, msg) SAGE::Debug::StructuredLogger::Get().Info(category, msg)
#define SAGE_LOG_WARNING(category, msg) SAGE::Debug::StructuredLogger::Get().Warning(category, msg)
#define SAGE_LOG_ERROR(category, msg) SAGE::Debug::StructuredLogger::Get().Error(category, msg)
#define SAGE_LOG_CRITICAL(category, msg) SAGE::Debug::StructuredLogger::Get().Critical(category, msg)

} // namespace Debug
} // namespace SAGE
