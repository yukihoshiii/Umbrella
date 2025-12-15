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

std::unique_ptr<Statement> Parser::parseVariableDeclaration() {
    bool isConst = tokens[current - 1].type == TokenType::CONST;
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    Type varType = Type::ANY;
    std::string cppType = "";
    if (match(TokenType::COLON)) {
        size_t startToken = current;
        varType = parseType();
        size_t endToken = current;
        // Reconstruct type string from tokens with mapping
        for (size_t i = startToken; i < endToken; i++) {
             std::string val = tokens[i].value;
             if (tokens[i].type == TokenType::TYPE_STRING) val = "std::string";
             else if (tokens[i].type == TokenType::TYPE_NUMBER) val = "double";
             else if (tokens[i].type == TokenType::TYPE_BOOLEAN) val = "bool";
             else if (tokens[i].type == TokenType::TYPE_VOID) val = "void";
             // else if (tokens[i].type == TokenType::TYPE_ANY) val = "auto";
             else if (tokens[i].type == TokenType::FUNCTION) val = "auto";
             else if (tokens[i].value == "function") val = "auto"; // Just in case lexer difference
             
             cppType += val;
        }
        // Hack: parseType() consumes tokens but tokens[i].value for < and > etc are correct.
        // Wait, parseType consumes tokens, so 'current' advances.
        // tokens[startToken] is the first token of type.
        // BUT tokens are objects.
        // I need to ensure spacing or just concat?
        // Array<Thread> -> Array < Thread >
        // C++ parser will handle spaces fine usually.
    }
    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::EQUAL)) {
        initializer = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VariableDeclaration>(name.value, varType, 
                                                  std::move(initializer), isConst, cppType);
}
std::unique_ptr<Statement> Parser::parseFunctionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");
    auto func = std::make_unique<FunctionDeclaration>(name.value, Type::ANY);
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
    consume(TokenType::LBRACE, "Expected '{' before try block");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        tryStmt->tryBlock.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' after try block");

    if (match(TokenType::CATCH)) {
        consume(TokenType::LPAREN, "Expected '(' after 'catch'");
        Token errorVar = consume(TokenType::IDENTIFIER, "Expected error variable name");
        tryStmt->catchVar = errorVar.value;
        consume(TokenType::RPAREN, "Expected ')' after error variable");
        consume(TokenType::LBRACE, "Expected '{' before catch block");
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            tryStmt->catchBlock.push_back(parseStatement());
        }
        consume(TokenType::RBRACE, "Expected '}' after catch block");
    }

    if (match(TokenType::FINALLY)) {
        consume(TokenType::LBRACE, "Expected '{' before finally block");
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            tryStmt->finallyBlock.push_back(parseStatement());
        }
        consume(TokenType::RBRACE, "Expected '}' after finally block");
    }

    return tryStmt;
}

std::unique_ptr<Statement> Parser::parseThrowStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after throw");
    return std::make_unique<ThrowStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::FUNCTION)) return parseFunctionDeclaration();
    if (match(TokenType::CLASS)) return parseClassDeclaration();
    if (match(TokenType::LET) || match(TokenType::CONST)) return parseVariableDeclaration();
    if (match(TokenType::RETURN)) return parseReturnStatement();
    if (match(TokenType::THROW)) return parseThrowStatement();
    if (match(TokenType::IF)) return parseIfStatement();
    if (match(TokenType::WHILE)) return parseWhileStatement();
    if (match(TokenType::FOR)) return parseForStatement();
    if (match(TokenType::TRY)) return parseTryStatement();
    if (match(TokenType::LBRACE)) return parseBlockStatement();
    
    return parseExpressionStatement();
}

std::unique_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseTernary(); // Was parseLogicalOr
    if (match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, 
               TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL, TokenType::PERCENT_EQUAL,
               TokenType::AND_EQUAL, TokenType::OR_EQUAL, TokenType::XOR_EQUAL})) {
        std::string op = tokens[current - 1].value;
        auto value = parseAssignment();
        return std::make_unique<AssignmentExpression>(std::move(expr), op, std::move(value));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseTernary() {
    auto expr = parseLogicalOr();
    if (match(TokenType::QUESTION)) {
        auto thenBranch = parseExpression(); // Allow assignment in branches? usually yes, or parseTernary
        consume(TokenType::COLON, "Expected ':' in ternary operator");
        auto elseBranch = parseTernary(); // Right associative
        return std::make_unique<ConditionalExpression>(std::move(expr), std::move(thenBranch), std::move(elseBranch));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
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
    auto expr = parseBitwiseOr(); // Was parseEquality
    while (match(TokenType::AND_AND)) {
        std::string op = tokens[current - 1].value;
        auto right = parseBitwiseOr();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseBitwiseOr() {
    auto expr = parseBitwiseXor();
    while (match(TokenType::PIPE)) {
        std::string op = tokens[current - 1].value;
        auto right = parseBitwiseXor();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseBitwiseXor() {
    auto expr = parseBitwiseAnd();
    while (match(TokenType::CARET)) {
        std::string op = tokens[current - 1].value;
        auto right = parseBitwiseAnd();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseBitwiseAnd() {
    auto expr = parseEquality();
    while (match(TokenType::AMPERSAND)) {
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
    auto expr = parseShift(); // Was parseAddition
    while (match({TokenType::LESS, TokenType::LESS_EQUAL, 
                  TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        std::string op = tokens[current - 1].value;
        auto right = parseShift();
        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseShift() {
    auto expr = parseAddition();
    while (match({TokenType::LEFT_SHIFT, TokenType::RIGHT_SHIFT})) {
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
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::TILDE})) { // Added TILDE
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
    // Handle (expression) or Arrow Function (params) => body
    if (match(TokenType::LPAREN)) {
        size_t savedCurrent = current;
        bool isArrow = false;
        std::vector<FunctionParameter> params;
        // Try to parse as parameter list
        try {
            if (check(TokenType::RPAREN)) {
                advance(); // Empty params ()
                if (check(TokenType::ARROW)) isArrow = true;
            } else {
                do {
                    if (!check(TokenType::IDENTIFIER)) throw false;
                    Token name = advance();
                    Type type = Type::ANY;
                    if (match(TokenType::COLON)) type = parseType();
                    params.emplace_back(name.value, type);
                } while (match(TokenType::COMMA));
                
                if (match(TokenType::RPAREN)) {
                    if (check(TokenType::ARROW)) isArrow = true;
                }
            }
        } catch (...) {
            isArrow = false;
        }

        if (isArrow) {
            consume(TokenType::ARROW, "Expected '=>' after parameters");
            auto func = std::make_unique<FunctionExpression>();
            func->parameters = params;
            if (match(TokenType::LBRACE)) {
                while (!check(TokenType::RBRACE) && !isAtEnd()) {
                    func->body.push_back(parseStatement());
                }
                consume(TokenType::RBRACE, "Expected '}'");
            } else {
                auto expr = parseExpression();
                func->body.push_back(std::make_unique<ReturnStatement>(std::move(expr)));
            }
            return func;
        }

        // Backtrack and parse as normal expression
        current = savedCurrent;
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    if (match(TokenType::THIS)) {
        return std::make_unique<Identifier>("this");
    }
    
    if (match(TokenType::IDENTIFIER)) {
        Token name = tokens[current - 1];
        // Check for Single Argument Arrow Function: x => ...
        if (match(TokenType::ARROW)) {
             auto func = std::make_unique<FunctionExpression>();
             func->parameters.emplace_back(name.value, Type::ANY);
             if (match(TokenType::LBRACE)) {
                while (!check(TokenType::RBRACE) && !isAtEnd()) {
                    func->body.push_back(parseStatement());
                }
                consume(TokenType::RBRACE, "Expected '}'");
            } else {
                auto expr = parseExpression();
                func->body.push_back(std::make_unique<ReturnStatement>(std::move(expr)));
            }
            return func;
        }
        return std::make_unique<Identifier>(name.value);
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
            do {
                parseType();
            } while (match(TokenType::COMMA));
            consume(TokenType::GREATER, "Expected '>' after generic type arguments");
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
