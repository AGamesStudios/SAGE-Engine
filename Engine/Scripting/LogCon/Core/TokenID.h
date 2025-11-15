#pragma once

#include <cstdint>

namespace SAGE::Scripting::LogCon {

enum class TokenID : std::uint16_t {
    // Structural
    ENTITY,
    SYSTEM,
    FUNCTION,

    // Events
    ON,
    CREATE,
    UPDATE,
    DESTROY,
    COLLISION,
    EVENT,
    KEY_PRESS,

    // Flow control
    IF,
    ELSE,
    ELSE_IF,
    WHILE,
    FOR,
    REPEAT,
    BREAK,
    CONTINUE,
    RETURN,

    // Variable scopes
    VAR_KEYWORD,
    GLOBAL_KEYWORD,
    LET_KEYWORD,
    CONST_KEYWORD,

    // Custom events
    TRIGGER_KEYWORD,
    EMIT_KEYWORD,

    // Types
    NUMBER_TYPE,
    TEXT_TYPE,
    BOOLEAN_TYPE,
    LIST_TYPE,
    OBJECT_TYPE,

    // Literals
    TRUE_LITERAL,
    FALSE_LITERAL,
    NULL_LITERAL,

    // Operators
    ASSIGN,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MODULO,
    EQUAL_EQUAL,
    BANG_EQUAL,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    AND,
    OR,
    NOT,

    // Builtins (subset for initial implementation)
    PRINT,
    MOVE,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    TELEPORT,
    CREATE_OBJECT,
    DESTROY_OBJECT,
    FIND,
    WAIT,
    RANDOM,

    // Symbols / punctuation
    IDENTIFIER,
    NUMBER_LITERAL,
    STRING_LITERAL,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    DOT,
    COLON,
    SEMICOLON,
    NEWLINE,
    END_OF_FILE,
    INVALID
};

const char* TokenIDToString(TokenID id);

} // namespace SAGE::Scripting::LogCon
