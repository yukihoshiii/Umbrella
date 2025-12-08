#include "ast.h"
#include <sstream>
namespace umbrella {
std::string typeToString(Type type) {
    switch (type) {
        case Type::NUMBER: return "number";
        case Type::STRING: return "string";
        case Type::BOOLEAN: return "boolean";
        case Type::VOID: return "void";
        case Type::ANY: return "any";
        case Type::FUNCTION: return "function";
        case Type::ARRAY: return "Array";
        case Type::CLASS: return "class";
        default: return "unknown";
    }
}
std::string NumberLiteral::toString() const {
    return std::to_string(value);
}
std::string StringLiteral::toString() const {
    return "\"" + value + "\"";
}
std::string BooleanLiteral::toString() const {
    return value ? "true" : "false";
}
std::string Identifier::toString() const {
    return name;
}
std::string BinaryExpression::toString() const {
    return "(" + left->toString() + " " + op + " " + right->toString() + ")";
}
std::string UnaryExpression::toString() const {
    return "(" + op + operand->toString() + ")";
}
std::string CallExpression::toString() const {
    std::stringstream ss;
    ss << callee->toString() << "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    ss << ")";
    return ss.str();
}
std::string ArrayExpression::toString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements.size(); i++) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    ss << "]";
    ss << "]";
    return ss.str();
}
std::string MapLiteral::toString() const {
    std::stringstream ss;
    ss << "{";
    for (size_t i = 0; i < keys.size(); i++) {
        if (i > 0) ss << ", ";
        ss << "\"" << keys[i] << "\": " << values[i]->toString();
    }
    ss << "}";
    return ss.str();
}
std::string ArrayAccess::toString() const {
    return array->toString() + "[" + index->toString() + "]";
}
std::string MemberExpression::toString() const {
    return object->toString() + "." + property;
}
std::string NewExpression::toString() const {
    std::stringstream ss;
    ss << "new " << className << "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    ss << ")";
    return ss.str();
}
std::string ConditionalExpression::toString() const {
    return "(" + condition->toString() + " ? " + 
           thenExpr->toString() + " : " + elseExpr->toString() + ")";
}
std::string AssignmentExpression::toString() const {
    return left->toString() + " " + op + " " + right->toString();
}
std::string ExpressionStatement::toString() const {
    return expression->toString() + ";";
}
std::string VariableDeclaration::toString() const {
    std::stringstream ss;
    ss << (isConst ? "const " : "let ") << name;
    if (varType != Type::ANY) {
        ss << ": " << typeToString(varType);
    }
    if (initializer) {
        ss << " = " << initializer->toString();
    }
    ss << ";";
    return ss.str();
}
std::string FunctionDeclaration::toString() const {
    std::stringstream ss;
    ss << "function " << name << "(";
    for (size_t i = 0; i < parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << parameters[i].name << ": " << typeToString(parameters[i].type);
    }
    ss << "): " << typeToString(returnType) << " { ... }";
    return ss.str();
}
std::string ReturnStatement::toString() const {
    if (value) {
        return "return " + value->toString() + ";";
    }
    return "return;";
}
std::string IfStatement::toString() const {
    std::stringstream ss;
    ss << "if (" << condition->toString() << ") { ... }";
    if (!elseBranch.empty()) {
        ss << " else { ... }";
    }
    return ss.str();
}
std::string WhileStatement::toString() const {
    return "while (" + condition->toString() + ") { ... }";
}
std::string ForStatement::toString() const {
    return "for (...) { ... }";
}
std::string BlockStatement::toString() const {
    std::stringstream ss;
    ss << "{\n";
    for (const auto& stmt : statements) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}
std::string Program::toString() const {
    std::stringstream ss;
    for (const auto& stmt : statements) {
        ss << stmt->toString() << "\n";
    }
    return ss.str();
}
std::string ClassDeclaration::toString() const {
    std::stringstream ss;
    ss << "class " << name;
    if (!superclass.empty()) {
        ss << " extends " << superclass;
    }
    ss << " { ... }";
    return ss.str();
}
}  
