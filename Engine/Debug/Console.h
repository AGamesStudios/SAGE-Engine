// Engine/Debug/Console.h
#pragma once

#include <vector>
#include <string>

namespace SAGE {

enum class LogLevel {
    Info,
    Warning,
    Error
};

struct LogEntry {
    std::string Message;
    LogLevel Level;
    double Timestamp;
};

/**
 * @brief Console Window for displaying logs
 * 
 * Captures all SAGE_INFO, SAGE_WARN, SAGE_ERROR messages
 * with filtering, scrolling, and clear functionality.
 */
class Console {
public:
    Console();

    /**
     * @brief Add log entry
     */
    void AddLog(const std::string& message, LogLevel level = LogLevel::Info);

    /**
     * @brief Render console window
     */
    void Render();

    /**
     * @brief Clear all logs
     */
    void Clear();

    /**
     * @brief Check if window is open
     */
    bool IsOpen() const { return m_IsOpen; }

    /**
     * @brief Set window open state
     */
    void SetOpen(bool open) { m_IsOpen = open; }

private:
    std::vector<LogEntry> m_Logs;
    bool m_IsOpen = true;
    bool m_AutoScroll = true;
    bool m_FilterInfo = true;
    bool m_FilterWarning = true;
    bool m_FilterError = true;
};

} // namespace SAGE
