#include "TestFramework.h"

#include <Scripting/LogCon/ScriptCompiler.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path WriteTempScript(const std::string& filename, const std::string& content) {
    const auto tempRoot = std::filesystem::temp_directory_path() / "sage_logcon_tests";
    std::filesystem::create_directories(tempRoot);

    const auto scriptPath = tempRoot / filename;
    std::ofstream file(scriptPath, std::ios::binary | std::ios::trunc);
    file << content;
    return scriptPath;
}

} // namespace

TEST_CASE(LogCon_ScriptCompiler_ParsesRussianScript) {
    const std::string script = R"(сущность Игрок {
    здоровье = 100
    при создании {
        вывести("Игрок создан")
    }
}
)";

    const auto path = WriteTempScript("Player.ru.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    CHECK(compiler.CompileScript(path.string()));
}

TEST_CASE(LogCon_ScriptCompiler_ParsesEnglishScript) {
    const std::string script = R"(entity Player {
    health = 100
    on create {
        print("Player spawned")
    }
}
)";

    const auto path = WriteTempScript("Player.en.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    CHECK(compiler.CompileScript(path.string()));
}

TEST_CASE(LogCon_ScriptCompiler_DetectsUnknownLanguage) {
    const std::string script = R"(unknown token)";

    const auto path = WriteTempScript("Player.xx.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    CHECK_FALSE(compiler.CompileScript(path.string()));
}
