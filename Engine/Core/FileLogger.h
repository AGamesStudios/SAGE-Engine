#pragma once

#include "Core/Logger.h"
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace SAGE {

/// @brief File-based logger for production builds
/// Writes logs to file in addition to console output
/// Thread-safe and supports log rotation
class FileLogger {
public:
    /// @brief Create file logger
    /// @param filename Path to log file (default: "engine.log")
    /// @param append If true, append to existing file; otherwise overwrite
    explicit FileLogger(const std::string& filename = "engine.log", bool append = true)
        : m_Filename(filename)
        , m_Append(append)
    {
        Open();
    }

    ~FileLogger() {
        Close();
    }

    // Non-copyable, movable
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;
    FileLogger(FileLogger&&) noexcept = default;
    FileLogger& operator=(FileLogger&&) noexcept = default;

    /// @brief Write log message to file
    /// @param level Log level
    /// @param message Message to write
    void Log(LogLevel level, const std::string& message) {
        if (!m_File.is_open()) {
            return;
        }

        m_File << GetTimestamp() << " [" << LogLevelToString(level) << "] " 
               << message << std::endl;
        m_File.flush(); // Ensure immediate write
    }

    /// @brief Close current log file
    void Close() {
        if (m_File.is_open()) {
            m_File.close();
        }
    }

    /// @brief Rotate log file (create new file with timestamp)
    void Rotate() {
        Close();
        
        // Rename old file with timestamp
        std::string newName = m_Filename + "." + GetTimestamp();
        std::rename(m_Filename.c_str(), newName.c_str());
        
        // Open new file
        Open();
    }

    /// @brief Get current log file path
    const std::string& GetFilename() const { return m_Filename; }

private:
    void Open() {
        auto mode = m_Append ? (std::ios::out | std::ios::app) : std::ios::out;
        m_File.open(m_Filename, mode);
        
        if (!m_File.is_open()) {
            SAGE_ERROR("Failed to open log file: {}", m_Filename);
            return;
        }

        // Write header
        m_File << "========================================" << std::endl;
        m_File << "SAGE Engine Log - " << GetTimestamp() << std::endl;
        m_File << "========================================" << std::endl;
    }

    static std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        #ifdef _MSC_VER
        struct tm timeInfo;
        localtime_s(&timeInfo, &time);
        oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
        #else
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        #endif
        
        return oss.str();
    }

    static const char* LogLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

    std::string m_Filename;
    bool m_Append;
    std::ofstream m_File;
};

} // namespace SAGE
