#include "TestFramework.h"

#include "logcon/bytecode.hpp"
#include "logcon/compiler.hpp"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] std::string ToString(double value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

logcon::ASTNode MakeNumberLiteral(double value) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::LiteralNumber;
    node.identifier = ToString(value);
    return node;
}

logcon::ASTNode MakeStringLiteral(std::string value) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::LiteralString;
    node.identifier = std::move(value);
    return node;
}

logcon::ASTNode MakeBooleanLiteral(bool value) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::LiteralBoolean;
    node.identifier = value ? "true" : "false";
    return node;
}

logcon::ASTNode MakeIdentifier(std::string name) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::Identifier;
    node.identifier = std::move(name);
    return node;
}

logcon::ASTNode MakeBinaryOperation(std::string op,
                                    logcon::ASTNode lhs,
                                    logcon::ASTNode rhs) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::BinaryOperation;
    node.identifier = std::move(op);
    node.children.push_back(std::move(lhs));
    node.children.push_back(std::move(rhs));
    return node;
}

logcon::ASTNode MakeVariable(std::string name, logcon::ASTNode initializer) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::Variable;
    node.identifier = std::move(name);
    node.children.push_back(std::move(initializer));
    return node;
}

logcon::ASTNode MakeFunctionCall(std::string name, std::vector<logcon::ASTNode> arguments = {}) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::FunctionCall;
    node.identifier = std::move(name);
    node.children = std::move(arguments);
    return node;
}

logcon::ASTNode MakeWhileLoop(logcon::ASTNode condition,
                              std::vector<logcon::ASTNode> body) {
    logcon::ASTNode node;
    node.type = logcon::ASTNode::Type::WhileLoop;
    node.children.push_back(std::move(condition));
    for (auto& stmt : body) {
        node.children.push_back(std::move(stmt));
    }
    return node;
}

} // namespace

TEST_CASE(LogCon_Compiler_FailsOnUnknownRoot) {
    auto compiler = logcon::Compiler::Create();
    logcon::ASTNode root;

    const auto result = compiler->compile(root);

    CHECK_FALSE(result.succeeded());
    CHECK_FALSE(result.errors.empty());
}

TEST_CASE(LogCon_Compiler_CompilesNumericVariable) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;
    root.identifier = "root";
    root.children.push_back(MakeVariable("здоровье", MakeNumberLiteral(100.0)));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());
    CHECK(result.errors.empty());
    CHECK(result.warnings.empty());

    CHECK(result.chunk.constants.size() == 1);
    CHECK(result.chunk.constants.front().is_number());
    CHECK(result.chunk.constants.front().as_number() == 100.0);

    CHECK(result.chunk.variables.contains("здоровье"));

    CHECK(result.chunk.code.size() >= 6);
    CHECK(result.chunk.code[0] == static_cast<std::uint8_t>(logcon::OpCode::OP_LOAD_CONST));
    CHECK(result.chunk.code[3] == static_cast<std::uint8_t>(logcon::OpCode::OP_STORE_VAR));
}

TEST_CASE(LogCon_Compiler_CompilesStringVariable) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;
    root.children.push_back(MakeVariable("имя", MakeStringLiteral("Игрок")));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());
    CHECK(result.chunk.constants.size() == 1);
    CHECK(result.chunk.constants.front().is_string());
    CHECK(result.chunk.constants.front().as_string() == "Игрок");
    CHECK(result.chunk.variables.contains("имя"));
}

TEST_CASE(LogCon_Compiler_CompilesBooleanVariable) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;
    root.children.push_back(MakeVariable("isActive", MakeBooleanLiteral(true)));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());
    CHECK(result.chunk.constants.size() == 1);
    CHECK(result.chunk.constants.front().is_boolean());
    CHECK(result.chunk.constants.front().as_boolean());
    CHECK(result.chunk.variables.contains("isActive"));
}

TEST_CASE(LogCon_Compiler_CompilesBinaryAddition) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;
    root.children.push_back(MakeVariable(
        "здоровье",
        MakeBinaryOperation("+", MakeNumberLiteral(100.0), MakeNumberLiteral(20.0))));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());
    CHECK(result.chunk.code.size() >= 9);

    const auto it = std::find(result.chunk.code.begin(), result.chunk.code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_ADD));
    CHECK(it != result.chunk.code.end());
}

TEST_CASE(LogCon_Compiler_EmitsConditionalTriggerJump) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;

    logcon::ASTNode trigger;
    trigger.type = logcon::ASTNode::Type::Trigger;
    trigger.children.push_back(MakeBinaryOperation("<",
        MakeIdentifier("здоровье"), MakeNumberLiteral(50.0)));
    trigger.children.push_back(MakeFunctionCall("показать_предупреждение"));

    root.children.push_back(std::move(trigger));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());

    const auto& code = result.chunk.code;
    const auto jumpIfFalse = std::find(code.begin(), code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_JUMP_IF_FALSE));
    CHECK(jumpIfFalse != code.end());
}

TEST_CASE(LogCon_Compiler_CompilesIfElseStatement) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;

    logcon::ASTNode ifNode;
    ifNode.type = logcon::ASTNode::Type::IfStatement;
    ifNode.children.push_back(MakeIdentifier("имеет_ключ"));
    ifNode.children.push_back(MakeFunctionCall("открыть_дверь"));

    logcon::ASTNode elseClause;
    elseClause.type = logcon::ASTNode::Type::ElseClause;
    elseClause.children.push_back(MakeFunctionCall("показать_сообщение"));
    ifNode.children.push_back(std::move(elseClause));

    root.children.push_back(std::move(ifNode));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());

    const auto& code = result.chunk.code;
    const auto conditionalCount = std::count(code.begin(), code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_JUMP_IF_FALSE));
    CHECK(conditionalCount >= 1);

    const auto unconditionalCount = std::count(code.begin(), code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_JUMP));
    CHECK(unconditionalCount >= 1);
}

TEST_CASE(LogCon_Compiler_CompilesWhileLoop) {
    auto compiler = logcon::Compiler::Create();

    logcon::ASTNode root;
    root.children.push_back(MakeWhileLoop(
        MakeBinaryOperation(">",
            MakeIdentifier("таймер"),
            MakeNumberLiteral(0.0)),
        { MakeFunctionCall("tick") }));

    const auto result = compiler->compile(root);

    CHECK(result.succeeded());

    const auto& code = result.chunk.code;
    const auto condJumpCount = std::count(code.begin(), code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_JUMP_IF_FALSE));
    CHECK(condJumpCount >= 1);

    const auto backJumpCount = std::count(code.begin(), code.end(),
        static_cast<std::uint8_t>(logcon::OpCode::OP_JUMP));
    CHECK(backJumpCount >= 1);
}
