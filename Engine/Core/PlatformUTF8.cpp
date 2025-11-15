#include "PlatformUTF8.h"

#include <filesystem>
#include <mutex>
#include <system_error>
#include <vector>

// Platform-specific UTF8 support removed - using standard C++17 filesystem
// #if defined(_WIN32)
//     #include "../Platform/Windows/WindowsUTF8Support.h"
// #else
//     #include "../Platform/Unix/UnixUTF8Support.h"
// #endif

namespace SAGE::Platform {

namespace {

using Utf8String = UTF8Support::Utf8String;

std::filesystem::path ToFsPath(const Utf8String& path) {
    if (path.empty()) {
        return {};
    }

    std::u8string u8;
    u8.reserve(path.size());
    for (unsigned char ch : path) {
        u8.push_back(static_cast<char8_t>(ch));
    }

    return std::filesystem::path(u8);
}

Utf8String FromFsPath(const std::filesystem::path& path) {
    auto u8 = path.u8string();
    Utf8String result;
    result.reserve(u8.size());
    for (char8_t ch : u8) {
        result.push_back(static_cast<char>(ch));
    }
    return result;
}

} // namespace

bool UTF8Support::s_initialized = false;

bool UTF8Support::Initialize() {
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        s_initialized = InitializeInternal();
    });
    return s_initialized;
}

bool UTF8Support::IsInitialized() {
    return Initialize();
}

bool UTF8Support::CreateDirectory(const Utf8String& path) {
    Initialize();

    if (path.empty()) {
        return false;
    }

    std::error_code ec;
    const std::filesystem::path fsPath = ToFsPath(path);
    std::filesystem::create_directories(fsPath, ec);
    if (!ec) {
        return std::filesystem::exists(fsPath, ec) && std::filesystem::is_directory(fsPath, ec);
    }
    return false;
}

bool UTF8Support::FileExists(const Utf8String& path) {
    Initialize();

    if (path.empty()) {
        return false;
    }

    const std::filesystem::path fsPath = ToFsPath(path);
    std::error_code ec;
    return std::filesystem::exists(fsPath, ec) && !ec;
}

std::vector<Utf8String> UTF8Support::ListDirectory(const Utf8String& path) {
    Initialize();

    std::vector<Utf8String> entries;

    const std::filesystem::path fsPath = ToFsPath(path);
    std::error_code ec;
    std::filesystem::directory_iterator iter(fsPath, ec);
    if (ec) {
        return entries;
    }

    const std::filesystem::directory_iterator end;
    for (; iter != end; iter.increment(ec)) {
        if (ec) {
            entries.clear();
            return entries;
        }
        entries.push_back(FromFsPath(iter->path().filename()));
    }

    return entries;
}

UTF8Support::Utf8String UTF8Support::GetCurrentDirectory() {
    Initialize();

    std::error_code ec;
    const auto current = std::filesystem::current_path(ec);
    if (ec) {
        return {};
    }

    return FromFsPath(current);
}

UTF8Support::Utf8String UTF8Support::NormalizePath(const Utf8String& path) {
    Initialize();

    if (path.empty()) {
        return {};
    }

    std::filesystem::path fsPath = ToFsPath(path);
    std::error_code ec;
    fsPath = std::filesystem::weakly_canonical(fsPath, ec);
    if (ec) {
        ec.clear();
        fsPath = fsPath.lexically_normal();
    }
    fsPath.make_preferred();

    return FromFsPath(fsPath);
}

bool UTF8Support::InitializeInternal() {
    // Platform-specific console initialization removed - using standard C++17
    // Modern terminals support UTF-8 by default
    return true;
    
    // #if defined(_WIN32)
    //     return WindowsUTF8Support::InitializeConsole();
    // #else
    //     return UnixUTF8Support::InitializeConsole();
    // #endif
}

} // namespace SAGE::Platform
