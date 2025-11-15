#include "LogConRuntime.h"

#include "Core/Logger.h"

#include <fstream>

namespace SAGE::Scripting::LogCon {

LogConRuntime::LogConRuntime(std::string scriptPath)
    : m_ScriptPath(std::move(scriptPath)) {}

int LogConRuntime::Run() {
    SAGE_INFO("LogCon runtime prototype started");

    std::ifstream file(m_ScriptPath);
    if (!file.is_open()) {
        SAGE_ERROR("Unable to open LogCon script: {}", m_ScriptPath);
        return 1;
    }

    SAGE_WARN("LogCon execution is not implemented yet. Script content will be ignored.");
    return 0;
}

} // namespace SAGE::Scripting::LogCon
