#include "ScriptCompiler.h"

#include "Core/Lexer.h"
#include "Core/Parser.h"
#include "Languages/LanguageDefinition.h"
#include "Runtime/FunctionRegistry.h"
#include <Modding/ModScriptBindings.h>

#include <Core/Logger.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace SAGE::Scripting::LogCon {

namespace {
std::string ReadFile(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

const LanguageDefinition* DetectLanguageFromPath(const std::filesystem::path& path) {
    const auto& registry = LanguageRegistry::Get();

    // Files can be named like Player.ru.logcon, Player.en.logcon etc.
    if (path.extension() == ".logcon") {
        std::filesystem::path stem = path.stem();
        if (!stem.extension().empty()) {
            std::string code = stem.extension().string();
            if (!code.empty() && code.front() == '.') {
                code.erase(code.begin());
            }
            if (const auto* lang = registry.GetLanguage(code)) {
                return lang;
            }
        }
    }

    return nullptr;
}

} // namespace

bool ScriptCompiler::CompileScript(const std::string& scriptPath) {
    // Initialize language and function systems (idempotent)
    RegisterBuiltinLanguages();
    Runtime::RegisterBuiltinFunctions();
    Modding::RegisterLogConFunctions();

    const std::string source = ReadFile(scriptPath);
    if (source.empty()) {
        SAGE_ERROR("LogCon: Unable to read script '{}'", scriptPath);
        m_LastScript.reset();
        return false;
    }

    std::filesystem::path fsPath(scriptPath);
    const auto& registry = LanguageRegistry::Get();
    const LanguageDefinition* language = DetectLanguageFromPath(fsPath);

    if (!language) {
        language = registry.DetectLanguage(source);
    }

    if (!language) {
        SAGE_ERROR("LogCon: Failed to detect language for '{}'", scriptPath);
        m_LastScript.reset();
        return false;
    }

    Lexer lexer(language);
    std::vector<Token> tokens = lexer.Tokenize(source);
    if (tokens.empty()) {
        SAGE_ERROR("LogCon: Tokenization failed for '{}'", scriptPath);
        m_LastScript.reset();
        return false;
    }

    if (tokens.back().id != TokenID::END_OF_FILE) {
        SAGE_ERROR("LogCon: Script '{}' did not terminate correctly", scriptPath);
        m_LastScript.reset();
        return false;
    }

    for (const auto& token : tokens) {
        if (token.id == TokenID::INVALID) {
            SAGE_ERROR("LogCon: Invalid token '{}' at line {} column {}", token.lexeme, token.line, token.column);
            m_LastScript.reset();
            return false;
        }
    }

    Parser parser(*language, tokens);
    ParseResult parseResult = parser.Parse();
    if (!parseResult.succeeded) {
        for (const auto& error : parseResult.errors) {
            SAGE_ERROR("LogCon: {}", error);
        }
        SAGE_ERROR("LogCon: Parsing failed for '{}'", scriptPath);
        m_LastScript.reset();
        return false;
    }

    m_LastScript = std::make_shared<AST::Script>(std::move(parseResult.script));

    std::size_t entityCount = m_LastScript ? m_LastScript->entities.size() : 0;
    SAGE_INFO("LogCon: Script '{}' parsed successfully ({} сущностей)", scriptPath, entityCount);
    return true;
}

const AST::Script* ScriptCompiler::GetLastScript() const {
    return m_LastScript.get();
}

const AST::Entity* ScriptCompiler::FindEntity(const std::string& name) const {
    if (!m_LastScript) {
        return nullptr;
    }

    for (const auto& entity : m_LastScript->entities) {
        if (entity.name == name) {
            return &entity;
        }
    }

    return nullptr;
}

std::shared_ptr<const AST::Script> ScriptCompiler::GetScriptShared() const {
    return m_LastScript;
}

} // namespace SAGE::Scripting::LogCon
