#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
namespace umbrella {
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};
enum class Type {
    NUMBER,
    STRING,
    BOOLEAN,
    VOID,
    ANY,
    FUNCTION,
    ARRAY,
    CLASS
};
std::string typeToString(Type type);
class Expression : public ASTNode {
public:
    Type type = Type::ANY;
};
class NumberLiteral : public Expression {
public:
    double value;
    NumberLiteral(double val) : value(val) { type = Type::NUMBER; }
    std::string toString() const override;
};
class StringLiteral : public Expression {
public:
    std::string value;
    StringLiteral(const std::string& val) : value(val) { type = Type::STRING; }
    std::string toString() const override;
};
class BooleanLiteral : public Expression {
public:
    bool value;
    BooleanLiteral(bool val) : value(val) { type = Type::BOOLEAN; }
    std::string toString() const override;
};
class Identifier : public Expression {
public:
    std::string name;
    Identifier(const std::string& n) : name(n) {}
    std::string toString() const override;
};
class BinaryExpression : public Expression {
public:
    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    BinaryExpression(const std::string& operation,
                     std::unique_ptr<Expression> l,
                     std::unique_ptr<Expression> r)
        : op(operation), left(std::move(l)), right(std::move(r)) {}
    std::string toString() const override;
};
class UnaryExpression : public Expression {
public:
    std::string op;
    std::unique_ptr<Expression> operand;
    UnaryExpression(const std::string& operation, std::unique_ptr<Expression> expr)
        : op(operation), operand(std::move(expr)) {}
    std::string toString() const override;
};
class CallExpression : public Expression {
public:
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;
    CallExpression(std::unique_ptr<Expression> c)
        : callee(std::move(c)) {}
    std::string toString() const override;
};
class ArrayExpression : public Expression {
public:
    std::vector<std::unique_ptr<Expression>> elements;
    Type elementType = Type::ANY;
    std::string toString() const override;
};
class ArrayAccess : public Expression {
public:
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;
    ArrayAccess(std::unique_ptr<Expression> arr, std::unique_ptr<Expression> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
    std::string toString() const override;
};
class MemberExpression : public Expression {
public:
    std::unique_ptr<Expression> object;
    std::string property;
    MemberExpression(std::unique_ptr<Expression> obj, const std::string& prop)
        : object(std::move(obj)), property(prop) {}
    std::string toString() const override;
};
class NewExpression : public Expression {
public:
    std::string className;
    std::vector<std::unique_ptr<Expression>> arguments;
    NewExpression(const std::string& name) : className(name) {}
    std::string toString() const override;
};
class ConditionalExpression : public Expression {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> thenExpr;
    std::unique_ptr<Expression> elseExpr;
    ConditionalExpression(std::unique_ptr<Expression> cond,
                         std::unique_ptr<Expression> then,
                         std::unique_ptr<Expression> els)
        : condition(std::move(cond)), thenExpr(std::move(then)), elseExpr(std::move(els)) {}
    std::string toString() const override;
};
class AssignmentExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::string op;  
    std::unique_ptr<Expression> right;
    AssignmentExpression(std::unique_ptr<Expression> l, const std::string& operation,
                        std::unique_ptr<Expression> r)
        : left(std::move(l)), op(operation), right(std::move(r)) {}
    std::string toString() const override;
};
class Statement : public ASTNode {};
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    ExpressionStatement(std::unique_ptr<Expression> expr)
        : expression(std::move(expr)) {}
    std::string toString() const override;
};
class VariableDeclaration : public Statement {
public:
    std::string name;
    Type varType;
    std::unique_ptr<Expression> initializer;
    bool isConst;
    VariableDeclaration(const std::string& n, Type t, 
                       std::unique_ptr<Expression> init, bool constant = false)
        : name(n), varType(t), initializer(std::move(init)), isConst(constant) {}
    std::string toString() const override;
};
class FunctionParameter {
public:
    std::string name;
    Type type;
    FunctionParameter(const std::string& n, Type t) : name(n), type(t) {}
};
class FunctionDeclaration : public Statement {
public:
    std::string name;
    std::vector<FunctionParameter> parameters;
    Type returnType;
    std::vector<std::unique_ptr<Statement>> body;
    FunctionDeclaration(const std::string& n, Type ret)
        : name(n), returnType(ret) {}
    std::string toString() const override;
};
class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;
    ReturnStatement(std::unique_ptr<Expression> val = nullptr)
        : value(std::move(val)) {}
    std::string toString() const override;
};
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch;
    IfStatement(std::unique_ptr<Expression> cond)
        : condition(std::move(cond)) {}
    std::string toString() const override;
};
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
    WhileStatement(std::unique_ptr<Expression> cond)
        : condition(std::move(cond)) {}
    std::string toString() const override;
};
class ForStatement : public Statement {
public:
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::vector<std::unique_ptr<Statement>> body;
    std::string toString() const override;
};
class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    std::string toString() const override;
};
class ClassMember {
public:
    std::string name;
    Type type;
    std::unique_ptr<Expression> initializer;
    ClassMember(const std::string& n, Type t, std::unique_ptr<Expression> init = nullptr)
        : name(n), type(t), initializer(std::move(init)) {}
};
class MethodDeclaration {
public:
    std::string name;
    std::vector<FunctionParameter> parameters;
    Type returnType;
    std::vector<std::unique_ptr<Statement>> body;
    MethodDeclaration(const std::string& n, Type ret)
        : name(n), returnType(ret) {}
};
class ConstructorDeclaration {
public:
    std::vector<FunctionParameter> parameters;
    std::vector<std::unique_ptr<Statement>> body;
};
class ClassDeclaration : public Statement {
public:
    std::string name;
    std::string superclass;
    std::vector<ClassMember> members;
    std::vector<MethodDeclaration> methods;
    std::unique_ptr<ConstructorDeclaration> constructor;
    ClassDeclaration(const std::string& n) : name(n) {}
    std::string toString() const override;
};
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    std::string toString() const override;
};
}  
