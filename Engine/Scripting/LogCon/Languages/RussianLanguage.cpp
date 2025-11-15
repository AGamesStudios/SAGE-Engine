#include "LanguageDefinition.h"

namespace SAGE::Scripting::LogCon {

LanguageDefinition CreateRussianLanguage() {
    LanguageDefinition lang;
    lang.name = "Русский";
    lang.code = "ru";

    // Structural
    lang.keywordMap["сущность"] = TokenID::ENTITY;
    lang.keywordMap["система"] = TokenID::SYSTEM;
    lang.keywordMap["функция"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["при"] = TokenID::ON;
    lang.keywordMap["создании"] = TokenID::CREATE;
    lang.keywordMap["обновлении"] = TokenID::UPDATE;
    lang.keywordMap["уничтожении"] = TokenID::DESTROY;
    lang.keywordMap["столкновении"] = TokenID::COLLISION;
    lang.keywordMap["событии"] = TokenID::EVENT;
    lang.keywordMap["нажатии"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["если"] = TokenID::IF;
    lang.keywordMap["иначе"] = TokenID::ELSE;
    lang.keywordMap["пока"] = TokenID::WHILE;
    lang.keywordMap["для"] = TokenID::FOR;
    lang.keywordMap["повторить"] = TokenID::REPEAT;
    lang.keywordMap["прервать"] = TokenID::BREAK;
    lang.keywordMap["продолжить"] = TokenID::CONTINUE;
    lang.keywordMap["вернуть"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["перем"] = TokenID::VAR_KEYWORD;      // переменная
    lang.keywordMap["глобал"] = TokenID::GLOBAL_KEYWORD;   // глобальная
    lang.keywordMap["пусть"] = TokenID::LET_KEYWORD;       // пусть (альтернатива var)
    lang.keywordMap["конст"] = TokenID::CONST_KEYWORD;     // константа

    // Custom events
    lang.keywordMap["вызвать"] = TokenID::TRIGGER_KEYWORD; // вызвать событие
    lang.keywordMap["испустить"] = TokenID::EMIT_KEYWORD;  // испустить событие

    // Types
    lang.keywordMap["число"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["текст"] = TokenID::TEXT_TYPE;
    lang.keywordMap["булево"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["список"] = TokenID::LIST_TYPE;
    lang.keywordMap["объект"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["да"] = TokenID::TRUE_LITERAL;
    lang.keywordMap["нет"] = TokenID::FALSE_LITERAL;
    lang.keywordMap["пусто"] = TokenID::NULL_LITERAL;

    // Logical operators
    lang.keywordMap["и"] = TokenID::AND;
    lang.keywordMap["или"] = TokenID::OR;
    lang.keywordMap["не"] = TokenID::NOT;

    // Built-in functions (subset)
    lang.keywordMap["вывести"] = TokenID::PRINT;
    lang.keywordMap["двигать"] = TokenID::MOVE;
    lang.keywordMap["вверх"] = TokenID::MOVE_UP;
    lang.keywordMap["вниз"] = TokenID::MOVE_DOWN;
    lang.keywordMap["влево"] = TokenID::MOVE_LEFT;
    lang.keywordMap["вправо"] = TokenID::MOVE_RIGHT;
    lang.keywordMap["телепортировать"] = TokenID::TELEPORT;
    lang.keywordMap["создать"] = TokenID::CREATE_OBJECT;
    lang.keywordMap["уничтожить"] = TokenID::DESTROY_OBJECT;
    lang.keywordMap["найти"] = TokenID::FIND;
    lang.keywordMap["ждать"] = TokenID::WAIT;
    lang.keywordMap["случайное"] = TokenID::RANDOM;

    return lang;
}

} // namespace SAGE::Scripting::LogCon
