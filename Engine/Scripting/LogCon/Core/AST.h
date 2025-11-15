#pragma once

#include "TokenID.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SAGE::Scripting::LogCon::AST {

struct Expression;
using ExpressionPtr = std::shared_ptr<Expression>;

struct Expression {
    enum class Kind {
        Identifier,
        NumberLiteral,
        StringLiteral,
        BooleanLiteral,
        ArrayLiteral,
        ArrayAccess,
        Binary,
        Unary,
        Call
    };

    Kind kind = Kind::Identifier;

    std::string identifier;   // имя переменной или функции
    std::string stringValue;  // строковые литералы
    double numberValue = 0.0; // числовые литералы
    bool boolValue = false;   // булевы литералы

    TokenID binaryOperator = TokenID::INVALID;
    TokenID unaryOperator = TokenID::INVALID;
    ExpressionPtr left;
    ExpressionPtr right;
    ExpressionPtr operand;

    std::string callName;
    std::vector<ExpressionPtr> callArguments;
    
    // Array support
    std::vector<ExpressionPtr> arrayElements;
    ExpressionPtr arrayTarget;
    ExpressionPtr arrayIndex;

    static ExpressionPtr MakeIdentifier(std::string name);
    static ExpressionPtr MakeString(std::string value);
    static ExpressionPtr MakeNumber(double value);
    static ExpressionPtr MakeBoolean(bool value);
    static ExpressionPtr MakeArray(std::vector<ExpressionPtr> elements);
    static ExpressionPtr MakeArrayAccess(ExpressionPtr target, ExpressionPtr index);
    static ExpressionPtr MakeBinary(TokenID op, ExpressionPtr lhs, ExpressionPtr rhs);
    static ExpressionPtr MakeUnary(TokenID op, ExpressionPtr value);
    static ExpressionPtr MakeCall(std::string name, std::vector<ExpressionPtr> arguments);
};

struct Statement {
    enum class Kind {
        Assignment,
        FunctionCall,
        Block,
        If,
        FunctionDefinition,
        VariableDeclaration,  // перем x = 5
        TriggerEvent,         // вызвать событие("имя")
        Return,               // вернуть значение
        While,                // пока condition { }
        For,                  // для i = 1 до 10 { }
        Break,                // прервать
        Continue              // продолжить
    };

    enum class VariableScope {
        Local,
        Global,
        Constant
    };

    struct AssignmentData {
        std::string variable;
        ExpressionPtr expression;
        ExpressionPtr targetExpression; // Для присваивания arr[i] = value
        ExpressionPtr indexExpression;   // Для присваивания arr[i] = value
        bool isArrayAccess = false;
    };

    struct FunctionCallData {
        std::string function;
        std::vector<ExpressionPtr> arguments;
    };

    struct BlockData {
        std::vector<Statement> statements;
    };

    struct IfData {
        ExpressionPtr condition;
        std::vector<Statement> thenBranch;
        std::vector<Statement> elseBranch;
    };

    struct FunctionDefinitionData {
        std::string name;
        std::vector<std::string> parameters;
        std::vector<Statement> body;
    };

    struct VariableDeclarationData {
        VariableScope scope = VariableScope::Local;
        std::string name;
        std::string typeName;  // опционально: "число", "текст", и т.д.
        ExpressionPtr initializer;
    };

    struct TriggerEventData {
        std::string eventName;
        std::vector<ExpressionPtr> arguments;
    };

    struct ReturnData {
        ExpressionPtr value;  // может быть nullptr для "return" без значения
    };

    struct WhileData {
        ExpressionPtr condition;
        std::vector<Statement> body;
    };

    struct ForData {
        std::string variable;     // счетчик цикла
        ExpressionPtr from;       // начальное значение
        ExpressionPtr to;         // конечное значение
        ExpressionPtr step;       // шаг (опционально, по умолчанию 1)
        std::vector<Statement> body;
    };

    Kind kind = Kind::Assignment;
    AssignmentData assignment;
    FunctionCallData functionCall;
    BlockData block;
    IfData ifStatement;
    FunctionDefinitionData functionDefinition;
    VariableDeclarationData variableDeclaration;
    TriggerEventData triggerEvent;
    ReturnData returnStatement;
    WhileData whileLoop;
    ForData forLoop;

    static Statement MakeAssignment(std::string variable, ExpressionPtr value);
    static Statement MakeArrayAssignment(ExpressionPtr target, ExpressionPtr index, ExpressionPtr value);
    static Statement MakeFunctionCall(std::string function, std::vector<ExpressionPtr> arguments);
    static Statement MakeBlock(std::vector<Statement> statements);
    static Statement MakeIf(ExpressionPtr condition, std::vector<Statement> thenBranch, std::vector<Statement> elseBranch);
    static Statement MakeFunctionDefinition(std::string name, std::vector<std::string> parameters, std::vector<Statement> body);
    static Statement MakeVariableDeclaration(VariableScope scope, std::string name, std::string typeName, ExpressionPtr initializer);
    static Statement MakeTriggerEvent(std::string eventName, std::vector<ExpressionPtr> arguments);
    static Statement MakeReturn(ExpressionPtr value);
    static Statement MakeWhile(ExpressionPtr condition, std::vector<Statement> body);
    static Statement MakeFor(std::string variable, ExpressionPtr from, ExpressionPtr to, ExpressionPtr step, std::vector<Statement> body);
    static Statement MakeBreak();
    static Statement MakeContinue();
};

struct EventBlock {
    enum class Type {
        OnCreate,
        OnUpdate,
        OnDestroy,
        OnCollision,
        OnEvent,      // при событии("custom_event")
        OnKeyPress,
        Custom,       // пользовательское событие
        Unknown
    };

    Type type = Type::Unknown;
    std::string eventName;     // имя пользовательского события (для Custom)
    std::string parameter;     // имя параметра события (если есть)
    std::vector<Statement> statements;
};

struct Entity {
    std::string name;
    std::vector<Statement> properties;
    std::vector<EventBlock> events;
    std::vector<Statement> functions;
};

struct Script {
    std::vector<Entity> entities;
};

inline ExpressionPtr Expression::MakeIdentifier(std::string name) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::Identifier;
    expr->identifier = std::move(name);
    return expr;
}

inline ExpressionPtr Expression::MakeString(std::string value) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::StringLiteral;
    expr->stringValue = std::move(value);
    return expr;
}

inline ExpressionPtr Expression::MakeNumber(double value) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::NumberLiteral;
    expr->numberValue = value;
    return expr;
}

inline ExpressionPtr Expression::MakeBoolean(bool value) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::BooleanLiteral;
    expr->boolValue = value;
    return expr;
}

inline ExpressionPtr Expression::MakeBinary(TokenID op, ExpressionPtr lhs, ExpressionPtr rhs) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::Binary;
    expr->binaryOperator = op;
    expr->left = std::move(lhs);
    expr->right = std::move(rhs);
    return expr;
}

inline ExpressionPtr Expression::MakeUnary(TokenID op, ExpressionPtr value) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::Unary;
    expr->unaryOperator = op;
    expr->operand = std::move(value);
    return expr;
}

inline ExpressionPtr Expression::MakeCall(std::string name, std::vector<ExpressionPtr> arguments) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::Call;
    expr->callName = std::move(name);
    expr->callArguments = std::move(arguments);
    return expr;
}

inline Statement Statement::MakeAssignment(std::string variable, ExpressionPtr value) {
    Statement stmt;
    stmt.kind = Kind::Assignment;
    stmt.assignment.variable = std::move(variable);
    stmt.assignment.expression = std::move(value);
    stmt.assignment.isArrayAccess = false;
    return stmt;
}

inline Statement Statement::MakeArrayAssignment(ExpressionPtr target, ExpressionPtr index, ExpressionPtr value) {
    Statement stmt;
    stmt.kind = Kind::Assignment;
    stmt.assignment.targetExpression = std::move(target);
    stmt.assignment.indexExpression = std::move(index);
    stmt.assignment.expression = std::move(value);
    stmt.assignment.isArrayAccess = true;
    return stmt;
}

inline Statement Statement::MakeFunctionCall(std::string function, std::vector<ExpressionPtr> arguments) {
    Statement stmt;
    stmt.kind = Kind::FunctionCall;
    stmt.functionCall.function = std::move(function);
    stmt.functionCall.arguments = std::move(arguments);
    return stmt;
}

inline Statement Statement::MakeBlock(std::vector<Statement> statements) {
    Statement stmt;
    stmt.kind = Kind::Block;
    stmt.block.statements = std::move(statements);
    return stmt;
}

inline Statement Statement::MakeIf(ExpressionPtr condition, std::vector<Statement> thenBranch, std::vector<Statement> elseBranch) {
    Statement stmt;
    stmt.kind = Kind::If;
    stmt.ifStatement.condition = std::move(condition);
    stmt.ifStatement.thenBranch = std::move(thenBranch);
    stmt.ifStatement.elseBranch = std::move(elseBranch);
    return stmt;
}

inline Statement Statement::MakeFunctionDefinition(std::string name, std::vector<std::string> parameters, std::vector<Statement> body) {
    Statement stmt;
    stmt.kind = Kind::FunctionDefinition;
    stmt.functionDefinition.name = std::move(name);
    stmt.functionDefinition.parameters = std::move(parameters);
    stmt.functionDefinition.body = std::move(body);
    return stmt;
}

inline Statement Statement::MakeVariableDeclaration(VariableScope scope, std::string name, std::string typeName, ExpressionPtr initializer) {
    Statement stmt;
    stmt.kind = Kind::VariableDeclaration;
    stmt.variableDeclaration.scope = scope;
    stmt.variableDeclaration.name = std::move(name);
    stmt.variableDeclaration.typeName = std::move(typeName);
    stmt.variableDeclaration.initializer = std::move(initializer);
    return stmt;
}

inline Statement Statement::MakeTriggerEvent(std::string eventName, std::vector<ExpressionPtr> arguments) {
    Statement stmt;
    stmt.kind = Kind::TriggerEvent;
    stmt.triggerEvent.eventName = std::move(eventName);
    stmt.triggerEvent.arguments = std::move(arguments);
    return stmt;
}

inline Statement Statement::MakeReturn(ExpressionPtr value) {
    Statement stmt;
    stmt.kind = Kind::Return;
    stmt.returnStatement.value = std::move(value);
    return stmt;
}

inline Statement Statement::MakeWhile(ExpressionPtr condition, std::vector<Statement> body) {
    Statement stmt;
    stmt.kind = Kind::While;
    stmt.whileLoop.condition = std::move(condition);
    stmt.whileLoop.body = std::move(body);
    return stmt;
}

inline Statement Statement::MakeFor(std::string variable, ExpressionPtr from, ExpressionPtr to, ExpressionPtr step, std::vector<Statement> body) {
    Statement stmt;
    stmt.kind = Kind::For;
    stmt.forLoop.variable = std::move(variable);
    stmt.forLoop.from = std::move(from);
    stmt.forLoop.to = std::move(to);
    stmt.forLoop.step = std::move(step);
    stmt.forLoop.body = std::move(body);
    return stmt;
}

inline Statement Statement::MakeBreak() {
    Statement stmt;
    stmt.kind = Kind::Break;
    return stmt;
}

inline Statement Statement::MakeContinue() {
    Statement stmt;
    stmt.kind = Kind::Continue;
    return stmt;
}

inline ExpressionPtr Expression::MakeArray(std::vector<ExpressionPtr> elements) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::ArrayLiteral;
    expr->arrayElements = std::move(elements);
    return expr;
}

inline ExpressionPtr Expression::MakeArrayAccess(ExpressionPtr target, ExpressionPtr index) {
    auto expr = std::make_shared<Expression>();
    expr->kind = Kind::ArrayAccess;
    expr->arrayTarget = std::move(target);
    expr->arrayIndex = std::move(index);
    return expr;
}

} // namespace SAGE::Scripting::LogCon::AST
