#pragma once

#include "Token.h"
#include "../Languages/LanguageDefinition.h"

#include <string>
#include <vector>

namespace SAGE::Scripting::LogCon {

class Lexer {
public:
    explicit Lexer(const LanguageDefinition* language);

    std::vector<Token> Tokenize(const std::string& source);

private:
    const LanguageDefinition* m_Language = nullptr;
    std::string m_Source;
    std::size_t m_Position = 0;
    int m_Line = 1;
    int m_Column = 1;

    void Reset(const std::string& source);
    [[nodiscard]] bool IsAtEnd() const;
    [[nodiscard]] char Peek(int offset = 0) const;
    char Advance();
    void SkipWhitespace();
    void SkipComment();

    Token ScanToken();
    Token ScanNumber(int line, int column);
    Token ScanString(int line, int column);
    Token ScanIdentifierOrKeyword(int line, int column);

    Token MakeToken(TokenID id, std::string lexeme, int line, int column) const;
    Token MakeLiteralToken(TokenID id, std::string lexeme, double numberValue, int line, int column) const;
    Token MakeStringToken(std::string lexeme, std::string value, int line, int column) const;
    Token MakeInvalidToken(std::string lexeme, int line, int column) const;
};

} // namespace SAGE::Scripting::LogCon
