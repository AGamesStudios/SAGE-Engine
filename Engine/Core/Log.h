#pragma once
// Simple logging API wrappers around Logger for minimal names.
#include "Logger.h"
#include <string>

namespace SAGE {

inline void LogTrace(const std::string& msg) { Logger::Trace("{}", msg); }
inline void LogInfo(const std::string& msg)  { Logger::Info("{}", msg); }
inline void LogWarn(const std::string& msg)  { Logger::Warning("{}", msg); }
inline void LogError(const std::string& msg) { Logger::Error("{}", msg); }
inline void LogFatal(const std::string& msg) { Logger::Error("FATAL: {}", msg); }

} // namespace SAGE

#define LOG_TRACE(msg)  ::SAGE::LogTrace(msg)
#define LOG_INFO(msg)   ::SAGE::LogInfo(msg)
#define LOG_WARN(msg)   ::SAGE::LogWarn(msg)
#define LOG_ERROR(msg)  ::SAGE::LogError(msg)
#define LOG_FATAL(msg)  ::SAGE::LogFatal(msg)

// Assertion macros (basic names per request)
#include <cassert>
#define SAGE_ASSERT(expr, msg) do { if(!(expr)) { LOG_FATAL(msg); assert(expr); } } while(0)
#define SAGE_VERIFY(expr, msg) do { if(!(expr)) { LOG_ERROR(msg); } } while(0)
