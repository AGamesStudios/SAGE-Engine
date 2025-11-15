#include "Logger.h"

#include <cctype>
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <thread>

#include "PlatformUTF8.h"

namespace SAGE {

std::mutex Logger::s_Mutex;
std::ofstream Logger::s_File;
std::atomic<bool> Logger::s_FileEnabled{true};
std::atomic<bool> Logger::s_Initialized{false}; // Идемпотентность инициализации
std::atomic<LogLevel> Logger::s_MinLevel{LogLevel::Trace};
size_t Logger::s_MaxBytes = 5 * 1024 * 1024; // 5 MB rotation default
std::string Logger::s_LogDir;
std::string Logger::s_LogPath;
std::vector<LogMessage> Logger::s_Buffer;
std::vector<std::string> Logger::s_DisabledCategories;

void Logger::Init(const std::string& logDir) {
    static std::once_flag initFlag;
    std::call_once(initFlag, [&]() {
        s_LogDir = logDir;
        if (!SAGE::Platform::UTF8Support::Initialize()) {
            std::cerr << "[log] utf8 init failed" << std::endl;
        }
        try {
            std::filesystem::create_directories(s_LogDir);
        } catch (...) {}
        s_LogPath = s_LogDir + "/engine.log";
        s_File.open(s_LogPath, std::ios::out | std::ios::app);
        if (!s_File.is_open()) {
            std::cerr << "[log] cannot open file: " << s_LogPath << std::endl;
            s_FileEnabled = false;
        }
        s_Initialized.store(true); // Установить флаг после успешной инициализации
        Info("Logger started (file: %s)", s_LogPath);
    });
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_Mutex);
    Flush();
    if (s_File.is_open()) s_File.close();
}

void Logger::SetMinLevel(LogLevel level) { s_MinLevel = level; }
void Logger::SetFileLogging(bool enabled) { s_FileEnabled = enabled; }
void Logger::SetCategoryEnabled(const std::string& category, bool enabled) {
    auto it = std::find(s_DisabledCategories.begin(), s_DisabledCategories.end(), category);
    if (!enabled) {
        if (it == s_DisabledCategories.end()) s_DisabledCategories.push_back(category);
    } else if (it != s_DisabledCategories.end()) {
        s_DisabledCategories.erase(it);
    }
}
void Logger::EnableRotation(size_t maxBytes) { s_MaxBytes = maxBytes; }

bool Logger::IsCategoryEnabled(const std::string& category) {
    return std::find(s_DisabledCategories.begin(), s_DisabledCategories.end(), category) == s_DisabledCategories.end();
}

void Logger::InternalLog(LogLevel level, const std::string& category, const std::string& message) {
    LogMessage msg{level, category, message, std::this_thread::get_id(), std::chrono::system_clock::now()};
    
    // FUTURE OPTIMIZATION (#26): Replace std::mutex with lock-free circular buffer
    // Current implementation: mutex-protected vector (simple, correct, but contended)
    // Proposed: Lock-free SPSC/MPSC ring buffer with atomic operations
    // Trade-off: Current approach is sufficient for most use cases (<1000 logs/sec)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_Buffer.push_back(msg);
        if (s_Buffer.size() > 1024) s_Buffer.erase(s_Buffer.begin()); // ring buffer trim
    }

    // Compose prefix
    std::ostringstream ossPrefix;
    ossPrefix << '[' << TimeToString(msg.timestamp) << ']';
    ossPrefix << '[' << category << ']';
    switch (level) {
        case LogLevel::Trace:   ossPrefix << "[trace]"; break;
        case LogLevel::Info:    ossPrefix << "[info ]"; break;
        case LogLevel::Warning: ossPrefix << "[warn ]"; break;
        case LogLevel::Error:   ossPrefix << "[error]"; break;
        case LogLevel::Fatal:   ossPrefix << "[fatal]"; break;
    }
    ossPrefix << ' ' << message;
    std::string finalLine = ossPrefix.str();

    if (level == LogLevel::Error || level == LogLevel::Fatal)
        std::cerr << finalLine << std::endl;
    else
        std::cout << finalLine << std::endl;

    // File I/O protected by mutex (unavoidable for std::ofstream)
    // FUTURE OPTIMIZATION (#26): Async file writer thread with lock-free queue
    if (s_FileEnabled && s_File.is_open()) {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_File << finalLine << std::endl;
        RotateIfNeeded();
    }
}

std::string Logger::Format(const std::string& format, const std::vector<std::string>& args) {
    std::string result; result.reserve(format.size() + args.size() * 4);
    size_t sequentialIndex = 0;
    for (size_t i = 0; i < format.size(); ++i) {
        char c = format[i];
        if (c == '%' && i + 1 < format.size()) {
            char next = format[i + 1];
            if (next == '%') { result += '%'; ++i; continue; }
            if (next == 'd' || next == 'i' || next == 'f' || next == 's' || next == 'u' || next == 'x' || next == 'X' || next == 'c') {
                if (sequentialIndex < args.size()) result += args[sequentialIndex++]; else { result += '%'; result += next; }
                ++i; continue;
            }
            size_t j = i + 1;
            while (j < format.size() && (std::isdigit(static_cast<unsigned char>(format[j])) || format[j] == '.')) ++j;
            if (j < format.size()) {
                char spec = format[j];
                if (spec == 'd' || spec == 'i' || spec == 'f' || spec == 's' || spec == 'u' || spec == 'x' || spec == 'X' || spec == 'c') {
                    if (sequentialIndex < args.size()) result += args[sequentialIndex++]; else result.append(format, i, j - i + 1);
                    i = j; continue;
                }
            }
        }
        if (c == '{') {
            size_t j = i + 1; bool hasIndex = false; size_t explicitIndex = 0;
            while (j < format.size() && std::isdigit(static_cast<unsigned char>(format[j]))) { hasIndex = true; explicitIndex = explicitIndex * 10 + (format[j]-'0'); ++j; }
            if (j < format.size() && format[j] == '}') {
                size_t idx = hasIndex ? explicitIndex : sequentialIndex++;
                if (idx < args.size()) result += args[idx]; else result += "{}";
                i = j; continue;
            }
        }
        if (c == '}' && i + 1 < format.size() && format[i + 1] == '}') { result += '}'; ++i; continue; }
        result += c;
    }
    return result;
}

std::string Logger::TimeToString(const std::chrono::system_clock::time_point& tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#ifdef _MSC_VER
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32]; std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

void Logger::Flush() {
    if (s_File.is_open()) s_File.flush();
}

void Logger::RotateIfNeeded() {
    if (!s_File.is_open()) return;
    auto size = s_File.tellp();
    if (size > static_cast<std::streampos>(s_MaxBytes)) {
        s_File.close();
        std::string rotated = s_LogPath + ".1"; // simple single rotation
        std::error_code ec; std::filesystem::remove(rotated, ec);
        std::filesystem::rename(s_LogPath, rotated, ec);
        s_File.open(s_LogPath, std::ios::out | std::ios::trunc);
    }
}

void Logger::Abort() {
    // Attempt stack trace placeholder
    std::cerr << "[fatal] terminating application" << std::endl;
    std::abort();
}

}
