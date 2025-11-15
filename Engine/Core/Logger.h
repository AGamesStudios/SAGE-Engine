#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <fstream>
#include <atomic>
#include <chrono>

namespace SAGE {

    enum class LogLevel {
        Trace,
        Info,
        Warning,
        Error,
        Fatal
    };

    struct LogMessage {
        LogLevel level;
        std::string category;
        std::string text;
        std::thread::id threadId;
        std::chrono::system_clock::time_point timestamp;
    };

    /**
     * @brief Static logger class
     * 
     * TODO (#22): Convert to instance-based Logger for better testability
     * Current: Static methods with static state (global singleton)
     * Future: Logger instance owned by EngineContext
     * 
     * Migration plan:
     * 1. Create Logger class with instance methods
     * 2. Add Logger* to EngineContext
     * 3. Update SAGE_* macros to use g_Logger global pointer
     * 4. Gradually migrate code to dependency injection
     */
    class Logger {
    public:
        // Basic Init with optional log directory (идемпотентна)
        static void Init(const std::string& logDir = "logs");
        static void Shutdown();
        static bool IsInitialized() { return s_Initialized.load(); }
        static void SetMinLevel(LogLevel level);
        static void SetFileLogging(bool enabled);
        static void SetCategoryEnabled(const std::string& category, bool enabled);
        static void EnableRotation(size_t maxBytes);

        template<typename... Args>
        static void Trace(const std::string& format, Args&&... args) { Log(LogLevel::Trace, "core", format, std::forward<Args>(args)...); }
        template<typename... Args>
        static void Info(const std::string& format, Args&&... args) { Log(LogLevel::Info, "core", format, std::forward<Args>(args)...); }
        template<typename... Args>
        static void Warning(const std::string& format, Args&&... args) { Log(LogLevel::Warning, "core", format, std::forward<Args>(args)...); }
        template<typename... Args>
        static void Error(const std::string& format, Args&&... args) { Log(LogLevel::Error, "core", format, std::forward<Args>(args)...); }
        template<typename... Args>
        static void Fatal(const std::string& format, Args&&... args) { 
            Log(LogLevel::Fatal, "core", format, std::forward<Args>(args)...); 
            Flush(); 
            WriteCrashInfo(); // Сохранить crashlog перед abort
            Abort(); 
        }

        // Category versions
        template<typename... Args>
        static void Cat(LogLevel level, const std::string& category, const std::string& format, Args&&... args) {
            Log(level, category, format, std::forward<Args>(args)...);
        }

    private:
        template<typename... Args>
        static void Log(LogLevel level, const std::string& category, const std::string& format, Args&&... args) {
            // OPTIMIZATION: Check level/category BEFORE expensive string formatting
            if (level < s_MinLevel) return;
            if (!IsCategoryEnabled(category)) return;
            
            // Lazy formatting - only convert args if we're actually logging
            const std::vector<std::string> arguments = { ToString(std::forward<Args>(args))... };
            InternalLog(level, category, Format(format, arguments));
        }

        static bool IsCategoryEnabled(const std::string& category);
        static void InternalLog(LogLevel level, const std::string& category, const std::string& message);
        static std::string Format(const std::string& format, const std::vector<std::string>& args);
        static std::string TimeToString(const std::chrono::system_clock::time_point& tp);
        static void Flush();
        static void RotateIfNeeded();
        static void Abort();

        template<typename T>
        static std::string ToString(T&& value) {
            std::ostringstream stream;
            stream << std::forward<T>(value);
            return stream.str();
        }

        static void WriteCrashInfo(); // Сохранить информацию о крахе
        
        static std::mutex s_Mutex;
        static std::ofstream s_File;
        static std::atomic<bool> s_FileEnabled;
        static std::atomic<bool> s_Initialized; // Защита от повторной инициализации
        static std::atomic<LogLevel> s_MinLevel;
        static size_t s_MaxBytes;
        static std::string s_LogDir;
        static std::string s_LogPath;
        static std::vector<LogMessage> s_Buffer;
        static std::vector<std::string> s_DisabledCategories;
    };

} // namespace SAGE

#define SAGE_TRACE(...)    ::SAGE::Logger::Trace(__VA_ARGS__)
#define SAGE_INFO(...)     ::SAGE::Logger::Info(__VA_ARGS__)
#define SAGE_WARNING(...)  ::SAGE::Logger::Warning(__VA_ARGS__)
#define SAGE_WARN(...)     ::SAGE::Logger::Warning(__VA_ARGS__)
#define SAGE_ERROR(...)    ::SAGE::Logger::Error(__VA_ARGS__)
#define SAGE_FATAL(...)    ::SAGE::Logger::Fatal(__VA_ARGS__)

#define SAGE_LOG_CAT(level, cat, ...) ::SAGE::Logger::Cat(level, cat, __VA_ARGS__)
