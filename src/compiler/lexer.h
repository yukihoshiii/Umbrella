#pragma once
#include <string>
#include <vector>
#include <map>
namespace umbrella {
enum class TokenType {
    NUMBER,
    STRING,
    TRUE,
    FALSE,
    LET,
    CONST,
    FUNCTION,
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
    CLASS,
    NEW,
    THIS,
    EXTENDS,
    CONSTRUCTOR,
    IMPORT,
    EXPORT,
    FROM,
    ASYNC,
    AWAIT,
    TRY,
    CATCH,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_VOID,
    TYPE_ARRAY,
    IDENTIFIER,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    EQUAL,
    EQUAL_EQUAL,
    BANG,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    AND_AND,
    OR_OR,
    PLUS_PLUS,
    MINUS_MINUS,
    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    PERCENT_EQUAL,
    AMPERSAND,
    PIPE,
    CARET,
    TILDE,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    AND_EQUAL,
    OR_EQUAL,
    XOR_EQUAL,
    QUESTION,
    QUESTION_QUESTION,   
    QUESTION_DOT,        
    DOT_DOT_DOT,         
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    ARROW,
    END_OF_FILE,
    INVALID
};
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};
class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    Token nextToken();
private:
    std::string source;
    size_t position;
    int line;
    int column;
    char current();
    char peek(int offset = 1);
    void advance();
    void skipWhitespace();
    void skipComment();
    Token makeToken(TokenType type, const std::string& value);
    Token readNumber();
    Token readString();
    Token readIdentifier();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
    bool isAtEnd();
    static std::map<std::string, TokenType> keywords;
};
std::string tokenTypeToString(TokenType type);
}  
