#include "codegen.h"
#include <sstream>
#include <iostream> // Added
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
    ss << "using namespace umbrella::runtime;\n\n";

    std::stringstream mainBody;
    std::stringstream declarations;
    bool hasUserMain = false;

    // Separate declarations from executable statements
    for (const auto& stmt : program.statements) {
        if (dynamic_cast<const FunctionDeclaration*>(stmt.get()) || 
            dynamic_cast<const ClassDeclaration*>(stmt.get()) ||
            dynamic_cast<const VariableDeclaration*>(stmt.get())) {
            
            if (auto func = dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
                if (func->name == "main") hasUserMain = true;
            }
            declarations << generateStatement(stmt.get());
        } else {
             // Executable statements go to main
             // Variables at top level are tricky: global or local to main?
             // For script-like behavior, they should be in main.
             // If they are strictly global vars, C++ requires them outside.
             // But Umbrella scripts allow "let x = 10; print(x)". x must be visible.
             // If we put them in main, it works.
             // Exception: VariableDeclaration might be intended as global? 
             // Simplification: Treat all non-func/class definitions as main body Code.
             // If main already exists, this might be an issue. 
             // If main exists, top-level code outside main is generally invalid in C++ (except global vars).
             // Let's assume if there are top-level statements, valid Umbrella code implies they run? 
             // If user defined main(), they probably shouldn't have loose code. 
             // But let's append loosely to declarations if it is a variable? No.
             // Let's accumulate them for a generated main.
             mainBody << generateStatement(stmt.get());
        }
    }

    ss << declarations.str();

    if (!hasUserMain) {
        ss << "int main() {\n";
        // Indent main body
        std::string body = mainBody.str();
        // Naive indentation or assumes generateStatement does it? 
        // generateStatement uses 'indent()'. we should probably set indentLevel 1 before generating?
        // But we are generating into a separate stream.
        // Let's just output raw for now or fixing indentation would be nice but not critical for compilation.
        ss << body; 
        ss << "    return 0;\n";
        ss << "}\n";
    } else {
        // mixed mode? If user has main, but also top-statements.
        // In C++, those statements would be illegal at file scope (except var decls).
        // We will just put them in global scope and let C++ compile or fail (as it did before).
        // Actually, previous implementation just looped and outputted everything.
        // Reverting to global output if main exists might be safer if they are truly global variables?
        if (mainBody.tellp() > 0) {
             // If we have content that isn't decls, and we have a main...
             // It's likely global variables.
             ss << mainBody.str();
        }
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
    if (auto throwStmt = dynamic_cast<const ThrowStatement*>(stmt)) {
        return generateThrowStatement(throwStmt);
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
    if (auto condExpr = dynamic_cast<const ConditionalExpression*>(expr)) {
        return generateConditionalExpression(condExpr);
    }
    return "";
}

std::string CodeGenerator::generateConditionalExpression(const ConditionalExpression* expr) {
    return "(" + generateExpression(expr->condition.get()) + " ? " + 
           generateExpression(expr->thenExpr.get()) + " : " + 
           generateExpression(expr->elseExpr.get()) + ")";
}

std::string CodeGenerator::generateThrowStatement(const ThrowStatement* stmt) {
    return indent() + "throw " + generateExpression(stmt->expression.get()) + ";\n";
}

std::string CodeGenerator::generateTryStatement(const TryStatement* stmt) {
    std::stringstream ss;
    ss << indent() << "{\n"; // Enclose in block for scope
    indentLevel++;
    
    // Handle finally using RAII
    if (!stmt->finallyBlock.empty()) {
        ss << indent() << "struct Finally {\n";
        ss << indent() << "    std::function<void()> f;\n";
        ss << indent() << "    Finally(std::function<void()> func) : f(func) {}\n";
        ss << indent() << "    ~Finally() { f(); }\n";
        ss << indent() << "} _finally([&]() {\n";
        indentLevel++;
        for (const auto& s : stmt->finallyBlock) {
            ss << generateStatement(s.get());
        }
        indentLevel--;
        ss << indent() << "});\n";
    }

    ss << indent() << "try {\n";
    indentLevel++;
    for (const auto& s : stmt->tryBlock) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "} catch (const std::string& " << stmt->catchVar << ") { // Catch string exceptions\n";
    indentLevel++;
    for (const auto& s : stmt->catchBlock) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "} catch (const char* " << stmt->catchVar << "_ctr) { // Catch const char* exceptions\n";
    indentLevel++;
    ss << indent() << "std::string " << stmt->catchVar << "(" << stmt->catchVar << "_ctr);\n";
    for (const auto& s : stmt->catchBlock) {
        ss << generateStatement(s.get());
    }
    indentLevel--;
    ss << indent() << "} catch (...) {\n";
    if (!stmt->catchVar.empty()) {
        indentLevel++;
        ss << indent() << "std::string " << stmt->catchVar << " = \"Unknown error\";\n";
        for (const auto& s : stmt->catchBlock) {
            ss << generateStatement(s.get());
        }
        indentLevel--;
    }
    ss << indent() << "}\n";
    
    indentLevel--;
    ss << indent() << "}\n";
    return ss.str();
}

std::string CodeGenerator::generateAssignmentExpression(const AssignmentExpression* expr) {
    std::stringstream ss;
    std::string left = generateExpression(expr->left.get());
    std::string right = generateExpression(expr->right.get());
    
    // Handle bitwise compound assignment: a &= b -> a = (long long)a & (long long)b
    if (expr->op == "&=" || expr->op == "|=" || expr->op == "^=" || 
        expr->op == "<<=" || expr->op == ">>=") {
        std::string baseOp = expr->op.substr(0, expr->op.length() - 1); // remove =
        ss << left << " = ((long long)" << left << " " << baseOp << " (long long)" << right << ")";
    } else {
        ss << left << " " << expr->op << " " << right;
    }
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
    std::string safeName = sanitize(decl->name);
    
    // Use explicitly captured type if available (handles Generics like Array<Thread>)
    if (!decl->cppType.empty()) {
        ss << decl->cppType << " " << safeName;
    } else {
        if (decl->varType != Type::ANY) {
            ss << typeToCppType(decl->varType) << " " << safeName;
        } else {
            ss << "auto " << safeName;
        }
    }

    if (decl->initializer) {
        // Special handling for empty array literals to avoid defaulting to double
        bool isEmptyArray = false;
        if (auto arrExpr = dynamic_cast<const ArrayExpression*>(decl->initializer.get())) {
            if (arrExpr->elements.empty()) {
                isEmptyArray = true;
            }
        }
        
        bool isEmptyGenericCtor = false;
        if (!decl->cppType.empty()) {
            if (auto newExpr = dynamic_cast<const NewExpression*>(decl->initializer.get())) {
                if (newExpr->arguments.empty() && decl->cppType.find(newExpr->className) == 0) {
                     isEmptyGenericCtor = true;
                }
            }
        }

        if (isEmptyArray && !decl->cppType.empty()) {
             // Array<Thread> threads = {};
             ss << " = {}"; 
        } else if (isEmptyGenericCtor) {
             // Map<K,V> m; instead of = Map();
        } else {
             ss << " = " << generateExpression(decl->initializer.get());
        }
    }
    ss << ";\n";
    declaredVariables.insert(decl->name);
    variableTypes[decl->name] = decl->varType;
    return ss.str();
}
std::string CodeGenerator::generateFunctionDeclaration(const FunctionDeclaration* decl) {
    std::stringstream ss;
    std::string returnType = typeToCppType(decl->returnType);
    std::string safeName = sanitize(decl->name);
    if (decl->name == "main") {
        returnType = "int";
        safeName = "main"; // Don't sanitize main
    }
    ss << indent() << returnType << " " << safeName << "(";
    for (size_t i = 0; i < decl->parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << typeToCppType(decl->parameters[i].type) << " " << sanitize(decl->parameters[i].name);
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
    ss << "[=](";
    for (size_t i = 0; i < expr->parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << typeToCppType(expr->parameters[i].type) << " " << sanitize(expr->parameters[i].name);
    }
    ss << ") mutable -> " << typeToCppType(expr->returnType) << " {\n"; // Added mutable
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
std::string CodeGenerator::sanitize(const std::string& name) {
    static const std::set<std::string> keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", 
        "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", 
        "class", "compl", "concept", "const", "constexpr", "const_cast", "continue", "co_await", 
        "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", 
        "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", 
        "if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept", 
        "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", 
        "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", 
        "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", 
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", 
        "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
    if (keywords.count(name)) {
        return name + "_";
    }
    return name;
}

std::string CodeGenerator::generateIdentifier(const Identifier* expr) {
    return sanitize(expr->name);
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
            right.find("toString") != std::string::npos ||
            left.find("std::string") != std::string::npos || 
            right.find("std::string") != std::string::npos) { // Added more checks for string concat
            ss << "(" << left << " + " << right << ")";
            return ss.str();
        }
    }
    
    // Bitwise operators require integer operands in C++
    if (expr->op == "&" || expr->op == "|" || expr->op == "^" || expr->op == "<<" || expr->op == ">>") {
         ss << "((long long)" << left << " " << expr->op << " (long long)" << right << ")";
         return ss.str();
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

    // Handle string instance methods via static helpers in runtime::String
    if (auto member = dynamic_cast<const MemberExpression*>(expr->callee.get())) {
        const std::string& method = member->property;
        std::string objectCode = generateExpression(member->object.get());

        auto joinArgs = [&](size_t startIndex = 0) {
            std::stringstream argsSs;
            for (size_t i = startIndex; i < expr->arguments.size(); ++i) {
                if (i > startIndex) argsSs << ", ";
                argsSs << generateExpression(expr->arguments[i].get());
            }
            return argsSs.str();
        };

        if (method == "toUpperCase") {
            ss << "String::toUpperCase(" << objectCode << ")";
            return ss.str();
        }
        if (method == "toLowerCase") {
            ss << "String::toLowerCase(" << objectCode << ")";
            return ss.str();
        }
        if (method == "substring") {
            ss << "String::substring(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "indexOf") {
            ss << "String::indexOf(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "replace") {
            ss << "String::replace(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "split") {
            ss << "String::split(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "trim") {
            ss << "String::trim(" << objectCode << ")";
            return ss.str();
        }
        if (method == "startsWith") {
            ss << "String::startsWith(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "endsWith") {
            ss << "String::endsWith(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "repeat") {
            ss << "String::repeat(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "padStart") {
            ss << "String::padStart(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
            return ss.str();
        }
        if (method == "padEnd") {
            ss << "String::padEnd(" << objectCode;
            if (!expr->arguments.empty()) {
                ss << ", " << joinArgs(0);
            }
            ss << ")";
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
        case Type::FUNCTION: return "auto";
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
