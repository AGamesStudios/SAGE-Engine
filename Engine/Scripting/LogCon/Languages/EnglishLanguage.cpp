#include "LanguageDefinition.h"

namespace SAGE::Scripting::LogCon {

LanguageDefinition CreateEnglishLanguage() {
    LanguageDefinition lang;
    lang.name = "English";
    lang.code = "en";

    // Structural
    lang.keywordMap["entity"] = TokenID::ENTITY;
    lang.keywordMap["system"] = TokenID::SYSTEM;
    lang.keywordMap["function"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["on"] = TokenID::ON;
    lang.keywordMap["create"] = TokenID::CREATE;
    lang.keywordMap["update"] = TokenID::UPDATE;
    lang.keywordMap["destroy"] = TokenID::DESTROY;
    lang.keywordMap["collision"] = TokenID::COLLISION;
    lang.keywordMap["event"] = TokenID::EVENT;
    lang.keywordMap["key"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["if"] = TokenID::IF;
    lang.keywordMap["else"] = TokenID::ELSE;
    lang.keywordMap["while"] = TokenID::WHILE;
    lang.keywordMap["for"] = TokenID::FOR;
    lang.keywordMap["repeat"] = TokenID::REPEAT;
    lang.keywordMap["break"] = TokenID::BREAK;
    lang.keywordMap["continue"] = TokenID::CONTINUE;
    lang.keywordMap["return"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["var"] = TokenID::VAR_KEYWORD;
    lang.keywordMap["global"] = TokenID::GLOBAL_KEYWORD;
    lang.keywordMap["let"] = TokenID::LET_KEYWORD;
    lang.keywordMap["const"] = TokenID::CONST_KEYWORD;

    // Custom events
    lang.keywordMap["trigger"] = TokenID::TRIGGER_KEYWORD;
    lang.keywordMap["emit"] = TokenID::EMIT_KEYWORD;

    // Types
    lang.keywordMap["number"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["text"] = TokenID::TEXT_TYPE;
    lang.keywordMap["boolean"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["list"] = TokenID::LIST_TYPE;
    lang.keywordMap["object"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["true"] = TokenID::TRUE_LITERAL;
    lang.keywordMap["false"] = TokenID::FALSE_LITERAL;
    lang.keywordMap["null"] = TokenID::NULL_LITERAL;

    // Logical operators
    lang.keywordMap["and"] = TokenID::AND;
    lang.keywordMap["or"] = TokenID::OR;
    lang.keywordMap["not"] = TokenID::NOT;

    // Built-in functions
    lang.keywordMap["print"] = TokenID::PRINT;
    lang.keywordMap["move"] = TokenID::MOVE;
    lang.keywordMap["up"] = TokenID::MOVE_UP;
    lang.keywordMap["down"] = TokenID::MOVE_DOWN;
    lang.keywordMap["left"] = TokenID::MOVE_LEFT;
    lang.keywordMap["right"] = TokenID::MOVE_RIGHT;
    lang.keywordMap["teleport"] = TokenID::TELEPORT;
    lang.keywordMap["spawn"] = TokenID::CREATE_OBJECT;
    lang.keywordMap["destroy"] = TokenID::DESTROY_OBJECT;
    lang.keywordMap["find"] = TokenID::FIND;
    lang.keywordMap["wait"] = TokenID::WAIT;
    lang.keywordMap["random"] = TokenID::RANDOM;

    return lang;
}

} // namespace SAGE::Scripting::LogCon
