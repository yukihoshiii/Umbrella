#pragma once
#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>
namespace umbrella {
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();
private:
    std::vector<Token> tokens;
    size_t current;
    Token peek(int offset = 0);
    Token advance();
    bool check(TokenType type);
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseVariableDeclaration();
    std::unique_ptr<Statement> parseFunctionDeclaration();
    std::unique_ptr<Statement> parseClassDeclaration();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseForStatement();
    std::unique_ptr<Statement> parseTryStatement();
    std::unique_ptr<Statement> parseReturnStatement();
    std::unique_ptr<Statement> parseThrowStatement(); // Added
    std::unique_ptr<Statement> parseBlockStatement();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignment();
    std::unique_ptr<Expression> parseTernary(); // Added
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseBitwiseOr(); // Added
    std::unique_ptr<Expression> parseBitwiseXor(); // Added
    std::unique_ptr<Expression> parseBitwiseAnd(); // Added
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseShift(); // Added
    std::unique_ptr<Expression> parseAddition();
    std::unique_ptr<Expression> parseMultiplication();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parsePostfix();
    std::unique_ptr<Expression> parseCall();
    std::unique_ptr<Expression> parsePrimary();
    std::unique_ptr<Expression> parseArrayLiteral();
    std::unique_ptr<Expression> parseMapLiteral();
    Type parseType();
    void error(const std::string& message);
};
}  
