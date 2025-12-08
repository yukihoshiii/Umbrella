#pragma once
#include "ast.h"
#include <string>
#include <map>
#include <set>
namespace umbrella {
class CodeGenerator {
public:
    CodeGenerator();
    std::string generate(const Program& program);
private:
    std::string generateStatement(const Statement* stmt);
    std::string generateExpression(const Expression* expr);
    std::string generateVariableDeclaration(const VariableDeclaration* decl);
    std::string generateFunctionDeclaration(const FunctionDeclaration* decl);
    std::string generateReturnStatement(const ReturnStatement* stmt);
    std::string generateIfStatement(const IfStatement* stmt);
    std::string generateWhileStatement(const WhileStatement* stmt);
    std::string generateForStatement(const ForStatement* stmt);
    std::string generateBlockStatement(const BlockStatement* stmt);
    std::string generateExpressionStatement(const ExpressionStatement* stmt);
    std::string generateNumberLiteral(const NumberLiteral* expr);
    std::string generateStringLiteral(const StringLiteral* expr);
    std::string generateBooleanLiteral(const BooleanLiteral* expr);
    std::string generateIdentifier(const Identifier* expr);
    std::string generateBinaryExpression(const BinaryExpression* expr);
    std::string generateUnaryExpression(const UnaryExpression* expr);
    std::string generateCallExpression(const CallExpression* expr);
    std::string generateArrayExpression(const ArrayExpression* expr);
    std::string typeToCppType(Type type);
    std::string escapeString(const std::string& str);
    int indentLevel;
    std::string indent();
    std::set<std::string> declaredVariables;
    std::map<std::string, Type> variableTypes;
};
}  
