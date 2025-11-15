#include "Lexer.h"

#include <algorithm>
#include <cctype>

namespace SAGE::Scripting::LogCon {

namespace {
bool IsAlpha(char ch) {
    return static_cast<unsigned char>(std::isalpha(static_cast<unsigned char>(ch))) || ch == '_' || static_cast<unsigned char>(ch) >= 0x80;
}

bool IsAlphaNumeric(char ch) {
    return IsAlpha(ch) || static_cast<unsigned char>(std::isdigit(static_cast<unsigned char>(ch)));
}

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

} // namespace

Lexer::Lexer(const LanguageDefinition* language)
    : m_Language(language) {}

std::vector<Token> Lexer::Tokenize(const std::string& source) {
    Reset(source);
    std::vector<Token> tokens;
    if (!m_Language) {
        tokens.push_back(MakeInvalidToken("<no-language>", m_Line, m_Column));
        return tokens;
    }

    while (!IsAtEnd()) {
        SkipWhitespace();
        if (IsAtEnd()) {
            break;
        }

        if (Peek() == '/' && Peek(1) == '/') {
            SkipComment();
            continue;
        }

        Token token = ScanToken();
        tokens.push_back(token);
        if (token.id == TokenID::INVALID) {
            break;
        }
    }

    tokens.push_back(MakeToken(TokenID::END_OF_FILE, "", m_Line, m_Column));
    return tokens;
}

void Lexer::Reset(const std::string& source) {
    m_Source = source;
    m_Position = 0;
    m_Line = 1;
    m_Column = 1;
}

bool Lexer::IsAtEnd() const {
    return m_Position >= m_Source.size();
}

char Lexer::Peek(int offset) const {
    if (m_Position + offset >= m_Source.size()) {
        return '\0';
    }
    return m_Source[m_Position + offset];
}

char Lexer::Advance() {
    if (IsAtEnd()) {
        return '\0';
    }
    char ch = m_Source[m_Position++];
    if (ch == '\n') {
        m_Line++;
        m_Column = 1;
    } else {
        m_Column++;
    }
    return ch;
}

void Lexer::SkipWhitespace() {
    while (!IsAtEnd()) {
        char ch = Peek();
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            Advance();
        } else {
            break;
        }
    }
}

void Lexer::SkipComment() {
    while (!IsAtEnd() && Peek() != '\n') {
        Advance();
    }
}

Token Lexer::ScanToken() {
    int tokenLine = m_Line;
    int tokenColumn = m_Column;
    char ch = Advance();
    switch (ch) {
    case '{': return MakeToken(TokenID::LEFT_BRACE, "{", tokenLine, tokenColumn);
    case '}': return MakeToken(TokenID::RIGHT_BRACE, "}", tokenLine, tokenColumn);
    case '(': return MakeToken(TokenID::LEFT_PAREN, "(", tokenLine, tokenColumn);
    case ')': return MakeToken(TokenID::RIGHT_PAREN, ")", tokenLine, tokenColumn);
    case '[': return MakeToken(TokenID::LEFT_BRACKET, "[", tokenLine, tokenColumn);
    case ']': return MakeToken(TokenID::RIGHT_BRACKET, "]", tokenLine, tokenColumn);
    case ',': return MakeToken(TokenID::COMMA, ",", tokenLine, tokenColumn);
    case '.': return MakeToken(TokenID::DOT, ".", tokenLine, tokenColumn);
    case ':': return MakeToken(TokenID::COLON, ":", tokenLine, tokenColumn);
    case ';': return MakeToken(TokenID::SEMICOLON, ";", tokenLine, tokenColumn);
    case '\n': return MakeToken(TokenID::NEWLINE, "\n", tokenLine, tokenColumn);
    case '+': return MakeToken(TokenID::PLUS, "+", tokenLine, tokenColumn);
    case '-': return MakeToken(TokenID::MINUS, "-", tokenLine, tokenColumn);
    case '*': return MakeToken(TokenID::STAR, "*", tokenLine, tokenColumn);
    case '%': return MakeToken(TokenID::MODULO, "%", tokenLine, tokenColumn);
    case '!':
        if (Peek() == '=') {
            Advance();
            return MakeToken(TokenID::BANG_EQUAL, "!=", tokenLine, tokenColumn);
        }
        return MakeToken(TokenID::NOT, "!", tokenLine, tokenColumn);
    case '=':
        if (Peek() == '=') {
            Advance();
            return MakeToken(TokenID::EQUAL_EQUAL, "==", tokenLine, tokenColumn);
        }
        return MakeToken(TokenID::ASSIGN, "=", tokenLine, tokenColumn);
    case '<':
        if (Peek() == '=') {
            Advance();
            return MakeToken(TokenID::LESS_EQUAL, "<=", tokenLine, tokenColumn);
        }
        return MakeToken(TokenID::LESS, "<", tokenLine, tokenColumn);
    case '>':
        if (Peek() == '=') {
            Advance();
            return MakeToken(TokenID::GREATER_EQUAL, ">=", tokenLine, tokenColumn);
        }
        return MakeToken(TokenID::GREATER, ">", tokenLine, tokenColumn);
    case '/':
        return MakeToken(TokenID::SLASH, "/", tokenLine, tokenColumn);
    case '"':
        return ScanString(tokenLine, tokenColumn);
    default:
        break;
    }

    if (std::isdigit(static_cast<unsigned char>(ch))) {
        m_Position--;
        m_Column--;
        return ScanNumber(tokenLine, tokenColumn);
    }

    if (IsAlpha(ch)) {
        m_Position--;
        m_Column--;
        return ScanIdentifierOrKeyword(tokenLine, tokenColumn);
    }

    return MakeInvalidToken(std::string(1, ch), tokenLine, tokenColumn);
}

Token Lexer::ScanNumber(int line, int column) {
    std::size_t start = m_Position;
    while (std::isdigit(static_cast<unsigned char>(Peek()))) {
        Advance();
    }

    if (Peek() == '.' && std::isdigit(static_cast<unsigned char>(Peek(1)))) {
        Advance(); // consume '.'
        while (std::isdigit(static_cast<unsigned char>(Peek()))) {
            Advance();
        }
    }

    std::size_t end = m_Position;
    std::string lexeme = m_Source.substr(start, end - start);
    double value = std::stod(lexeme);
    return MakeLiteralToken(TokenID::NUMBER_LITERAL, lexeme, value, line, column);
}

Token Lexer::ScanString(int line, int column) {
    std::size_t start = m_Position; // after opening quote
    while (!IsAtEnd() && Peek() != '"') {
        Advance();
    }

    if (IsAtEnd()) {
        return MakeInvalidToken(m_Source.substr(start - 1, m_Position - start + 1), line, column);
    }

    Advance(); // consume closing quote

    std::size_t end = m_Position;
    std::string lexeme = m_Source.substr(start - 1, end - (start - 1));
    std::string value = m_Source.substr(start, (end - 1) - start);
    return MakeStringToken(lexeme, value, line, column);
}

Token Lexer::ScanIdentifierOrKeyword(int line, int column) {
    std::size_t start = m_Position;
    while (IsAlphaNumeric(Peek())) {
        Advance();
    }

    std::size_t end = m_Position;
    std::string lexeme = m_Source.substr(start, end - start);
    std::string lookup = ToLower(lexeme);

    if (auto tokenId = m_Language->GetTokenID(lookup)) {
    return MakeToken(*tokenId, lexeme, line, column);
    }

    // Allow multi-word keywords using underscore mapping (e.g. "else if")
    return MakeToken(TokenID::IDENTIFIER, lexeme, line, column);
}

Token Lexer::MakeToken(TokenID id, std::string lexeme, int line, int column) const {
    Token token;
    token.id = id;
    token.lexeme = std::move(lexeme);
    token.line = line;
    token.column = column;
    return token;
}

Token Lexer::MakeLiteralToken(TokenID id, std::string lexeme, double numberValue, int line, int column) const {
    Token token = MakeToken(id, std::move(lexeme), line, column);
    token.numberValue = numberValue;
    return token;
}

Token Lexer::MakeStringToken(std::string lexeme, std::string value, int line, int column) const {
    Token token = MakeToken(TokenID::STRING_LITERAL, std::move(lexeme), line, column);
    token.stringValue = std::move(value);
    return token;
}

Token Lexer::MakeInvalidToken(std::string lexeme, int line, int column) const {
    Token token = MakeToken(TokenID::INVALID, std::move(lexeme), line, column);
    return token;
}

} // namespace SAGE::Scripting::LogCon
