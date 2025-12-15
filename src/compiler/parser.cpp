#include "parser.h"
#include <stdexcept>
#include <iostream>
namespace umbrella {
Parser::Parser(const std::vector<Token>& toks)
    : tokens(toks), current(0) {}
std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                program->statements.push_back(std::move(stmt));
            }
        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            while (!isAtEnd() && !check(TokenType::SEMICOLON)) {
                advance();
            }
            if (check(TokenType::SEMICOLON)) advance();
        }
    }
    return program;
}
Token Parser::peek(int offset) {
    if (current + offset >= tokens.size()) {
        return tokens.back();
    }
    return tokens[current + offset];
}
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}
bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}
bool Parser::match(const std::vector<TokenType>& types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(message);
    return peek();
}
bool Parser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}
std::unique_ptr<Statement> Parser::parseStatement() {
    std::unique_ptr<Statement> stmt;
    if (match(TokenType::LET) || match(TokenType::CONST)) {
        stmt = parseVariableDeclaration();
    } else if (match(TokenType::FUNCTION)) {
        stmt = parseFunctionDeclaration();
    } else if (match(TokenType::CLASS)) {
        stmt = parseClassDeclaration();
    } else if (match(TokenType::IF)) {
        stmt = parseIfStatement();
    } else if (match(TokenType::WHILE)) {
        stmt = parseWhileStatement();
    } else if (match(TokenType::FOR)) {
        stmt = parseForStatement();
    } else if (match(TokenType::RETURN)) {
        stmt = parseReturnStatement();
    } else if (match(TokenType::TRY)) {
        stmt = parseTryStatement();
    } else if (match(TokenType::LBRACE)) {
        stmt = parseBlockStatement();
    } else {
        stmt = parseExpressionStatement();
    }
    // Consume optional semicolon if present (e.g. after block/loop)
    match(TokenType::SEMICOLON); 
    return stmt;
}
std::unique_ptr<Statement> Parser::parseVariableDeclaration() {
    bool isConst = tokens[current - 1].type == TokenType::CONST;
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    Type varType = Type::ANY;
    if (match(TokenType::COLON)) {
        varType = parseType();
    }
    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::EQUAL)) {
        initializer = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VariableDeclaration>(name.value, varType, 
                                                  std::move(initializer), isConst);
}
std::unique_ptr<Statement> Parser::parseFunctionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");
    auto func = std::make_unique<FunctionDeclaration>(name.value, Type::VOID);
    if (!check(TokenType::RPAREN)) {
        do {
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            Type paramType = Type::ANY;
            if (match(TokenType::COLON)) {
                paramType = parseType();
            }
            func->parameters.push_back(FunctionParameter(paramName.value, paramType));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    if (match(TokenType::COLON)) {
        func->returnType = parseType();
    }
    consume(TokenType::LBRACE, "Expected '{' before function body");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        func->body.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after function body");
    return func;
}

std::unique_ptr<Statement> Parser::parseClassDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected class name");
    auto classDecl = std::make_unique<ClassDeclaration>(name.value);
    
    if (match(TokenType::EXTENDS)) {
        Token super = consume(TokenType::IDENTIFIER, "Expected superclass name");
        classDecl->superclass = super.value;
    }
    
    consume(TokenType::LBRACE, "Expected '{' before class body");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match(TokenType::CONSTRUCTOR)) {
            consume(TokenType::LPAREN, "Expected '(' after constructor");
            auto ctor = std::make_unique<ConstructorDeclaration>();
            if (!check(TokenType::RPAREN)) {
                do {
                    Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
                    Type paramType = Type::ANY;
                    if (match(TokenType::COLON)) paramType = parseType();
                    ctor->parameters.emplace_back(paramName.value, paramType);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after parameters");
            consume(TokenType::LBRACE, "Expected '{' before constructor body");
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                ctor->body.push_back(parseStatement());
            }
            consume(TokenType::RBRACE, "Expected '}' after constructor body");
            classDecl->constructor = std::move(ctor);
        } else {
            Token memberName = consume(TokenType::IDENTIFIER, "Expected member name");
            if (match(TokenType::LPAREN)) { // Method
                Type retType = Type::VOID;
                std::vector<FunctionParameter> params;
                if (!check(TokenType::RPAREN)) {
                    do {
                        Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
                        Type paramType = Type::ANY;
                        if (match(TokenType::COLON)) paramType = parseType();
                        params.emplace_back(paramName.value, paramType);
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::RPAREN, "Expected ')' after parameters");
                if (match(TokenType::COLON)) {
                    retType = parseType();
                }
                auto method = MethodDeclaration(memberName.value, retType);
                method.parameters = params;
                consume(TokenType::LBRACE, "Expected '{' before method body");
                while (!check(TokenType::RBRACE) && !isAtEnd()) {
                    method.body.push_back(parseStatement());
                }
                consume(TokenType::RBRACE, "Expected '}' after method body");
                classDecl->methods.push_back(std::move(method));
            } else { // Field
                Type fieldType = Type::ANY;
                if (match(TokenType::COLON)) {
                    fieldType = parseType();
                }
                std::unique_ptr<Expression> init = nullptr;
                if (match(TokenType::EQUAL)) {
                    init = parseExpression();
                }
                consume(TokenType::SEMICOLON, "Expected ';' after field declaration");
                classDecl->members.emplace_back(memberName.value, fieldType, std::move(init));
            }
        }
    }
    consume(TokenType::RBRACE, "Expected '}' after class body");
    return classDecl;
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto ifStmt = std::make_unique<IfStatement>(std::move(condition));
    if (match(TokenType::LBRACE)) {
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            ifStmt->thenBranch.push_back(parseStatement());
        }
        consume(TokenType::RBRACE, "Expected '}' after if body");
    } else {
        ifStmt->thenBranch.push_back(parseStatement());
    }
    if (match(TokenType::ELSE)) {
        if (match(TokenType::LBRACE)) {
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                ifStmt->elseBranch.push_back(parseStatement());
            }
            consume(TokenType::RBRACE, "Expected '}' after else body");
        } else {
            ifStmt->elseBranch.push_back(parseStatement());
        }
    }
    return ifStmt;
}
std::unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto whileStmt = std::make_unique<WhileStatement>(std::move(condition));
    consume(TokenType::LBRACE, "Expected '{' before while body");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        whileStmt->body.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after while body");
    return whileStmt;
}
std::unique_ptr<Statement> Parser::parseForStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'for'");
    auto forStmt = std::make_unique<ForStatement>();
    if (match(TokenType::SEMICOLON)) {
        forStmt->initializer = nullptr;
    } else if (match(TokenType::LET) || match(TokenType::CONST)) {
        forStmt->initializer = parseVariableDeclaration();
    } else {
        forStmt->initializer = parseExpressionStatement();
    }
    if (!check(TokenType::SEMICOLON)) {
        forStmt->condition = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");
    if (!check(TokenType::RPAREN)) {
        forStmt->increment = parseExpression();
    }
    consume(TokenType::RPAREN, "Expected ')' after for clauses");
    consume(TokenType::LBRACE, "Expected '{' before for body");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        forStmt->body.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after for body");
    return forStmt;
}
std::unique_ptr<Statement> Parser::parseReturnStatement() {
    std::unique_ptr<Expression> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    return std::make_unique<ReturnStatement>(std::move(value));
}
std::unique_ptr<Statement> Parser::parseBlockStatement() {
    auto block = std::make_unique<BlockStatement>();
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after block");
    return block;
}
std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}
std::unique_ptr<Statement> Parser::parseTryStatement() {
    auto tryStmt = std::make_unique<TryStatement>();
    consume(TokenType::LBRACE, "Expected '{' before try body");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        tryStmt->tryBlock.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after try body");

    if (match(TokenType::CATCH)) {
        if (match(TokenType::LPAREN)) {
            Token errorVar = consume(TokenType::IDENTIFIER, "Expected catch variable name");
            tryStmt->catchVar = errorVar.value;
            consume(TokenType::RPAREN, "Expected ')' after catch variable");
        }
        consume(TokenType::LBRACE, "Expected '{' before catch body");
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            tryStmt->catchBlock.push_back(parseStatement());
        }
        consume(TokenType::RBRACE, "Expected '}' after catch body");
    }
    return tryStmt;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}
std::unique_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    if (match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, 
               TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL})) {
        std::string op = tokens[current - 1].value;
        auto value = parseAssignment();
        return std::make_unique<AssignmentExpression>(std::move(expr), op, std::move(value));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    while (match(TokenType::OR_OR)) {
        std::string op = tokens[current - 1].value;
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    while (match(TokenType::AND_AND)) {
        std::string op = tokens[current - 1].value;
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();
    while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        std::string op = tokens[current - 1].value;
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseAddition();
    while (match({TokenType::LESS, TokenType::LESS_EQUAL, 
                  TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        std::string op = tokens[current - 1].value;
        auto right = parseAddition();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseAddition() {
    auto expr = parseMultiplication();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        std::string op = tokens[current - 1].value;
        auto right = parseMultiplication();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseMultiplication() {
    auto expr = parseUnary();
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        std::string op = tokens[current - 1].value;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parseUnary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        std::string op = tokens[current - 1].value;
        auto right = parseUnary();
        return std::make_unique<UnaryExpression>(op, std::move(right));
    }
    return parsePostfix();
}
std::unique_ptr<Expression> Parser::parsePostfix() {
    return parseCall();
}
std::unique_ptr<Expression> Parser::parseCall() {
    auto expr = parsePrimary();
    while (true) {
        if (match(TokenType::LPAREN)) {
            auto call = std::make_unique<CallExpression>(std::move(expr));
            if (!check(TokenType::RPAREN)) {
                do {
                    call->arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            expr = std::move(call);
        } else if (match(TokenType::DOT)) {
            Token name = consume(TokenType::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<MemberExpression>(std::move(expr), name.value);
        } else if (match(TokenType::LBRACKET)) {
            auto index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_unique<ArrayAccess>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}
std::unique_ptr<Expression> Parser::parsePrimary() {
    if (match(TokenType::TRUE)) {
        return std::make_unique<BooleanLiteral>(true);
    }
    if (match(TokenType::FALSE)) {
        return std::make_unique<BooleanLiteral>(false);
    }
    if (match(TokenType::NUMBER)) {
        double value = std::stod(tokens[current - 1].value);
        return std::make_unique<NumberLiteral>(value);
    }
    if (match(TokenType::STRING)) {
        return std::make_unique<StringLiteral>(tokens[current - 1].value);
    }
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<Identifier>(tokens[current - 1].value);
    }
    if (match(TokenType::THIS)) {
        return std::make_unique<Identifier>("this");
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    if (match(TokenType::LBRACKET)) {
        return parseArrayLiteral();
    }
    if (match(TokenType::LBRACE)) {
        return parseMapLiteral();
    }
    if (match(TokenType::FUNCTION)) {
        auto func = std::make_unique<FunctionExpression>();
        consume(TokenType::LPAREN, "Expected '(' after function");
        if (!check(TokenType::RPAREN)) {
            do {
                Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
                Type paramType = Type::ANY;
                if (match(TokenType::COLON)) paramType = parseType();
                func->parameters.emplace_back(paramName.value, paramType);
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after parameters");
        if (match(TokenType::COLON)) {
            func->returnType = parseType();
        }
        consume(TokenType::LBRACE, "Expected '{' before function body");
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            func->body.push_back(parseStatement());
        }
        consume(TokenType::RBRACE, "Expected '}' after function body");
        return func;
    }
    
    if (match(TokenType::NEW)) {
        Token className = consume(TokenType::IDENTIFIER, "Expected class name after 'new'");
        auto newExpr = std::make_unique<NewExpression>(className.value);
        consume(TokenType::LPAREN, "Expected '(' after class name");
        if (!check(TokenType::RPAREN)) {
            do {
                newExpr->arguments.push_back(parseExpression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after arguments");
        return newExpr;
    }
    error("Expected expression");
    return nullptr;
}
std::unique_ptr<Expression> Parser::parseMapLiteral() {
    auto map = std::make_unique<MapLiteral>();
    if (!check(TokenType::RBRACE)) {
        do {
            std::string key = consume(TokenType::STRING, "Expected string key").value;
            consume(TokenType::COLON, "Expected ':' after key");
            auto value = parseExpression();
            if (map->keys.empty()) {
                map->valueType = value->type;
            }
            map->keys.push_back(key);
            map->values.push_back(std::move(value));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RBRACE, "Expected '}' after map entries");
    return map;
}
std::unique_ptr<Expression> Parser::parseArrayLiteral() {
    auto array = std::make_unique<ArrayExpression>();
    if (!check(TokenType::RBRACKET)) {
        do {
            auto element = parseExpression();
            if (array->elements.empty()) {
                array->elementType = element->type;
            }
            array->elements.push_back(std::move(element));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RBRACKET, "Expected ']' after array elements");
    return array;
}
Type Parser::parseType() {
    if (match(TokenType::TYPE_NUMBER)) return Type::NUMBER;
    if (match(TokenType::TYPE_STRING)) return Type::STRING;
    if (match(TokenType::TYPE_BOOLEAN)) return Type::BOOLEAN;
    if (match(TokenType::TYPE_VOID)) return Type::VOID;
    if (match(TokenType::FUNCTION)) return Type::FUNCTION;
    
    if (match(TokenType::TYPE_ARRAY)) {
        if (match(TokenType::LESS)) {
            parseType();
            consume(TokenType::GREATER, "Expected '>' after array element type");
        }
        return Type::ARRAY;
    }

    if (match(TokenType::LPAREN)) { // Function type: () => T
        // Parse parameters if needed, for simplicity assumed empty or just (types)
        // Simplified: consume until ) then =>
        while (!check(TokenType::RPAREN) && !isAtEnd()) {
            parseType(); 
            match(TokenType::COMMA);
        }
        consume(TokenType::RPAREN, "Expected ')' in function type");
        consume(TokenType::ARROW, "Expected '=>' after function type parameters");
        parseType(); // Return type
        return Type::FUNCTION;
    }

    if (match(TokenType::IDENTIFIER)) { // Custom types and Generics
        if (match(TokenType::LESS)) {
            parseType();
            consume(TokenType::GREATER, "Expected '>' after generic type argument");
        }
        return Type::ANY; 
    }
    
    return Type::ANY;
}
void Parser::error(const std::string& message) {
    Token token = peek();
    throw std::runtime_error("Parse error at line " + std::to_string(token.line) + 
                           ": " + message);
}
}  
