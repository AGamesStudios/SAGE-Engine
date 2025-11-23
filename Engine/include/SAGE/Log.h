#pragma once

#include <exception>

#include "SAGE/Logger.h"

#define SAGE_TRACE(...)    ::SAGE::Logger::Trace(__VA_ARGS__)
#define SAGE_INFO(...)     ::SAGE::Logger::Info(__VA_ARGS__)
#define SAGE_WARN(...)     ::SAGE::Logger::Warn(__VA_ARGS__)
#define SAGE_WARNING(...)  ::SAGE::Logger::Warn(__VA_ARGS__)
#define SAGE_ERROR(...)    ::SAGE::Logger::Error(__VA_ARGS__)
#define SAGE_CRITICAL(...) ::SAGE::Logger::Critical(__VA_ARGS__)

#define SAGE_CORE_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            ::SAGE::Logger::Critical(__VA_ARGS__); \
            std::terminate(); \
        } \
    } while (0)
