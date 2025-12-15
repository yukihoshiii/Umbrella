#include "lexer.h"
#include <cctype>
#include <stdexcept>
namespace umbrella {
std::map<std::string, TokenType> Lexer::keywords = {
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"function", TokenType::FUNCTION},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"class", TokenType::CLASS},
    {"new", TokenType::NEW},
    {"this", TokenType::THIS},
    {"extends", TokenType::EXTENDS},
    {"constructor", TokenType::CONSTRUCTOR},
    {"import", TokenType::IMPORT},
    {"export", TokenType::EXPORT},
    {"from", TokenType::FROM},
    {"async", TokenType::ASYNC},
    {"await", TokenType::AWAIT},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY}, // Added
    {"throw", TokenType::THROW},     // Added
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"number", TokenType::TYPE_NUMBER},
    {"string", TokenType::TYPE_STRING},
    {"boolean", TokenType::TYPE_BOOLEAN},
    {"void", TokenType::TYPE_VOID},
    {"Array", TokenType::TYPE_ARRAY}
};
Lexer::Lexer(const std::string& src)
    : source(src), position(0), line(1), column(1) {}
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        Token token = nextToken();
        if (token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    return tokens;
}
Token Lexer::nextToken() {
    skipWhitespace();
    if (isAtEnd()) {
        return makeToken(TokenType::END_OF_FILE, "");
    }
    char c = current();
    if (isDigit(c)) {
        return readNumber();
    }
    if (isAlpha(c)) {
        return readIdentifier();
    }
    if (c == '"' || c == '\'') {
        return readString();
    }
    if (c == '=' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::EQUAL_EQUAL, "==");
    }
    if (c == '!' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::BANG_EQUAL, "!=");
    }
    if (c == '<' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::LESS_EQUAL, "<=");
    }
    if (c == '>' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::GREATER_EQUAL, ">=");
    }
    if (c == '<' && peek() == '<') {
        advance();
        advance();
        return makeToken(TokenType::LEFT_SHIFT, "<<");
    }
    if (c == '>' && peek() == '>') {
        advance();
        advance();
        return makeToken(TokenType::RIGHT_SHIFT, ">>");
    }
    if (c == '&' && peek() == '&') {
        advance();
        advance();
        return makeToken(TokenType::AND_AND, "&&");
    }
    if (c == '&' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::AND_EQUAL, "&=");
    }
    if (c == '|' && peek() == '|') {
        advance();
        advance();
        return makeToken(TokenType::OR_OR, "||");
    }
    if (c == '|' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::OR_EQUAL, "|=");
    }
    if (c == '^' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::XOR_EQUAL, "^=");
    }
    if (c == '+' && peek() == '+') {
        advance();
        advance();
        return makeToken(TokenType::PLUS_PLUS, "++");
    }
    if (c == '+' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::PLUS_EQUAL, "+=");
    }
    if (c == '-' && peek() == '-') {
        advance();
        advance();
        return makeToken(TokenType::MINUS_MINUS, "--");
    }
    if (c == '-' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::MINUS_EQUAL, "-=");
    }
    if (c == '*' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::STAR_EQUAL, "*=");
    }
    if (c == '/' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::SLASH_EQUAL, "/=");
    }
    if (c == '%' && peek() == '=') {
        advance();
        advance();
        return makeToken(TokenType::PERCENT_EQUAL, "%=");
    }
    if (c == '=' && peek() == '>') {
        advance();
        advance();
        return makeToken(TokenType::ARROW, "=>");
    }
    if (c == '?' && peek() == '?') {
        advance();
        advance();
        return makeToken(TokenType::QUESTION_QUESTION, "??");
    }
    if (c == '?' && peek() == '.') {
        advance();
        advance();
        return makeToken(TokenType::QUESTION_DOT, "?.");
    }
    if (c == '.' && peek() == '.' && peek(2) == '.') {
        advance();
        advance();
        advance();
        return makeToken(TokenType::DOT_DOT_DOT, "...");
    }
    advance();
    switch (c) {
        case '+': return makeToken(TokenType::PLUS, "+");
        case '-': return makeToken(TokenType::MINUS, "-");
        case '*': return makeToken(TokenType::STAR, "*");
        case '/': 
            if (current() == '/') {
                skipComment();
                return nextToken();
            }
            return makeToken(TokenType::SLASH, "/");
        case '%': return makeToken(TokenType::PERCENT, "%");
        case '=': return makeToken(TokenType::EQUAL, "=");
        case '!': return makeToken(TokenType::BANG, "!");
        case '<': return makeToken(TokenType::LESS, "<");
        case '>': return makeToken(TokenType::GREATER, ">");
        case '&': return makeToken(TokenType::AMPERSAND, "&");
        case '|': return makeToken(TokenType::PIPE, "|");
        case '^': return makeToken(TokenType::CARET, "^");
        case '~': return makeToken(TokenType::TILDE, "~");
        case '?': return makeToken(TokenType::QUESTION, "?");
        case '(': return makeToken(TokenType::LPAREN, "(");
        case ')': return makeToken(TokenType::RPAREN, ")");
        case '{': return makeToken(TokenType::LBRACE, "{");
        case '}': return makeToken(TokenType::RBRACE, "}");
        case '[': return makeToken(TokenType::LBRACKET, "[");
        case ']': return makeToken(TokenType::RBRACKET, "]");
        case ';': return makeToken(TokenType::SEMICOLON, ";");
        case ',': return makeToken(TokenType::COMMA, ",");
        case '.': return makeToken(TokenType::DOT, ".");
        case ':': return makeToken(TokenType::COLON, ":");
        default:
            return makeToken(TokenType::INVALID, std::string(1, c));
    }
}
char Lexer::current() {
    if (isAtEnd()) return '\0';
    return source[position];
}
char Lexer::peek(int offset) {
    if (position + offset >= source.length()) return '\0';
    return source[position + offset];
}
void Lexer::advance() {
    if (!isAtEnd()) {
        if (current() == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}
void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(current())) {
        advance();
    }
}
void Lexer::skipComment() {
    while (!isAtEnd() && current() != '\n') {
        advance();
    }
}
Token Lexer::makeToken(TokenType type, const std::string& value) {
    return Token(type, value, line, column);
}
Token Lexer::readNumber() {
    std::string num;
    while (!isAtEnd() && (isDigit(current()) || current() == '.')) {
        num += current();
        advance();
    }
    return makeToken(TokenType::NUMBER, num);
}
Token Lexer::readString() {
    char quote = current();
    advance();  
    std::string str;
    while (!isAtEnd() && current() != quote) {
        if (current() == '\\') {
            advance();
            if (!isAtEnd()) {
                char escaped = current();
                switch (escaped) {
                    case 'n': str += '\n'; break;
                    case 't': str += '\t'; break;
                    case 'r': str += '\r'; break;
                    case '\\': str += '\\'; break;
                    case '"': str += '"'; break;
                    case '\'': str += '\''; break;
                    default: str += escaped;
                }
                advance();
            }
        } else {
            str += current();
            advance();
        }
    }
    if (!isAtEnd()) {
        advance();  
    }
    return makeToken(TokenType::STRING, str);
}
Token Lexer::readIdentifier() {
    std::string id;
    while (!isAtEnd() && isAlphaNumeric(current())) {
        id += current();
        advance();
    }
    auto it = keywords.find(id);
    if (it != keywords.end()) {
        return makeToken(it->second, id);
    }
    return makeToken(TokenType::IDENTIFIER, id);
}
bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}
bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}
bool Lexer::isAtEnd() {
    return position >= source.length();
}
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::LET: return "LET";
        case TokenType::CONST: return "CONST";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::RETURN: return "RETURN";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::PLUS: return "PLUS";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}
}  
