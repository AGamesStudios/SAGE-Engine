#include "TestFramework.h"

#include <Scripting/LogCon/ScriptCompiler.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path WriteTempScript(const std::string& filename, const std::string& content) {
    const auto tempRoot = std::filesystem::temp_directory_path() / "sage_logcon_parser_tests";
    std::filesystem::create_directories(tempRoot);

    const auto scriptPath = tempRoot / filename;
    std::ofstream file(scriptPath, std::ios::binary | std::ios::trunc);
    file << content;
    return scriptPath;
}

} // namespace

TEST_CASE(LogCon_Parser_BuildsEntityAST) {
    const std::string script = R"(сущность Игрок {
    здоровье = 100
    сила = здоровье * 2

    при создании {
        вывести("Игрок создан")
    }

    при обновлении {
        вывести("Игрок обновлён")
    }
}
)";

    const auto path = WriteTempScript("Player.ru.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    CHECK(compiler.CompileScript(path.string()));

    const auto* scriptAst = compiler.GetLastScript();
    REQUIRE(scriptAst != nullptr);
    REQUIRE(scriptAst->entities.size() == 1);

    const auto* entity = compiler.FindEntity("Игрок");
    REQUIRE(entity != nullptr);
    CHECK(entity->properties.size() == 2);
    CHECK(entity->events.size() == 2);

    const auto& createEvent = entity->events.front();
    CHECK(createEvent.type == SAGE::Scripting::LogCon::AST::EventBlock::Type::OnCreate);
    CHECK(createEvent.statements.size() == 1);
    CHECK(createEvent.statements.front().kind == SAGE::Scripting::LogCon::AST::Statement::Kind::FunctionCall);
}

TEST_CASE(LogCon_Parser_ReportsSyntaxErrors) {
    const std::string script = R"(entity Player {
    health =
    on create {
        print("Player spawned")
    }
}
)";

    const auto path = WriteTempScript("Broken.en.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    CHECK_FALSE(compiler.CompileScript(path.string()));
    CHECK(compiler.GetLastScript() == nullptr);
}
