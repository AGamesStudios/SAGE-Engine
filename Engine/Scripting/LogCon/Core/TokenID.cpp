#include "TokenID.h"

namespace SAGE::Scripting::LogCon {

const char* TokenIDToString(TokenID id) {
    switch (id) {
    case TokenID::ENTITY: return "ENTITY";
    case TokenID::SYSTEM: return "SYSTEM";
    case TokenID::FUNCTION: return "FUNCTION";
    case TokenID::ON: return "ON";
    case TokenID::CREATE: return "CREATE";
    case TokenID::UPDATE: return "UPDATE";
    case TokenID::DESTROY: return "DESTROY";
    case TokenID::COLLISION: return "COLLISION";
    case TokenID::EVENT: return "EVENT";
    case TokenID::KEY_PRESS: return "KEY_PRESS";
    case TokenID::IF: return "IF";
    case TokenID::ELSE: return "ELSE";
    case TokenID::ELSE_IF: return "ELSE_IF";
    case TokenID::WHILE: return "WHILE";
    case TokenID::FOR: return "FOR";
    case TokenID::REPEAT: return "REPEAT";
    case TokenID::BREAK: return "BREAK";
    case TokenID::CONTINUE: return "CONTINUE";
    case TokenID::RETURN: return "RETURN";
    case TokenID::VAR_KEYWORD: return "VAR";
    case TokenID::GLOBAL_KEYWORD: return "GLOBAL";
    case TokenID::LET_KEYWORD: return "LET";
    case TokenID::CONST_KEYWORD: return "CONST";
    case TokenID::TRIGGER_KEYWORD: return "TRIGGER";
    case TokenID::EMIT_KEYWORD: return "EMIT";
    case TokenID::NUMBER_TYPE: return "NUMBER_TYPE";
    case TokenID::TEXT_TYPE: return "TEXT_TYPE";
    case TokenID::BOOLEAN_TYPE: return "BOOLEAN_TYPE";
    case TokenID::LIST_TYPE: return "LIST_TYPE";
    case TokenID::OBJECT_TYPE: return "OBJECT_TYPE";
    case TokenID::TRUE_LITERAL: return "TRUE";
    case TokenID::FALSE_LITERAL: return "FALSE";
    case TokenID::NULL_LITERAL: return "NULL";
    case TokenID::ASSIGN: return "ASSIGN";
    case TokenID::PLUS: return "PLUS";
    case TokenID::MINUS: return "MINUS";
    case TokenID::STAR: return "STAR";
    case TokenID::SLASH: return "SLASH";
    case TokenID::MODULO: return "MODULO";
    case TokenID::EQUAL_EQUAL: return "EQUAL_EQUAL";
    case TokenID::BANG_EQUAL: return "BANG_EQUAL";
    case TokenID::LESS: return "LESS";
    case TokenID::GREATER: return "GREATER";
    case TokenID::LESS_EQUAL: return "LESS_EQUAL";
    case TokenID::GREATER_EQUAL: return "GREATER_EQUAL";
    case TokenID::AND: return "AND";
    case TokenID::OR: return "OR";
    case TokenID::NOT: return "NOT";
    case TokenID::PRINT: return "PRINT";
    case TokenID::MOVE: return "MOVE";
    case TokenID::MOVE_UP: return "MOVE_UP";
    case TokenID::MOVE_DOWN: return "MOVE_DOWN";
    case TokenID::MOVE_LEFT: return "MOVE_LEFT";
    case TokenID::MOVE_RIGHT: return "MOVE_RIGHT";
    case TokenID::TELEPORT: return "TELEPORT";
    case TokenID::CREATE_OBJECT: return "CREATE_OBJECT";
    case TokenID::DESTROY_OBJECT: return "DESTROY_OBJECT";
    case TokenID::FIND: return "FIND";
    case TokenID::WAIT: return "WAIT";
    case TokenID::RANDOM: return "RANDOM";
    case TokenID::IDENTIFIER: return "IDENTIFIER";
    case TokenID::NUMBER_LITERAL: return "NUMBER_LITERAL";
    case TokenID::STRING_LITERAL: return "STRING_LITERAL";
    case TokenID::LEFT_BRACE: return "LEFT_BRACE";
    case TokenID::RIGHT_BRACE: return "RIGHT_BRACE";
    case TokenID::LEFT_PAREN: return "LEFT_PAREN";
    case TokenID::RIGHT_PAREN: return "RIGHT_PAREN";
    case TokenID::LEFT_BRACKET: return "LEFT_BRACKET";
    case TokenID::RIGHT_BRACKET: return "RIGHT_BRACKET";
    case TokenID::COMMA: return "COMMA";
    case TokenID::DOT: return "DOT";
    case TokenID::COLON: return "COLON";
    case TokenID::SEMICOLON: return "SEMICOLON";
    case TokenID::NEWLINE: return "NEWLINE";
    case TokenID::END_OF_FILE: return "EOF";
    case TokenID::INVALID: return "INVALID";
    default: return "UNKNOWN";
    }
}

} // namespace SAGE::Scripting::LogCon
