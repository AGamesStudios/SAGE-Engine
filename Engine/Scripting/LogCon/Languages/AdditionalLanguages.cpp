#include "LanguageDefinition.h"

namespace SAGE::Scripting::LogCon {

LanguageDefinition CreateSpanishLanguage() {
    LanguageDefinition lang;
    lang.name = "Español";
    lang.code = "es";

    // Structural
    lang.keywordMap["entidad"] = TokenID::ENTITY;
    lang.keywordMap["sistema"] = TokenID::SYSTEM;
    lang.keywordMap["funcion"] = TokenID::FUNCTION;
    lang.keywordMap["función"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["en"] = TokenID::ON;
    lang.keywordMap["crear"] = TokenID::CREATE;
    lang.keywordMap["actualizar"] = TokenID::UPDATE;
    lang.keywordMap["destruir"] = TokenID::DESTROY;
    lang.keywordMap["colision"] = TokenID::COLLISION;
    lang.keywordMap["colisión"] = TokenID::COLLISION;
    lang.keywordMap["evento"] = TokenID::EVENT;
    lang.keywordMap["tecla"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["si"] = TokenID::IF;
    lang.keywordMap["sino"] = TokenID::ELSE;
    lang.keywordMap["mientras"] = TokenID::WHILE;
    lang.keywordMap["para"] = TokenID::FOR;
    lang.keywordMap["repetir"] = TokenID::REPEAT;
    lang.keywordMap["romper"] = TokenID::BREAK;
    lang.keywordMap["continuar"] = TokenID::CONTINUE;
    lang.keywordMap["retornar"] = TokenID::RETURN;
    lang.keywordMap["devolver"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["var"] = TokenID::VAR_KEYWORD;
    lang.keywordMap["global"] = TokenID::GLOBAL_KEYWORD;
    lang.keywordMap["const"] = TokenID::CONST_KEYWORD;

    // Types
    lang.keywordMap["numero"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["número"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["texto"] = TokenID::TEXT_TYPE;
    lang.keywordMap["booleano"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["lista"] = TokenID::LIST_TYPE;
    lang.keywordMap["objeto"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["verdadero"] = TokenID::TRUE;
    lang.keywordMap["falso"] = TokenID::FALSE;
    lang.keywordMap["nulo"] = TokenID::NULL_VALUE;

    // Operators (text-based)
    lang.keywordMap["y"] = TokenID::AND;
    lang.keywordMap["o"] = TokenID::OR;
    lang.keywordMap["no"] = TokenID::NOT;

    // Common keywords
    lang.keywordMap["hasta"] = TokenID::TO;
    lang.keywordMap["paso"] = TokenID::STEP;
    lang.keywordMap["hacer"] = TokenID::DO;

    // Build reverse map
    for (const auto& [word, id] : lang.keywordMap) {
        lang.tokenToKeyword[id] = word;
    }

    return lang;
}

LanguageDefinition CreateFrenchLanguage() {
    LanguageDefinition lang;
    lang.name = "Français";
    lang.code = "fr";

    // Structural
    lang.keywordMap["entite"] = TokenID::ENTITY;
    lang.keywordMap["entité"] = TokenID::ENTITY;
    lang.keywordMap["systeme"] = TokenID::SYSTEM;
    lang.keywordMap["système"] = TokenID::SYSTEM;
    lang.keywordMap["fonction"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["sur"] = TokenID::ON;
    lang.keywordMap["creer"] = TokenID::CREATE;
    lang.keywordMap["créer"] = TokenID::CREATE;
    lang.keywordMap["mettreajour"] = TokenID::UPDATE;
    lang.keywordMap["detruire"] = TokenID::DESTROY;
    lang.keywordMap["détruire"] = TokenID::DESTROY;
    lang.keywordMap["collision"] = TokenID::COLLISION;
    lang.keywordMap["evenement"] = TokenID::EVENT;
    lang.keywordMap["événement"] = TokenID::EVENT;
    lang.keywordMap["touche"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["si"] = TokenID::IF;
    lang.keywordMap["sinon"] = TokenID::ELSE;
    lang.keywordMap["tantque"] = TokenID::WHILE;
    lang.keywordMap["pour"] = TokenID::FOR;
    lang.keywordMap["repeter"] = TokenID::REPEAT;
    lang.keywordMap["répéter"] = TokenID::REPEAT;
    lang.keywordMap["arreter"] = TokenID::BREAK;
    lang.keywordMap["arrêter"] = TokenID::BREAK;
    lang.keywordMap["continuer"] = TokenID::CONTINUE;
    lang.keywordMap["retourner"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["var"] = TokenID::VAR_KEYWORD;
    lang.keywordMap["global"] = TokenID::GLOBAL_KEYWORD;
    lang.keywordMap["const"] = TokenID::CONST_KEYWORD;

    // Types
    lang.keywordMap["nombre"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["texte"] = TokenID::TEXT_TYPE;
    lang.keywordMap["booleen"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["booléen"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["liste"] = TokenID::LIST_TYPE;
    lang.keywordMap["objet"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["vrai"] = TokenID::TRUE;
    lang.keywordMap["faux"] = TokenID::FALSE;
    lang.keywordMap["nul"] = TokenID::NULL_VALUE;

    // Operators
    lang.keywordMap["et"] = TokenID::AND;
    lang.keywordMap["ou"] = TokenID::OR;
    lang.keywordMap["non"] = TokenID::NOT;

    // Common keywords
    lang.keywordMap["a"] = TokenID::TO;
    lang.keywordMap["à"] = TokenID::TO;
    lang.keywordMap["pas"] = TokenID::STEP;
    lang.keywordMap["faire"] = TokenID::DO;

    for (const auto& [word, id] : lang.keywordMap) {
        lang.tokenToKeyword[id] = word;
    }

    return lang;
}

LanguageDefinition CreateGermanLanguage() {
    LanguageDefinition lang;
    lang.name = "Deutsch";
    lang.code = "de";

    // Structural
    lang.keywordMap["entitaet"] = TokenID::ENTITY;
    lang.keywordMap["entität"] = TokenID::ENTITY;
    lang.keywordMap["system"] = TokenID::SYSTEM;
    lang.keywordMap["funktion"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["bei"] = TokenID::ON;
    lang.keywordMap["erstellen"] = TokenID::CREATE;
    lang.keywordMap["aktualisieren"] = TokenID::UPDATE;
    lang.keywordMap["zerstoeren"] = TokenID::DESTROY;
    lang.keywordMap["zerstören"] = TokenID::DESTROY;
    lang.keywordMap["kollision"] = TokenID::COLLISION;
    lang.keywordMap["ereignis"] = TokenID::EVENT;
    lang.keywordMap["taste"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["wenn"] = TokenID::IF;
    lang.keywordMap["sonst"] = TokenID::ELSE;
    lang.keywordMap["waehrend"] = TokenID::WHILE;
    lang.keywordMap["während"] = TokenID::WHILE;
    lang.keywordMap["fuer"] = TokenID::FOR;
    lang.keywordMap["für"] = TokenID::FOR;
    lang.keywordMap["wiederholen"] = TokenID::REPEAT;
    lang.keywordMap["abbrechen"] = TokenID::BREAK;
    lang.keywordMap["fortsetzen"] = TokenID::CONTINUE;
    lang.keywordMap["zurueckgeben"] = TokenID::RETURN;
    lang.keywordMap["zurückgeben"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["var"] = TokenID::VAR_KEYWORD;
    lang.keywordMap["global"] = TokenID::GLOBAL_KEYWORD;
    lang.keywordMap["const"] = TokenID::CONST_KEYWORD;

    // Types
    lang.keywordMap["zahl"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["text"] = TokenID::TEXT_TYPE;
    lang.keywordMap["boolean"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["liste"] = TokenID::LIST_TYPE;
    lang.keywordMap["objekt"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["wahr"] = TokenID::TRUE;
    lang.keywordMap["falsch"] = TokenID::FALSE;
    lang.keywordMap["null"] = TokenID::NULL_VALUE;

    // Operators
    lang.keywordMap["und"] = TokenID::AND;
    lang.keywordMap["oder"] = TokenID::OR;
    lang.keywordMap["nicht"] = TokenID::NOT;

    // Common keywords
    lang.keywordMap["bis"] = TokenID::TO;
    lang.keywordMap["schritt"] = TokenID::STEP;
    lang.keywordMap["machen"] = TokenID::DO;

    for (const auto& [word, id] : lang.keywordMap) {
        lang.tokenToKeyword[id] = word;
    }

    return lang;
}

LanguageDefinition CreateChineseLanguage() {
    LanguageDefinition lang;
    lang.name = "中文";
    lang.code = "zh";

    // Structural
    lang.keywordMap["实体"] = TokenID::ENTITY;
    lang.keywordMap["系统"] = TokenID::SYSTEM;
    lang.keywordMap["函数"] = TokenID::FUNCTION;

    // Events
    lang.keywordMap["当"] = TokenID::ON;
    lang.keywordMap["创建"] = TokenID::CREATE;
    lang.keywordMap["更新"] = TokenID::UPDATE;
    lang.keywordMap["销毁"] = TokenID::DESTROY;
    lang.keywordMap["碰撞"] = TokenID::COLLISION;
    lang.keywordMap["事件"] = TokenID::EVENT;
    lang.keywordMap["按键"] = TokenID::KEY_PRESS;

    // Flow control
    lang.keywordMap["如果"] = TokenID::IF;
    lang.keywordMap["否则"] = TokenID::ELSE;
    lang.keywordMap["当循环"] = TokenID::WHILE;
    lang.keywordMap["对于"] = TokenID::FOR;
    lang.keywordMap["重复"] = TokenID::REPEAT;
    lang.keywordMap["中断"] = TokenID::BREAK;
    lang.keywordMap["继续"] = TokenID::CONTINUE;
    lang.keywordMap["返回"] = TokenID::RETURN;

    // Variable scopes
    lang.keywordMap["变量"] = TokenID::VAR_KEYWORD;
    lang.keywordMap["全局"] = TokenID::GLOBAL_KEYWORD;
    lang.keywordMap["常量"] = TokenID::CONST_KEYWORD;

    // Types
    lang.keywordMap["数字"] = TokenID::NUMBER_TYPE;
    lang.keywordMap["文本"] = TokenID::TEXT_TYPE;
    lang.keywordMap["布尔"] = TokenID::BOOLEAN_TYPE;
    lang.keywordMap["列表"] = TokenID::LIST_TYPE;
    lang.keywordMap["对象"] = TokenID::OBJECT_TYPE;

    // Literals
    lang.keywordMap["真"] = TokenID::TRUE;
    lang.keywordMap["假"] = TokenID::FALSE;
    lang.keywordMap["空"] = TokenID::NULL_VALUE;

    // Operators
    lang.keywordMap["与"] = TokenID::AND;
    lang.keywordMap["或"] = TokenID::OR;
    lang.keywordMap["非"] = TokenID::NOT;

    // Common keywords
    lang.keywordMap["到"] = TokenID::TO;
    lang.keywordMap["步长"] = TokenID::STEP;
    lang.keywordMap["执行"] = TokenID::DO;

    for (const auto& [word, id] : lang.keywordMap) {
        lang.tokenToKeyword[id] = word;
    }

    return lang;
}

} // namespace SAGE::Scripting::LogCon
