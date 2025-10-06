#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace SAGE {

    enum class LogLevel {
        Trace,
        Info,
        Warning,
        Error
    };

    class Logger {
    public:
        static void Init();

        template<typename... Args>
        static void Trace(const std::string& format, Args&&... args) {
            Log(LogLevel::Trace, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Info(const std::string& format, Args&&... args) {
            Log(LogLevel::Info, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Warning(const std::string& format, Args&&... args) {
            Log(LogLevel::Warning, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Error(const std::string& format, Args&&... args) {
            Log(LogLevel::Error, format, std::forward<Args>(args)...);
        }

    private:
        template<typename... Args>
        static void Log(LogLevel level, const std::string& format, Args&&... args) {
            const std::vector<std::string> arguments = { ToString(std::forward<Args>(args))... };
            Log(level, Format(format, arguments));
        }

        static void Log(LogLevel level, const std::string& message);
        static std::string Format(const std::string& format, const std::vector<std::string>& args);

        template<typename T>
        static std::string ToString(T&& value) {
            std::ostringstream stream;
            stream << std::forward<T>(value);
            return stream.str();
        }
    };

} // namespace SAGE

#define SAGE_TRACE(...)    ::SAGE::Logger::Trace(__VA_ARGS__)
#define SAGE_INFO(...)     ::SAGE::Logger::Info(__VA_ARGS__)
#define SAGE_WARNING(...)  ::SAGE::Logger::Warning(__VA_ARGS__)
#define SAGE_ERROR(...)    ::SAGE::Logger::Error(__VA_ARGS__)
