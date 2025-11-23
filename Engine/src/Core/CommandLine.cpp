#include "SAGE/Core/CommandLine.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <iterator>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
    #include <shellapi.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

namespace SAGE {

namespace {
    std::vector<std::string> g_Args;
    bool g_Initialised = false;
    std::mutex g_Mutex;

    std::string Narrow(const std::wstring& value) {
        if (value.empty()) {
            return {};
        }
#if defined(_WIN32)
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
        if (sizeNeeded <= 0) {
            return {};
        }
        std::string result(sizeNeeded, '\0');
        WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(), sizeNeeded, nullptr, nullptr);
        return result;
#else
        return std::string(value.begin(), value.end());
#endif
    }

    std::vector<std::string> CaptureArgsFromOS() {
#if defined(_WIN32)
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        std::vector<std::string> args;
        if (argv) {
            args.reserve(static_cast<size_t>(argc));
            for (int i = 0; i < argc; ++i) {
                args.emplace_back(Narrow(argv[i]));
            }
            LocalFree(argv);
        }
        return args;
#elif defined(__linux__)
        std::vector<std::string> args;
        std::ifstream cmdline("/proc/self/cmdline", std::ios::binary);
        if (!cmdline.is_open()) {
            return args;
        }
        std::string buffer((std::istreambuf_iterator<char>(cmdline)), std::istreambuf_iterator<char>());
        std::string current;
        for (char c : buffer) {
            if (c == '\0') {
                if (!current.empty()) {
                    args.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            args.push_back(current);
        }
        return args;
#else
        return {};
#endif
    }

    std::string MakeKey(const std::string& name) {
        if (name.rfind("--", 0) == 0) {
            return name;
        }
        return "--" + name;
    }

} // namespace

void CommandLine::Initialize() {
    EnsureInitialized();
}

const std::vector<std::string>& CommandLine::GetArgs() {
    EnsureInitialized();
    return g_Args;
}

std::optional<std::string> CommandLine::GetOption(const std::string& name) {
    EnsureInitialized();
    const auto key = MakeKey(name);
    for (size_t i = 0; i < g_Args.size(); ++i) {
        const std::string& arg = g_Args[i];
        if (arg == key) {
            if (i + 1 < g_Args.size()) {
                return g_Args[i + 1];
            }
            return std::nullopt;
        }
        const auto pos = arg.find('=');
        if (pos != std::string::npos && arg.substr(0, pos) == key) {
            return arg.substr(pos + 1);
        }
    }
    return std::nullopt;
}

bool CommandLine::HasFlag(const std::string& name) {
    EnsureInitialized();
    const auto key = MakeKey(name);
    return std::any_of(g_Args.begin(), g_Args.end(), [&](const std::string& arg) {
        if (arg == key) {
            return true;
        }
        const auto pos = arg.find('=');
        return pos != std::string::npos && arg.substr(0, pos) == key;
    });
}

void CommandLine::EnsureInitialized() {
    std::scoped_lock lock(g_Mutex);
    if (!g_Initialised) {
        g_Args = CaptureArgsFromOS();
        g_Initialised = true;
    }
}

void CommandLine::OverrideForTesting(const std::vector<std::string>& args) {
    std::scoped_lock lock(g_Mutex);
    g_Args = args;
    g_Initialised = true;
}

void CommandLine::ResetForTesting() {
    std::scoped_lock lock(g_Mutex);
    g_Args.clear();
    g_Initialised = false;
}

} // namespace SAGE
