#include "codegen.h"
#include <sstream>
#include <stdexcept>
namespace umbrella {
CodeGenerator::CodeGenerator() : indentLevel(0) {}
std::string CodeGenerator::generate(const Program& program) {
    std::stringstream ss;
    ss << "#include <iostream>\n";
    ss << "#include <string>\n";
    ss << "#include <vector>\n";
    ss << "#include <cmath>\n";
    ss << "#include <algorithm>\n";
    ss << "#include <cstdlib>\n";
    ss << "#include <ctime>\n";
    ss << "#include \"runtime/runtime.h\"\n";
    ss << "#include \"runtime/advanced.h\"\n\n";
    ss << "using namespace umbrella::runtime;\n\n";
    for (const auto& stmt : program.statements) {
        ss << generateStatement(stmt.get());
    }
    return ss.str();
}
std::string CodeGenerator::generateStatement(const Statement* stmt) {
    if (auto varDecl = dynamic_cast<const VariableDeclaration*>(stmt)) {
        return generateVariableDeclaration(varDecl);
    }
    if (auto funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt)) {
        return generateFunctionDeclaration(funcDecl);
    }
    if (auto classDecl = dynamic_cast<const ClassDeclaration*>(stmt)) {
        return generateClassDeclaration(classDecl);
    }
    if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt)) {
        return generateReturnStatement(retStmt);
    }
    if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt)) {
        return generateIfStatement(ifStmt);
    }
    if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt)) {
        return generateWhileStatement(whileStmt);
    }
    if (auto forStmt = dynamic_cast<const ForStatement*>(stmt)) {
        return generateForStatement(forStmt);
    }
    if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt)) {
        return generateBlockStatement(blockStmt);
    }
    if (auto tryStmt = dynamic_cast<const TryStatement*>(stmt)) {
        return generateTryStatement(tryStmt);
    }
    if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt)) {
        return generateExpressionStatement(exprStmt);
    }
    return "";
}
std::string CodeGenerator::generateExpression(const Expression* expr) {
    if (auto numLit = dynamic_cast<const NumberLiteral*>(expr)) {
        return generateNumberLiteral(numLit);
    }
    if (auto strLit = dynamic_cast<const StringLiteral*>(expr)) {
        return generateStringLiteral(strLit);
    }
    if (auto boolLit = dynamic_cast<const BooleanLiteral*>(expr)) {
        return generateBooleanLiteral(boolLit);
    }
    if (auto id = dynamic_cast<const Identifier*>(expr)) {
        return generateIdentifier(id);
    }
    if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr)) {
        return generateBinaryExpression(binExpr);
    }
    if (auto assignExpr = dynamic_cast<const AssignmentExpression*>(expr)) {
        return generateAssignmentExpression(assignExpr);
    }
    if (auto unExpr = dynamic_cast<const UnaryExpression*>(expr)) {
        return generateUnaryExpression(unExpr);
    }
    if (auto callExpr = dynamic_cast<const CallExpression*>(expr)) {
        return generateCallExpression(callExpr);
    }
    if (auto arrExpr = dynamic_cast<const ArrayExpression*>(expr)) {
        return generateArrayExpression(arrExpr);
    }
    if (auto memExpr = dynamic_cast<const MemberExpression*>(expr)) {
        return generateMemberExpression(memExpr);
    }
    if (auto accessExpr = dynamic_cast<const ArrayAccess*>(expr)) {
        return generateArrayAccess(accessExpr);
    }
    if (auto mapLit = dynamic_cast<const MapLiteral*>(expr)) {
        return generateMapLiteral(mapLit);
    }
    if (auto newExpr = dynamic_cast<const NewExpression*>(expr)) {
        return generateNewExpression(newExpr);
    }
    if (auto funcExpr = dynamic_cast<const FunctionExpression*>(expr)) {
        return generateFunctionExpression(funcExpr);
    }
    return "";
}

std::string CodeGenerator::generateTryStatement(const TryStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "try {\n";
    indentLevel++;
    for (const auto& s : stmt->tryBlock) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "}";
    
    if (!stmt->catchBlock.empty()) {
        ss << " catch (const std::exception& e) {\n";
        indentLevel++;
        if (!stmt->catchVar.empty() && stmt->catchVar != "e") {
           ss << indent() << "std::string " << stmt->catchVar << " = " << "e.what();\n";
        }
        for (const auto& s : stmt->catchBlock) {
            ss << generateStatement(s.get());
        }
        indentLevel--;
        ss << indent() << "}\n";
    } else {
        ss << "\n"; 
    }
    return ss.str();
}

std::string CodeGenerator::generateAssignmentExpression(const AssignmentExpression* expr) {
    std::stringstream ss;
    ss << generateExpression(expr->left.get()) << " " << expr->op << " " << generateExpression(expr->right.get());
    return ss.str();
}

std::string CodeGenerator::generateNewExpression(const NewExpression* expr) {
    std::stringstream ss;
    ss << expr->className << "(";
    for (size_t i = 0; i < expr->arguments.size(); i++) {
        if (i > 0) ss << ", ";
        ss << generateExpression(expr->arguments[i].get());
    }
    ss << ")";
    return ss.str();
}

std::string CodeGenerator::generateArrayAccess(const ArrayAccess* expr) {
    return generateExpression(expr->array.get()) + "[" + generateExpression(expr->index.get()) + "]";
}

std::string CodeGenerator::generateMapLiteral(const MapLiteral* expr) {
    std::stringstream ss;
    std::string valueType = typeToCppType(expr->valueType);
    if (expr->values.empty() && expr->valueType == Type::ANY) {
        valueType = "std::string"; // Default for empty map?
    }
    ss << "Map<std::string, " << valueType << ">(std::map<std::string, " << valueType << ">{";
    for (size_t i = 0; i < expr->keys.size(); i++) {
        if (i > 0) ss << ", ";
        ss << "{\"" << expr->keys[i] << "\", " << generateExpression(expr->values[i].get()) << "}";
    }
    ss << "})";
    return ss.str();
}
std::string CodeGenerator::generateMemberExpression(const MemberExpression* expr) {
    if (auto id = dynamic_cast<const Identifier*>(expr->object.get())) {
        static const std::vector<std::string> staticClasses = {
            "Math", "String", "Date", "JSON", "File", "Console", 
            "HTTP", "Regex", "Env", "Thread", "Process", "Timer", "Database"
        };
        for (const auto& className : staticClasses) {
            if (id->name == className) {
                return className + "::" + expr->property;
            }
        }
    }
    if (expr->property == "length") {
        return generateExpression(expr->object.get()) + ".length()";
    }
    if (auto id = dynamic_cast<const Identifier*>(expr->object.get())) {
        if (id->name == "this") {
            return "this->" + expr->property;
        }
    }
    return generateExpression(expr->object.get()) + "." + expr->property;
}
std::string CodeGenerator::generateVariableDeclaration(const VariableDeclaration* decl) {
    std::stringstream ss;
    ss << indent();
    if (decl->isConst) {
        ss << "const ";
    }
    ss << typeToCppType(decl->varType) << " " << decl->name;
    if (decl->initializer) {
        ss << " = " << generateExpression(decl->initializer.get());
    }
    ss << ";\n";
    declaredVariables.insert(decl->name);
    variableTypes[decl->name] = decl->varType;
    return ss.str();
}
std::string CodeGenerator::generateFunctionDeclaration(const FunctionDeclaration* decl) {
    std::stringstream ss;
    std::string returnType = typeToCppType(decl->returnType);
    if (decl->name == "main") {
        returnType = "int";
    }
    ss << indent() << returnType << " " << decl->name << "(";
    for (size_t i = 0; i < decl->parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << typeToCppType(decl->parameters[i].type) << " " << decl->parameters[i].name;
    }
    ss << ") {\n";
    indentLevel++;
    for (const auto& stmt : decl->body) {
        ss << generateStatement(stmt.get());
    }
    indentLevel--;
    ss << indent() << "}\n\n";
    return ss.str();
}

std::string CodeGenerator::generateFunctionExpression(const FunctionExpression* expr) {
    std::stringstream ss;
    ss << "[&](";
    for (size_t i = 0; i < expr->parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << typeToCppType(expr->parameters[i].type) << " " << expr->parameters[i].name;
    }
    ss << ") -> " << typeToCppType(expr->returnType) << " {\n";
    indentLevel++;
    for (const auto& stmt : expr->body) {
        ss << generateStatement(stmt.get());
    }
    indentLevel--;
    ss << indent() << "}";
    return ss.str();
}

std::string CodeGenerator::generateClassDeclaration(const ClassDeclaration* decl) {
    std::stringstream ss;
    ss << indent() << "struct " << decl->name;
    if (!decl->superclass.empty()) {
        ss << " : public " << decl->superclass;
    }
    ss << " {\n";
    indentLevel++;
    
    // Fields
    for (const auto& member : decl->members) {
        ss << indent() << typeToCppType(member.type) << " " << member.name;
        if (member.initializer) {
            ss << " = " << generateExpression(member.initializer.get());
        }
        ss << ";\n";
    }

    // Constructor
    if (decl->constructor) {
        ss << "\n" << indent() << decl->name << "(";
        for (size_t i = 0; i < decl->constructor->parameters.size(); i++) {
            if (i > 0) ss << ", ";
            ss << typeToCppType(decl->constructor->parameters[i].type) << " " << decl->constructor->parameters[i].name;
        }
        ss << ") {\n";
        indentLevel++;
        for (const auto& stmt : decl->constructor->body) {
            ss << generateStatement(stmt.get());
        }
        indentLevel--;
        ss << indent() << "}\n";
    }

    // Methods
    for (const auto& method : decl->methods) {
        ss << "\n" << indent() << typeToCppType(method.returnType) << " " << method.name << "(";
        for (size_t i = 0; i < method.parameters.size(); i++) {
            if (i > 0) ss << ", ";
            ss << typeToCppType(method.parameters[i].type) << " " << method.parameters[i].name;
        }
        ss << ") {\n";
        indentLevel++;
        for (const auto& stmt : method.body) {
            ss << generateStatement(stmt.get());
        }
        indentLevel--;
        ss << indent() << "}\n";
    }

    indentLevel--;
    ss << indent() << "};\n\n";
    return ss.str();
}

std::string CodeGenerator::generateReturnStatement(const ReturnStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "return";
    if (stmt->value) {
        ss << " " << generateExpression(stmt->value.get());
    }
    ss << ";\n";
    return ss.str();
}
std::string CodeGenerator::generateIfStatement(const IfStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "if (" << generateExpression(stmt->condition.get()) << ") {\n";
    indentLevel++;
    for (const auto& s : stmt->thenBranch) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "}";
    if (!stmt->elseBranch.empty()) {
        ss << " else {\n";
        indentLevel++;
        for (const auto& s : stmt->elseBranch) {
            ss << generateStatement(s.get());
        }
        indentLevel--;
        ss << indent() << "}";
    }
    ss << "\n";
    return ss.str();
}
std::string CodeGenerator::generateWhileStatement(const WhileStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "while (" << generateExpression(stmt->condition.get()) << ") {\n";
    indentLevel++;
    for (const auto& s : stmt->body) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "}\n";
    return ss.str();
}
std::string CodeGenerator::generateForStatement(const ForStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "for (";
    if (stmt->initializer) {
        std::string init = generateStatement(stmt->initializer.get());
        size_t start = init.find_first_not_of(" \t");
        size_t end = init.find_last_not_of(" \t\n;");
        if (start != std::string::npos && end != std::string::npos) {
            ss << init.substr(start, end - start + 1);
        }
    }
    ss << "; ";
    if (stmt->condition) {
        ss << generateExpression(stmt->condition.get());
    }
    ss << "; ";
    if (stmt->increment) {
        ss << generateExpression(stmt->increment.get());
    }
    ss << ") {\n";
    indentLevel++;
    for (const auto& s : stmt->body) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "}\n";
    return ss.str();
}
std::string CodeGenerator::generateBlockStatement(const BlockStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "{\n";
    indentLevel++;
    for (const auto& s : stmt->statements) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "}\n";
    return ss.str();
}
std::string CodeGenerator::generateExpressionStatement(const ExpressionStatement* stmt) {
    return indent() + generateExpression(stmt->expression.get()) + ";\n";
}
std::string CodeGenerator::generateNumberLiteral(const NumberLiteral* expr) {
    return std::to_string(expr->value);
}
std::string CodeGenerator::generateStringLiteral(const StringLiteral* expr) {
    return "std::string(\"" + escapeString(expr->value) + "\")";
}
std::string CodeGenerator::generateBooleanLiteral(const BooleanLiteral* expr) {
    return expr->value ? "true" : "false";
}
std::string CodeGenerator::generateIdentifier(const Identifier* expr) {
    return expr->name;
}
std::string CodeGenerator::generateBinaryExpression(const BinaryExpression* expr) {
    std::stringstream ss;
    std::string left = generateExpression(expr->left.get());
    std::string right = generateExpression(expr->right.get());
    if (expr->op == "+") {
        bool leftIsString = dynamic_cast<const StringLiteral*>(expr->left.get()) != nullptr;
        bool rightIsString = dynamic_cast<const StringLiteral*>(expr->right.get()) != nullptr;
        if (leftIsString || rightIsString || 
            left.find("toString") != std::string::npos || 
            right.find("toString") != std::string::npos) {
            ss << "(" << left << " + " << right << ")";
            return ss.str();
        }
    }
    ss << "(" << left << " " << expr->op << " " << right << ")";
    return ss.str();
}
std::string CodeGenerator::generateUnaryExpression(const UnaryExpression* expr) {
    return "(" + expr->op + generateExpression(expr->operand.get()) + ")";
}
std::string CodeGenerator::generateCallExpression(const CallExpression* expr) {
    std::stringstream ss;
    if (auto id = dynamic_cast<const Identifier*>(expr->callee.get())) {
        if (id->name == "print" || id->name == "println") {
            ss << "std::cout";
            for (const auto& arg : expr->arguments) {
                ss << " << " << generateExpression(arg.get());
            }
            if (id->name == "println") {
                ss << " << std::endl";
            }
            return ss.str();
        }
    }
    ss << generateExpression(expr->callee.get()) << "(";
    for (size_t i = 0; i < expr->arguments.size(); i++) {
        if (i > 0) ss << ", ";
        ss << generateExpression(expr->arguments[i].get());
    }
    ss << ")";
    return ss.str();
}
std::string CodeGenerator::generateArrayExpression(const ArrayExpression* expr) {
    std::stringstream ss;
    std::string cppType = typeToCppType(expr->elementType);
    if (expr->elements.empty()) {
        // Default to double if empty, or perhaps ANY/auto might be tricky in C++
        // Let's default to double for empty arrays for now to maintain behavior
        // Or if elementType was set to ANY (default), maybe we should default to double?
        if (expr->elementType == Type::ANY) {
             cppType = "double";
        }
    }
    ss << "Array<" << cppType << ">(std::vector<" << cppType << ">{";
    for (size_t i = 0; i < expr->elements.size(); i++) {
        if (i > 0) ss << ", ";
        ss << generateExpression(expr->elements[i].get());
    }
    ss << "})";
    return ss.str();
}
std::string CodeGenerator::typeToCppType(Type type) {
    switch (type) {
        case Type::NUMBER: return "double";
        case Type::STRING: return "std::string";
        case Type::BOOLEAN: return "bool";
        case Type::VOID: return "void";
        case Type::ANY: return "auto";
        default: return "auto";
    }
}
std::string CodeGenerator::escapeString(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        switch (c) {
            case '\n': ss << "\\n"; break;
            case '\t': ss << "\\t"; break;
            case '\r': ss << "\\r"; break;
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            default: ss << c;
        }
    }
    return ss.str();
}
std::string CodeGenerator::indent() {
    return std::string(indentLevel * 4, ' ');
}
}  
