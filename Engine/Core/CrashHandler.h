#pragma once

#include "Core/Logger.h"
#include "Core/Version.h"
#include <string>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace SAGE {

/// @brief Crash handler for production builds
/// Captures unhandled exceptions, writes crash dumps, and logs stack traces
class CrashHandler {
public:
    using CrashCallback = std::function<void(const std::string& reason)>;

    /// @brief Install crash handler
    /// @param callback Optional callback to execute on crash
    static void Install(CrashCallback callback = nullptr) {
        s_CrashCallback = callback;

#ifdef _WIN32
        SetUnhandledExceptionFilter(WindowsExceptionHandler);
        SAGE_INFO("Crash handler installed (Windows)");
#else
        // TODO: Linux/macOS signal handlers
        SAGE_INFO("Crash handler not yet implemented for this platform");
#endif
    }

    /// @brief Uninstall crash handler
    static void Uninstall() {
#ifdef _WIN32
        SetUnhandledExceptionFilter(nullptr);
#endif
        s_CrashCallback = nullptr;
        SAGE_INFO("Crash handler uninstalled");
    }

    /// @brief Manually trigger crash dump (for testing)
    static void WriteDump(const std::string& reason = "Manual trigger") {
        SAGE_ERROR("Creating crash dump: {}", reason);
        
#ifdef _WIN32
        WriteMiniDump(nullptr);
#endif

        if (s_CrashCallback) {
            s_CrashCallback(reason);
        }
    }

private:
#ifdef _WIN32
    static LONG WINAPI WindowsExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
        SAGE_ERROR("=== FATAL ERROR: Unhandled Exception ===");
        SAGE_ERROR("Exception Code: 0x{:X}", exceptionInfo->ExceptionRecord->ExceptionCode);
        SAGE_ERROR("Exception Address: 0x{:p}", exceptionInfo->ExceptionRecord->ExceptionAddress);

        // Write minidump
        bool dumpWritten = WriteMiniDump(exceptionInfo);
        
        if (dumpWritten) {
            SAGE_ERROR("Crash dump written successfully");
        } else {
            SAGE_ERROR("Failed to write crash dump");
        }

        // Call user callback
        if (s_CrashCallback) {
            s_CrashCallback("Unhandled exception");
        }

        // Show message box to user
        std::string message = "SAGE Engine has encountered a fatal error.\n\n";
        message += "Exception Code: 0x" + std::to_string(exceptionInfo->ExceptionRecord->ExceptionCode) + "\n";
        message += "A crash dump has been created in the engine directory.\n\n";
        message += "Please report this issue to the developers.";
        
        MessageBoxA(nullptr, message.c_str(), "SAGE Engine - Fatal Error", MB_OK | MB_ICONERROR);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static bool WriteMiniDump(EXCEPTION_POINTERS* exceptionInfo) {
        // Generate unique filename
        char filename[256];
        std::time_t now = std::time(nullptr);
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);
        std::strftime(filename, sizeof(filename), "sage_crash_%Y%m%d_%H%M%S.dmp", &timeInfo);

        // Create minidump file
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            SAGE_ERROR("Failed to create dump file: {}", filename);
            return false;
        }

        MINIDUMP_EXCEPTION_INFORMATION exInfo;
        exInfo.ThreadId = GetCurrentThreadId();
        exInfo.ExceptionPointers = exceptionInfo;
        exInfo.ClientPointers = FALSE;

        // Write the dump
        BOOL success = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpNormal,
            exceptionInfo ? &exInfo : nullptr,
            nullptr,
            nullptr
        );

        CloseHandle(hFile);

        if (success) {
            SAGE_INFO("Minidump written to: {}", filename);
            return true;
        } else {
            SAGE_ERROR("MiniDumpWriteDump failed. Error: {}", GetLastError());
            return false;
        }
    }
#endif

    static CrashCallback s_CrashCallback;
};

// Static member initialization
inline CrashHandler::CrashCallback CrashHandler::s_CrashCallback = nullptr;

} // namespace SAGE
