#include "SAGE/Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace SAGE {

namespace {
    std::ofstream s_LogFile;

    std::mutex& LogMutex() {
        static std::mutex mutex;
        return mutex;
    }

    LogLevel& CurrentLevel() {
        static LogLevel level = LogLevel::Trace;
        return level;
    }

    const char* ToString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace:    return "TRACE";
            case LogLevel::Info:     return "INFO";
            case LogLevel::Warn:     return "WARN";
            case LogLevel::Error:    return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default:                 return "UNKNOWN";
        }
    }

    std::string CurrentTimestamp() {
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const auto timeSinceEpoch = clock::to_time_t(now);

        std::tm localTime{};
    #if defined(_WIN32)
        localtime_s(&localTime, &timeSinceEpoch);
    #else
        localtime_r(&timeSinceEpoch, &localTime);
    #endif

        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }
}

void Logger::Init() {
    std::lock_guard<std::mutex> lock(LogMutex());
    
    try {
        std::filesystem::path logDir = "logs";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        std::filesystem::path logPath = logDir / "engine.log";
        s_LogFile.open(logPath, std::ios::out | std::ios::trunc);

        if (s_LogFile.is_open()) {
            std::cout << "[Logger] Log file opened: " << logPath << std::endl;
        } else {
            std::cerr << "[Logger] Failed to open log file: " << logPath << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Logger] Error initializing logger: " << e.what() << std::endl;
    }
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(LogMutex());
    if (s_LogFile.is_open()) {
        s_LogFile.close();
    }
}

void Logger::SetLevel(LogLevel level) {
    CurrentLevel() = level;
}

LogLevel Logger::GetLevel() {
    return CurrentLevel();
}

void Logger::Write(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(LogMutex());

    std::string timestamp = CurrentTimestamp();
    std::string levelStr = ToString(level);
    
    // Console output
    std::ostream& stream = (level >= LogLevel::Error) ? std::cerr : std::cout;
    stream << '[' << timestamp << "] [" << levelStr << "] " << message << std::endl;

    // File output
    if (s_LogFile.is_open()) {
        s_LogFile << '[' << timestamp << "] [" << levelStr << "] " << message << std::endl;
        s_LogFile.flush(); // Ensure logs are written immediately in case of crash
    }
}

} // namespace SAGE
