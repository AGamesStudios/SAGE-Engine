#pragma once

#include "AST.h"
#include "Token.h"
#include "TokenID.h"
#include "../Languages/LanguageDefinition.h"

#include <optional>
#include <string>
#include <vector>

namespace SAGE::Scripting::LogCon {

struct ParseResult {
    bool succeeded = false;
    AST::Script script;
    std::vector<std::string> errors;
};

class Parser {
public:
    Parser(const LanguageDefinition& language, const std::vector<Token>& tokens);

    ParseResult Parse();

private:
    using EventType = AST::EventBlock::Type;

    [[nodiscard]] bool IsAtEnd() const;
    [[nodiscard]] const Token& Peek() const;
    [[nodiscard]] const Token& Previous() const;
    [[nodiscard]] const Token& PeekNext(std::size_t offset = 1) const;
    const Token& Advance();
    bool Check(TokenID id) const;
    bool Match(TokenID id);
    bool Match(TokenID first, TokenID second);

    void SkipNewlines();
    void Synchronize();
    void SynchronizeToNextStatement();
    void ReportError(const Token& token, const std::string& message);

    [[nodiscard]] std::optional<AST::Entity> ParseEntity();
    [[nodiscard]] std::optional<AST::EventBlock> ParseEventBlock();
    [[nodiscard]] std::optional<AST::Statement> ParseStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseIfStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseWhileStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseForStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseReturnStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseBlockStatement();
    [[nodiscard]] std::optional<AST::Statement> ParseFunctionDefinition();
    [[nodiscard]] std::optional<AST::Statement> ParseVariableDeclaration(TokenID scopeToken);
    [[nodiscard]] std::optional<AST::Statement> ParseAssignment(const Token& identifier);
    [[nodiscard]] std::optional<AST::Statement> ParseFunctionCall(const Token& identifier);
    [[nodiscard]] std::optional<AST::Statement> ParseFunctionCallFromKeyword(const Token& keywordToken);
    [[nodiscard]] std::optional<AST::Statement> ParseFunctionCallWithOrigin(const Token& originToken);
    [[nodiscard]] std::optional<AST::Statement> ParseTriggerEventStatement(const Token& triggerToken);

    [[nodiscard]] AST::ExpressionPtr ParseExpression();
    [[nodiscard]] AST::ExpressionPtr ParseEquality();
    [[nodiscard]] AST::ExpressionPtr ParseComparison();
    [[nodiscard]] AST::ExpressionPtr ParseTerm();
    [[nodiscard]] AST::ExpressionPtr ParseFactor();
    [[nodiscard]] AST::ExpressionPtr ParseUnary();
    [[nodiscard]] AST::ExpressionPtr ParsePostfix();
    [[nodiscard]] AST::ExpressionPtr ParsePrimary();
    [[nodiscard]] AST::ExpressionPtr ParseCallExpression(const Token& originToken);

    [[nodiscard]] std::vector<AST::ExpressionPtr> ParseArgumentList();
    [[nodiscard]] std::vector<std::string> ParseParameterList();
    [[nodiscard]] std::vector<AST::Statement> ParseBlockContents();
    [[nodiscard]] bool Expect(TokenID id, const std::string& message);
    [[nodiscard]] std::optional<Token> ExpectIdentifier(const std::string& message);
    void ExpectStatementTerminator();

    [[nodiscard]] static EventType DetectEventType(TokenID id);
    [[nodiscard]] static bool IsCallableToken(TokenID id);
    [[nodiscard]] static bool IsDirectionToken(TokenID id);
    [[nodiscard]] std::string BuildFunctionName(const Token& firstToken);

    const LanguageDefinition& m_Language;
    const std::vector<Token>& m_Tokens;
    ParseResult m_Result;
    size_t m_Current = 0;
};

} // namespace SAGE::Scripting::LogCon
