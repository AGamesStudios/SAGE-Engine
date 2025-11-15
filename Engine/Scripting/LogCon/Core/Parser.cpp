#include "Parser.h"

#include <sstream>

namespace SAGE::Scripting::LogCon {

bool Parser::IsCallableToken(TokenID id) {
    switch (id) {
    case TokenID::PRINT:
    case TokenID::MOVE:
    case TokenID::MOVE_UP:
    case TokenID::MOVE_DOWN:
    case TokenID::MOVE_LEFT:
    case TokenID::MOVE_RIGHT:
    case TokenID::TELEPORT:
    case TokenID::CREATE_OBJECT:
    case TokenID::DESTROY_OBJECT:
    case TokenID::FIND:
    case TokenID::WAIT:
    case TokenID::RANDOM:
        return true;
    default:
        return false;
    }
}

bool Parser::IsDirectionToken(TokenID id) {
    switch (id) {
    case TokenID::MOVE_UP:
    case TokenID::MOVE_DOWN:
    case TokenID::MOVE_LEFT:
    case TokenID::MOVE_RIGHT:
        return true;
    default:
        return false;
    }
}

Parser::Parser(const LanguageDefinition& language, const std::vector<Token>& tokens)
    : m_Language(language), m_Tokens(tokens) {}

ParseResult Parser::Parse() {
    m_Result = {};
    SkipNewlines();

    while (!IsAtEnd()) {
        auto entity = ParseEntity();
        if (entity) {
            m_Result.script.entities.push_back(std::move(*entity));
        } else {
            Synchronize();
        }
        SkipNewlines();
    }

    m_Result.succeeded = m_Result.errors.empty();
    return m_Result;
}

bool Parser::IsAtEnd() const {
    return m_Current >= m_Tokens.size() || m_Tokens[m_Current].id == TokenID::END_OF_FILE;
}

const Token& Parser::Peek() const {
    return m_Tokens[m_Current];
}

const Token& Parser::PeekNext(std::size_t offset) const {
    std::size_t index = m_Current + offset;
    if (index >= m_Tokens.size()) {
        return m_Tokens.back();
    }
    return m_Tokens[index];
}

const Token& Parser::Previous() const {
    return m_Tokens[m_Current - 1];
}

const Token& Parser::Advance() {
    if (!IsAtEnd()) {
        ++m_Current;
    }
    return Previous();
}

bool Parser::Check(TokenID id) const {
    if (IsAtEnd()) {
        return false;
    }
    return Peek().id == id;
}

bool Parser::Match(TokenID id) {
    if (Check(id)) {
        Advance();
        return true;
    }
    return false;
}

bool Parser::Match(TokenID first, TokenID second) {
    if (Check(first)) {
        Advance();
        if (Check(second)) {
            Advance();
            return true;
        }
        // Если второй токен не совпал, откатываемся
        --m_Current;
    }
    return false;
}

void Parser::SkipNewlines() {
    while (Match(TokenID::NEWLINE)) {
    }
}

void Parser::Synchronize() {
    while (!IsAtEnd()) {
        if (Previous().id == TokenID::SEMICOLON || Previous().id == TokenID::NEWLINE || Previous().id == TokenID::RIGHT_BRACE) {
            return;
        }

        switch (Peek().id) {
        case TokenID::ENTITY:
        case TokenID::ON:
        case TokenID::RIGHT_BRACE:
            return;
        default:
            break;
        }

        Advance();
    }
}

void Parser::SynchronizeToNextStatement() {
    while (!IsAtEnd()) {
        if (Peek().id == TokenID::NEWLINE || Peek().id == TokenID::SEMICOLON || Peek().id == TokenID::RIGHT_BRACE) {
            return;
        }
        Advance();
    }
}

void Parser::ReportError(const Token& token, const std::string& message) {
    std::ostringstream stream;
    stream << "строка " << token.line << ", столбец " << token.column << ": " << message;
    m_Result.errors.emplace_back(stream.str());
}

std::optional<AST::Entity> Parser::ParseEntity() {
    if (!Expect(TokenID::ENTITY, "Ожидалось ключевое слово сущности")) {
        return std::nullopt;
    }

    auto nameToken = ExpectIdentifier("Ожидалось имя сущности");
    if (!nameToken) {
        return std::nullopt;
    }

    if (!Expect(TokenID::LEFT_BRACE, "Ожидалась '{' для начала определения сущности")) {
        return std::nullopt;
    }

    AST::Entity entity;
    entity.name = nameToken->lexeme;

    SkipNewlines();
    while (!Check(TokenID::RIGHT_BRACE) && !IsAtEnd()) {
        if (Check(TokenID::FUNCTION)) {
            auto functionDefinition = ParseFunctionDefinition();
            if (functionDefinition) {
                entity.functions.push_back(std::move(*functionDefinition));
            } else {
                SynchronizeToNextStatement();
            }
        } else if (Check(TokenID::ON)) {
            auto eventBlock = ParseEventBlock();
            if (eventBlock) {
                entity.events.push_back(std::move(*eventBlock));
            } else {
                SynchronizeToNextStatement();
            }
        } else {
            auto statement = ParseStatement();
            if (statement) {
                entity.properties.push_back(std::move(*statement));
            } else {
                SynchronizeToNextStatement();
            }
        }
        SkipNewlines();
    }

    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце сущности")) {
        return std::nullopt;
    }

    return entity;
}

std::optional<AST::EventBlock> Parser::ParseEventBlock() {
    if (!Expect(TokenID::ON, "Ожидалось ключевое слово события")) {
        return std::nullopt;
    }

    SkipNewlines();

    if (IsAtEnd()) {
        ReportError(Previous(), "Ожидался тип события после ключевого слова");
        return std::nullopt;
    }

    const Token& eventToken = Advance();
    auto eventType = DetectEventType(eventToken.id);
    if (eventType == AST::EventBlock::Type::Unknown) {
        ReportError(eventToken, "Неизвестный тип события");
        return std::nullopt;
    }

    AST::EventBlock block;
    block.type = eventType;

    SkipNewlines();

    if (eventType == AST::EventBlock::Type::OnEvent) {
        if (!Expect(TokenID::LEFT_PAREN, "Ожидалась '(' после 'событии'")) {
            return std::nullopt;
        }

        if (Check(TokenID::STRING_LITERAL)) {
            block.eventName = Advance().stringValue;
        } else if (Check(TokenID::IDENTIFIER)) {
            block.eventName = Advance().lexeme;
        } else {
            ReportError(Peek(), "Ожидалось имя пользовательского события");
            return std::nullopt;
        }

        if (Match(TokenID::COMMA)) {
            auto parameterToken = ExpectIdentifier("Ожидалось имя параметра события");
            if (!parameterToken) {
                return std::nullopt;
            }
            block.parameter = parameterToken->lexeme;
        }

        if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' после имени события")) {
            return std::nullopt;
        }

        block.type = AST::EventBlock::Type::Custom;
    } else {
        if (Match(TokenID::LEFT_PAREN)) {
            if (Check(TokenID::IDENTIFIER)) {
                block.parameter = Advance().lexeme;
            }
            if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' после параметра события")) {
                return std::nullopt;
            }
        } else if (Check(TokenID::IDENTIFIER)) {
            block.parameter = Advance().lexeme;
        }
    }

    SkipNewlines();

    if (!Expect(TokenID::LEFT_BRACE, "Ожидался блок '{' после определения события")) {
        return std::nullopt;
    }

    SkipNewlines();

    while (!Check(TokenID::RIGHT_BRACE) && !IsAtEnd()) {
        auto statement = ParseStatement();
        if (statement) {
            block.statements.push_back(std::move(*statement));
        } else {
            SynchronizeToNextStatement();
        }
        SkipNewlines();
    }

    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце блока события")) {
        return std::nullopt;
    }

    return block;
}

std::optional<AST::Statement> Parser::ParseStatement() {
    SkipNewlines();

    if (IsAtEnd()) {
        return std::nullopt;
    }

    const Token& token = Peek();

    if (token.id == TokenID::LEFT_BRACE) {
        return ParseBlockStatement();
    }

    if (token.id == TokenID::IF) {
        return ParseIfStatement();
    }
    
    if (token.id == TokenID::WHILE) {
        return ParseWhileStatement();
    }
    
    if (token.id == TokenID::FOR) {
        return ParseForStatement();
    }
    
    if (token.id == TokenID::RETURN) {
        return ParseReturnStatement();
    }
    
    if (token.id == TokenID::BREAK) {
        Advance();
        ExpectStatementTerminator();
        return AST::Statement::MakeBreak();
    }
    
    if (token.id == TokenID::CONTINUE) {
        Advance();
        ExpectStatementTerminator();
        return AST::Statement::MakeContinue();
    }

    if (token.id == TokenID::FUNCTION) {
        return ParseFunctionDefinition();
    }

    if (token.id == TokenID::VAR_KEYWORD || token.id == TokenID::LET_KEYWORD || token.id == TokenID::GLOBAL_KEYWORD || token.id == TokenID::CONST_KEYWORD) {
        const Token scopeToken = Advance();
        return ParseVariableDeclaration(scopeToken.id);
    }

    if (token.id == TokenID::TRIGGER_KEYWORD || token.id == TokenID::EMIT_KEYWORD) {
        const Token triggerToken = Advance();
        return ParseTriggerEventStatement(triggerToken);
    }

    if (token.id == TokenID::IDENTIFIER) {
        const Token& identifier = Advance();
        
        // Проверка на присваивание массиву: arr[index] = value
        if (Check(TokenID::LEFT_BRACKET)) {
            Advance(); // Consume '['
            auto indexExpr = ParseExpression();
            if (!indexExpr) {
                ReportError(Peek(), "Ожидался индекс массива");
                return std::nullopt;
            }
            
            if (!Expect(TokenID::RIGHT_BRACKET, "Ожидалась ']'")) {
                return std::nullopt;
            }
            
            if (Check(TokenID::ASSIGN)) {
                Advance(); // Consume '='
                auto valueExpr = ParseExpression();
                if (!valueExpr) {
                    ReportError(Peek(), "Ожидалось выражение после '='");
                    return std::nullopt;
                }
                ExpectStatementTerminator();
                
                // Создаём присваивание к элементу массива
                auto targetExpr = AST::Expression::MakeIdentifier(identifier.lexeme);
                return AST::Statement::MakeArrayAssignment(std::move(targetExpr), std::move(indexExpr), std::move(valueExpr));
            }
        }
        
        if (Check(TokenID::ASSIGN)) {
            return ParseAssignment(identifier);
        }
        return ParseFunctionCall(identifier);
    }

    if (IsCallableToken(token.id)) {
        const Token keyword = Advance();
        return ParseFunctionCallFromKeyword(keyword);
    }

    ReportError(token, "Ожидалось выражение или инструкция");
    return std::nullopt;
}

std::optional<AST::Statement> Parser::ParseVariableDeclaration(TokenID scopeToken) {
    AST::Statement::VariableScope scope = AST::Statement::VariableScope::Local;
    switch (scopeToken) {
    case TokenID::GLOBAL_KEYWORD:
        scope = AST::Statement::VariableScope::Global;
        break;
    case TokenID::CONST_KEYWORD:
        scope = AST::Statement::VariableScope::Constant;
        break;
    case TokenID::VAR_KEYWORD:
    case TokenID::LET_KEYWORD:
    default:
        scope = AST::Statement::VariableScope::Local;
        break;
    }

    auto nameToken = ExpectIdentifier("Ожидалось имя переменной");
    if (!nameToken) {
        return std::nullopt;
    }

    std::string typeName;

    if (Match(TokenID::COLON)) {
        if (Check(TokenID::IDENTIFIER) || Check(TokenID::NUMBER_TYPE) || Check(TokenID::TEXT_TYPE) || Check(TokenID::BOOLEAN_TYPE) || Check(TokenID::LIST_TYPE) || Check(TokenID::OBJECT_TYPE)) {
            typeName = Advance().lexeme;
        } else {
            ReportError(Peek(), "Ожидался тип после ':'");
            return std::nullopt;
        }
    } else if (Check(TokenID::NUMBER_TYPE) || Check(TokenID::TEXT_TYPE) || Check(TokenID::BOOLEAN_TYPE) || Check(TokenID::LIST_TYPE) || Check(TokenID::OBJECT_TYPE)) {
        typeName = Advance().lexeme;
    }

    AST::ExpressionPtr initializer;
    if (Match(TokenID::ASSIGN)) {
        initializer = ParseExpression();
        if (!initializer) {
            ReportError(Peek(), "Ожидалось выражение после '='");
            return std::nullopt;
        }
    }

    ExpectStatementTerminator();

    return AST::Statement::MakeVariableDeclaration(scope, nameToken->lexeme, std::move(typeName), std::move(initializer));
}

std::optional<AST::Statement> Parser::ParseAssignment(const Token& identifier) {
    if (!Expect(TokenID::ASSIGN, "Ожидался оператор присваивания '='")) {
        return std::nullopt;
    }

    auto expression = ParseExpression();
    if (!expression) {
        ReportError(Peek(), "Ожидалось выражение после '='");
        return std::nullopt;
    }

    ExpectStatementTerminator();

    return AST::Statement::MakeAssignment(identifier.lexeme, std::move(expression));
}

std::optional<AST::Statement> Parser::ParseFunctionCall(const Token& identifier) {
    return ParseFunctionCallWithOrigin(identifier);
}

std::optional<AST::Statement> Parser::ParseFunctionCallFromKeyword(const Token& keywordToken) {
    return ParseFunctionCallWithOrigin(keywordToken);
}

std::optional<AST::Statement> Parser::ParseFunctionCallWithOrigin(const Token& originToken) {
    std::string functionName = BuildFunctionName(originToken);

    if (!Expect(TokenID::LEFT_PAREN, "Ожидалась '(' после имени функции")) {
        return std::nullopt;
    }

    auto arguments = ParseArgumentList();

    if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' в конце списка аргументов")) {
        return std::nullopt;
    }

    ExpectStatementTerminator();

    return AST::Statement::MakeFunctionCall(std::move(functionName), std::move(arguments));
}

std::optional<AST::Statement> Parser::ParseTriggerEventStatement(const Token& /*triggerToken*/) {
    SkipNewlines();

    // Дополнительное ключевое слово "событие" допускается, но не требуется
    Match(TokenID::EVENT);

    if (!Expect(TokenID::LEFT_PAREN, "Ожидалась '(' после вызова события")) {
        return std::nullopt;
    }

    std::string eventName;
    if (Check(TokenID::STRING_LITERAL)) {
        eventName = Advance().stringValue;
    } else if (Check(TokenID::IDENTIFIER)) {
        eventName = Advance().lexeme;
    } else {
        ReportError(Peek(), "Ожидалось имя события (строка или идентификатор)");
        return std::nullopt;
    }

    std::vector<AST::ExpressionPtr> arguments;
    while (Match(TokenID::COMMA)) {
        auto argument = ParseExpression();
        if (!argument) {
            ReportError(Peek(), "Ожидался аргумент события");
            return std::nullopt;
        }
        arguments.push_back(std::move(argument));
    }

    if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' после аргументов события")) {
        return std::nullopt;
    }

    ExpectStatementTerminator();

    return AST::Statement::MakeTriggerEvent(std::move(eventName), std::move(arguments));
}

std::optional<AST::Statement> Parser::ParseFunctionDefinition() {
    if (!Expect(TokenID::FUNCTION, "Ожидалось ключевое слово 'функция'")) {
        return std::nullopt;
    }

    auto nameToken = ExpectIdentifier("Ожидалось имя функции");
    if (!nameToken) {
        return std::nullopt;
    }

    if (!Expect(TokenID::LEFT_PAREN, "Ожидалась '(' в объявлении функции")) {
        return std::nullopt;
    }

    auto parameters = ParseParameterList();

    if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' в объявлении функции")) {
        return std::nullopt;
    }

    SkipNewlines();

    if (!Expect(TokenID::LEFT_BRACE, "Ожидался блок '{' после объявления функции")) {
        return std::nullopt;
    }

    auto body = ParseBlockContents();

    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце функции")) {
        return std::nullopt;
    }

    return AST::Statement::MakeFunctionDefinition(nameToken->lexeme, std::move(parameters), std::move(body));
}

std::optional<AST::Statement> Parser::ParseIfStatement() {
    if (!Expect(TokenID::IF, "Ожидалось ключевое слово 'если'")) {
        return std::nullopt;
    }

    SkipNewlines();

    auto condition = ParseExpression();
    if (!condition) {
        ReportError(Peek(), "Ожидалось выражение условия для 'если'");
        return std::nullopt;
    }

    SkipNewlines();

    if (!Expect(TokenID::LEFT_BRACE, "Ожидался блок '{' после условия")) {
        return std::nullopt;
    }

    auto thenBranch = ParseBlockContents();

    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' после блока 'если'") ) {
        return std::nullopt;
    }

    std::vector<AST::Statement> elseBranch;

    SkipNewlines();
    if (Match(TokenID::ELSE)) {
        SkipNewlines();
        if (Check(TokenID::IF)) {
            auto elseIf = ParseIfStatement();
            if (elseIf) {
                elseBranch.push_back(std::move(*elseIf));
            }
        } else if (Check(TokenID::LEFT_BRACE)) {
            Advance();
            elseBranch = ParseBlockContents();
            if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' после блока 'иначе'") ) {
                return std::nullopt;
            }
        } else {
            auto single = ParseStatement();
            if (single) {
                elseBranch.push_back(std::move(*single));
            }
        }
    }

    return AST::Statement::MakeIf(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::optional<AST::Statement> Parser::ParseBlockStatement() {
    if (!Expect(TokenID::LEFT_BRACE, "Ожидался блок '{'") ) {
        return std::nullopt;
    }

    auto statements = ParseBlockContents();

    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце блока")) {
        return std::nullopt;
    }

    return AST::Statement::MakeBlock(std::move(statements));
}
AST::ExpressionPtr Parser::ParseExpression() {
    return ParseEquality();
}

AST::ExpressionPtr Parser::ParseEquality() {
    auto expr = ParseComparison();
    while (Match(TokenID::EQUAL_EQUAL) || Match(TokenID::BANG_EQUAL)) {
        TokenID op = Previous().id;
        auto right = ParseComparison();
        expr = AST::Expression::MakeBinary(op, std::move(expr), std::move(right));
    }
    return expr;
}

AST::ExpressionPtr Parser::ParseComparison() {
    auto expr = ParseTerm();
    while (Match(TokenID::GREATER) || Match(TokenID::GREATER_EQUAL) || Match(TokenID::LESS) || Match(TokenID::LESS_EQUAL)) {
        TokenID op = Previous().id;
        auto right = ParseTerm();
        expr = AST::Expression::MakeBinary(op, std::move(expr), std::move(right));
    }
    return expr;
}

AST::ExpressionPtr Parser::ParseTerm() {
    auto expr = ParseFactor();
    while (Match(TokenID::PLUS) || Match(TokenID::MINUS)) {
        TokenID op = Previous().id;
        auto right = ParseFactor();
        expr = AST::Expression::MakeBinary(op, std::move(expr), std::move(right));
    }
    return expr;
}

AST::ExpressionPtr Parser::ParseFactor() {
    auto expr = ParseUnary();
    while (Match(TokenID::STAR) || Match(TokenID::SLASH) || Match(TokenID::MODULO)) {
        TokenID op = Previous().id;
        auto right = ParseUnary();
        expr = AST::Expression::MakeBinary(op, std::move(expr), std::move(right));
    }
    return expr;
}

AST::ExpressionPtr Parser::ParseUnary() {
    if (Match(TokenID::MINUS) || Match(TokenID::NOT)) {
        TokenID op = Previous().id;
        auto right = ParseUnary();
        if (!right) {
            return nullptr;
        }
        return AST::Expression::MakeUnary(op, std::move(right));
    }

    return ParsePostfix();
}

AST::ExpressionPtr Parser::ParsePostfix() {
    auto expr = ParsePrimary();
    
    // Доступ к элементам массива: arr[index]
    while (Match(TokenID::LEFT_BRACKET)) {
        auto index = ParseExpression();
        if (!index) {
            ReportError(Peek(), "Ожидался индекс массива");
            return nullptr;
        }
        if (!Expect(TokenID::RIGHT_BRACKET, "Ожидалась ']'")) {
            return nullptr;
        }
        expr = AST::Expression::MakeArrayAccess(std::move(expr), std::move(index));
    }
    
    return expr;
}

AST::ExpressionPtr Parser::ParsePrimary() {
    if (Check(TokenID::NUMBER_LITERAL)) {
        const Token& token = Advance();
        return AST::Expression::MakeNumber(token.numberValue);
    }

    if (Check(TokenID::STRING_LITERAL)) {
        const Token& token = Advance();
        return AST::Expression::MakeString(token.stringValue);
    }

    if (Check(TokenID::TRUE_LITERAL)) {
        Advance();
        return AST::Expression::MakeBoolean(true);
    }

    if (Check(TokenID::FALSE_LITERAL)) {
        Advance();
        return AST::Expression::MakeBoolean(false);
    }

    if (Check(TokenID::IDENTIFIER) || IsCallableToken(Peek().id)) {
        const Token& identifier = Advance();

        bool isCall = false;
        if (Check(TokenID::LEFT_PAREN)) {
            isCall = true;
        } else if (!IsAtEnd()) {
            const Token& next = Peek();
            if (IsDirectionToken(next.id)) {
                isCall = true;
            } else if (next.id == TokenID::IDENTIFIER && PeekNext().id == TokenID::LEFT_PAREN) {
                isCall = true;
            }
        }

        if (isCall) {
            return ParseCallExpression(identifier);
        }

        return AST::Expression::MakeIdentifier(identifier.lexeme);
    }

    if (Match(TokenID::LEFT_PAREN)) {
        auto expr = ParseExpression();
        if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')'")) {
            return nullptr;
        }
        return expr;
    }
    
    // Массивы: [1, 2, 3]
    if (Match(TokenID::LEFT_BRACKET)) {
        std::vector<AST::ExpressionPtr> elements;
        elements.reserve(8); // Оптимизация: резервируем память для типичных массивов
        
        if (!Check(TokenID::RIGHT_BRACKET)) {
            do {
                auto element = ParseExpression();
                if (element) {
                    elements.push_back(std::move(element));
                } else {
                    ReportError(Peek(), "Некорректный элемент массива");
                    break;
                }
            } while (Match(TokenID::COMMA));
        }
        
        if (!Expect(TokenID::RIGHT_BRACKET, "Ожидалась ']'")) {
            return nullptr;
        }
        
        return AST::Expression::MakeArray(std::move(elements));
    }

    ReportError(Peek(), "Ожидалось выражение");
    return nullptr;
}

std::vector<AST::ExpressionPtr> Parser::ParseArgumentList() {
    std::vector<AST::ExpressionPtr> arguments;

    if (Check(TokenID::RIGHT_PAREN)) {
        return arguments;
    }

    do {
        auto argument = ParseExpression();
        if (argument) {
            arguments.push_back(std::move(argument));
        } else {
            ReportError(Peek(), "Некорректный аргумент функции");
            break;
        }
    } while (Match(TokenID::COMMA));

    return arguments;
}

AST::ExpressionPtr Parser::ParseCallExpression(const Token& originToken) {
    std::string functionName = BuildFunctionName(originToken);

    if (!Expect(TokenID::LEFT_PAREN, "Ожидалась '(' после имени функции")) {
        return nullptr;
    }

    auto arguments = ParseArgumentList();

    if (!Expect(TokenID::RIGHT_PAREN, "Ожидалась ')' в конце списка аргументов")) {
        return nullptr;
    }

    return AST::Expression::MakeCall(std::move(functionName), std::move(arguments));
}

std::vector<std::string> Parser::ParseParameterList() {
    std::vector<std::string> parameters;

    if (Check(TokenID::RIGHT_PAREN)) {
        return parameters;
    }

    do {
        auto identifier = ExpectIdentifier("Ожидалось имя параметра");
        if (!identifier) {
            break;
        }
        parameters.push_back(identifier->lexeme);
    } while (Match(TokenID::COMMA));

    return parameters;
}

std::vector<AST::Statement> Parser::ParseBlockContents() {
    std::vector<AST::Statement> statements;

    SkipNewlines();
    while (!Check(TokenID::RIGHT_BRACE) && !IsAtEnd()) {
        auto statement = ParseStatement();
        if (statement) {
            statements.push_back(std::move(*statement));
        } else {
            SynchronizeToNextStatement();
        }
        SkipNewlines();
    }

    return statements;
}

std::string Parser::BuildFunctionName(const Token& firstToken) {
    std::string result = firstToken.lexeme;

    auto appendToken = [&result](const Token& token) {
        if (!result.empty()) {
            result += ' ';
        }
        result += token.lexeme;
    };

    while (!IsAtEnd()) {
        if (Check(TokenID::LEFT_PAREN)) {
            break;
        }

        const Token& next = Peek();
        if (IsDirectionToken(next.id)) {
            appendToken(Advance());
            continue;
        }

        if (next.id == TokenID::IDENTIFIER) {
            TokenID following = PeekNext().id;
            if (following == TokenID::LEFT_PAREN || IsDirectionToken(following)) {
                appendToken(Advance());
                continue;
            }
        }

        break;
    }

    return result;
}

bool Parser::Expect(TokenID id, const std::string& message) {
    if (Check(id)) {
        Advance();
        return true;
    }

    ReportError(Peek(), message);
    return false;
}

std::optional<Token> Parser::ExpectIdentifier(const std::string& message) {
    if (Check(TokenID::IDENTIFIER)) {
        return Advance();
    }

    ReportError(Peek(), message);
    return std::nullopt;
}

void Parser::ExpectStatementTerminator() {
    if (Match(TokenID::SEMICOLON)) {
        SkipNewlines();
        return;
    }

    if (Match(TokenID::NEWLINE)) {
        SkipNewlines();
        return;
    }

    if (Check(TokenID::RIGHT_BRACE) || Check(TokenID::END_OF_FILE)) {
        return;
    }

    ReportError(Peek(), "Ожидался конец инструкции (новая строка или ';')");
}

AST::EventBlock::Type Parser::DetectEventType(TokenID id) {
    switch (id) {
    case TokenID::CREATE:
        return AST::EventBlock::Type::OnCreate;
    case TokenID::UPDATE:
        return AST::EventBlock::Type::OnUpdate;
    case TokenID::DESTROY:
        return AST::EventBlock::Type::OnDestroy;
    case TokenID::COLLISION:
        return AST::EventBlock::Type::OnCollision;
    case TokenID::EVENT:
        return AST::EventBlock::Type::OnEvent;
    case TokenID::KEY_PRESS:
        return AST::EventBlock::Type::OnKeyPress;
    default:
        return AST::EventBlock::Type::Unknown;
    }
}

std::optional<AST::Statement> Parser::ParseReturnStatement() {
    Advance();  // consume 'return' / 'вернуть'
    
    // Check if there's a value to return
    AST::ExpressionPtr value;
    if (!Check(TokenID::NEWLINE) && !Check(TokenID::SEMICOLON) && !Check(TokenID::RIGHT_BRACE) && !Check(TokenID::END_OF_FILE)) {
        value = ParseExpression();
        if (!value) {
            ReportError(Peek(), "Ожидалось выражение после 'вернуть'");
            return std::nullopt;
        }
    }
    
    ExpectStatementTerminator();
    return AST::Statement::MakeReturn(std::move(value));
}

std::optional<AST::Statement> Parser::ParseWhileStatement() {
    Advance();  // consume 'while' / 'пока'
    
    // Parse condition
    auto condition = ParseExpression();
    if (!condition) {
        ReportError(Peek(), "Ожидалось условие после 'пока'");
        return std::nullopt;
    }
    
    SkipNewlines();
    
    // Parse body
    if (!Expect(TokenID::LEFT_BRACE, "Ожидался '{' после условия цикла")) {
        return std::nullopt;
    }
    
    std::vector<AST::Statement> body = ParseBlockContents();
    
    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце тела цикла")) {
        return std::nullopt;
    }
    
    return AST::Statement::MakeWhile(std::move(condition), std::move(body));
}

std::optional<AST::Statement> Parser::ParseForStatement() {
    Advance();  // consume 'for' / 'для'
    
    // Parse loop variable
    auto loopVar = ExpectIdentifier("Ожидалось имя переменной цикла");
    if (!loopVar) {
        return std::nullopt;
    }
    
    // Expect '=' or 'от' keyword
    if (!Expect(TokenID::ASSIGN, "Ожидалось '=' после переменной цикла")) {
        return std::nullopt;
    }
    
    // Parse start value
    auto fromExpr = ParseExpression();
    if (!fromExpr) {
        ReportError(Peek(), "Ожидалось начальное значение");
        return std::nullopt;
    }
    
    // Check for 'до' keyword or 'to' (TokenID::NUMBER_TYPE used as "to" in some cases)
    // For now, just expect an identifier "до" or look for next expression
    // Simplified: just parse next expression as 'to'
    
    auto toExpr = ParseExpression();
    if (!toExpr) {
        ReportError(Peek(), "Ожидалось конечное значение");
        return std::nullopt;
    }
    
    // Optional: parse step
    AST::ExpressionPtr stepExpr;
    // Check for 'шаг' keyword (if it exists) or just parse optional expression
    // For now, default to 1 if not provided
    
    SkipNewlines();
    
    // Parse body
    if (!Expect(TokenID::LEFT_BRACE, "Ожидался '{' после параметров цикла")) {
        return std::nullopt;
    }
    
    std::vector<AST::Statement> body = ParseBlockContents();
    
    if (!Expect(TokenID::RIGHT_BRACE, "Ожидалась '}' в конце тела цикла")) {
        return std::nullopt;
    }
    
    return AST::Statement::MakeFor(loopVar->lexeme, std::move(fromExpr), std::move(toExpr), std::move(stepExpr), std::move(body));
}

} // namespace SAGE::Scripting::LogCon
