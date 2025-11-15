#include "TestFramework.h"

#include <Scripting/LogCon/Runtime/Interpreter.h>
#include <Scripting/LogCon/ScriptCompiler.h>

#include <Core/GameObject.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path WriteTempScript(const std::string& filename, const std::string& content) {
    const auto tempRoot = std::filesystem::temp_directory_path() / "sage_logcon_runtime_tests";
    std::filesystem::create_directories(tempRoot);

    const auto scriptPath = tempRoot / filename;
    std::ofstream file(scriptPath, std::ios::binary | std::ios::trunc);
    file << content;
    return scriptPath;
}

} // namespace

TEST_CASE(LogCon_Runtime_InstantiatesGameObject) {
    const std::string script = R"(entity Runner {
    x = 5
    y = 10
    скорость = 2

    function увеличить_скорость(значение) {
        скорость = скорость + значение
    }

    on create {
        if (скорость > 1) {
            двигать вправо(скорость)
        }
    }

    on update {
        увеличить_скорость(1)
        двигать вверх(1)
    }
}
)";

    const auto path = WriteTempScript("Runner.en.logcon", script);

    SAGE::Scripting::LogCon::ScriptCompiler compiler;
    REQUIRE(compiler.CompileScript(path.string()));

    auto scriptPtr = compiler.GetScriptShared();
    REQUIRE(scriptPtr != nullptr);

    SAGE::Scripting::LogCon::Runtime::Interpreter interpreter;
    REQUIRE(interpreter.Instantiate(scriptPtr));

    // Trigger OnCreate + first OnUpdate
    SAGE::GameObject::UpdateAll(0.016f);

    SAGE::GameObject* object = SAGE::GameObject::Find("Runner");
    REQUIRE(object != nullptr);

    CHECK(object->x == Approx(7.0f));
    CHECK(object->y == Approx(9.0f));

    auto valueSpeed = interpreter.GetProperty(object, "скорость");
    REQUIRE(valueSpeed.has_value());
    CHECK(valueSpeed->AsNumber() == Approx(3.0));

    interpreter.Clear();
    SAGE::GameObject::DestroyAll();
}
