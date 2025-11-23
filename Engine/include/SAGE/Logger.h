#pragma once

#include <iosfwd>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace SAGE {

enum class LogLevel {
    Trace = 0,
    Info,
    Warn,
    Error,
    Critical
};

class Logger {
public:
    static void Init();
    static void Shutdown();

    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();

    template<typename... Args>
    static void Trace(Args&&... args) {
        Log(LogLevel::Trace, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Info(Args&&... args) {
        Log(LogLevel::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Warn(Args&&... args) {
        Log(LogLevel::Warn, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Error(Args&&... args) {
        Log(LogLevel::Error, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Critical(Args&&... args) {
        Log(LogLevel::Critical, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    static void Log(LogLevel level, Args&&... args) {
        if (level < GetLevel()) {
            return;
        }
        Write(level, BuildMessage(std::forward<Args>(args)...));
    }

    template<typename T>
    static void FormatArg(std::ostringstream& stream, const T& arg) {
        stream << arg;
    }

    template<typename Arg, typename... Args>
    static std::string BuildMessage(std::string_view fmt, Arg&& arg, Args&&... args) {
        std::ostringstream stream;
        size_t pos = 0;
        size_t lastPos = 0;

        while ((pos = fmt.find("{}", lastPos)) != std::string_view::npos) {
            stream << fmt.substr(lastPos, pos - lastPos);
            FormatArg(stream, std::forward<Arg>(arg));
            lastPos = pos + 2;
            
            if constexpr (sizeof...(args) > 0) {
                return stream.str() + BuildMessage(fmt.substr(lastPos), std::forward<Args>(args)...);
            } else {
                stream << fmt.substr(lastPos);
                return stream.str();
            }
        }
        
        stream << fmt.substr(lastPos);
        FormatArg(stream, std::forward<Arg>(arg));
        if constexpr (sizeof...(args) > 0) {
            (FormatArg(stream, std::forward<Args>(args)), ...);
        }
        return stream.str();
    }

    static std::string BuildMessage(std::string_view fmt) {
        return std::string(fmt);
    }

    static void Write(LogLevel level, std::string_view message);
};

} // namespace SAGE
