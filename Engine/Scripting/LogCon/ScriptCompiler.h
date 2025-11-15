#pragma once

#include "Core/AST.h"

#include <memory>
#include <optional>
#include <string>

namespace SAGE::Scripting::LogCon {

class ScriptCompiler {
public:
    bool CompileScript(const std::string& scriptPath);

    [[nodiscard]] const AST::Script* GetLastScript() const;
    [[nodiscard]] const AST::Entity* FindEntity(const std::string& name) const;
    [[nodiscard]] std::shared_ptr<const AST::Script> GetScriptShared() const;

private:
    std::shared_ptr<AST::Script> m_LastScript;
};

} // namespace SAGE::Scripting::LogCon
