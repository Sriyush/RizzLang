// src/ast.hpp
//This is Abstract syntax tree 
#pragma once
#include <string>
#include <vector>
#include <memory>

enum class ASTNodeType {
    PRINT_STMT,
    IF_STMT,
    ASSIGN_STMT,
    EXPR_STMT,
    INPUT_STMT, 
    IDENT,
    NUMBER,
    STRING,
    BINARY_EXPR,
    UNARY_EXPR,
    INDEX_EXPR,
    FUNC_DEF,
    CLASS_DEF,
    RETURN_STMT,
    CALL_EXPR,
    NEW_OBJECT_EXPR,
    METHOD_CALL_EXPR,
    // PROPERTY_ACCESS
    MEMBER_ACCESS_EXPR
};


struct ASTNode {
    ASTNodeType type;
    virtual ~ASTNode() = default;
};
struct ExprStmt : public ASTNode {
    std::shared_ptr<ASTNode> expr;
    ExprStmt(std::shared_ptr<ASTNode> e) { 
        type = ASTNodeType::EXPR_STMT; 
        expr = e; 
    }
};

// Print statement → bruh "hello"
struct PrintStmt : public ASTNode {
    std::shared_ptr<ASTNode> value;
    PrintStmt(std::shared_ptr<ASTNode> v) { 
        type = ASTNodeType::PRINT_STMT; 
        value = v; 
    }
};
struct InputStmt : public ASTNode {
    std::string varName;
    InputStmt(const std::string& var) {
        type = ASTNodeType::INPUT_STMT;  // 👈 important
        varName = var;
    }
};

// Assignment → x = 10
struct AssignStmt : public ASTNode {
    std::string name;
    std::shared_ptr<ASTNode> value;
    AssignStmt(const std::string& n, std::shared_ptr<ASTNode> v) {
        type = ASTNodeType::ASSIGN_STMT;
        name = n;
        value = v;
    }
};

// If statement → sus x > 5: ...
struct IfStmt : public ASTNode {
    std::shared_ptr<ASTNode> condition;                     // nullptr means ELSE
    std::vector<std::shared_ptr<ASTNode>> thenBranch;       // body of this branch
    std::shared_ptr<IfStmt> next;                           // chained else-if or else

    IfStmt(std::shared_ptr<ASTNode> cond,
           std::vector<std::shared_ptr<ASTNode>> thenB,
           std::shared_ptr<IfStmt> nextNode = nullptr)
        : condition(std::move(cond)),
          thenBranch(std::move(thenB)),
          next(std::move(nextNode)) {
        type = ASTNodeType::IF_STMT;
    }
};

// Literals (numbers, strings, identifiers)
struct NumberExpr : public ASTNode {
    double value;
    NumberExpr(double v) { type = ASTNodeType::NUMBER; value = v; }
};

struct StringExpr : public ASTNode {
    std::string value;
    StringExpr(const std::string& v) { type = ASTNodeType::STRING; value = v; }
};

struct IdentExpr : public ASTNode {
    std::string name;
    IdentExpr(const std::string& n) { type = ASTNodeType::IDENT; name = n; }
};

// Binary expression (x > 5)
struct BinaryExpr : public ASTNode {
    std::shared_ptr<ASTNode> left;
    std::string op;
    std::shared_ptr<ASTNode> right;
    BinaryExpr(std::shared_ptr<ASTNode> l, const std::string& o, std::shared_ptr<ASTNode> r) {
        type = ASTNodeType::BINARY_EXPR;
        left = l; op = o; right = r;
    }
};

struct UnaryExpr : public ASTNode {
    std::string op;  // e.g. "!"
    std::shared_ptr<ASTNode> operand;

    UnaryExpr(const std::string& op, std::shared_ptr<ASTNode> operand)
        : op(op), operand(std::move(operand)) {
        type = ASTNodeType::UNARY_EXPR;
    }
};

struct IndexExpr : public ASTNode {
    std::shared_ptr<ASTNode> target; // the array/string being indexed
    std::shared_ptr<ASTNode> index;  // the index expression

    IndexExpr(std::shared_ptr<ASTNode> t, std::shared_ptr<ASTNode> i) {
        type = ASTNodeType::INDEX_EXPR; // or better: add ASTNodeType::INDEX_EXPR
        target = std::move(t);
        index = std::move(i);
    }
};
// Function definition → drip foo(x, y): ...
struct FuncDef : public ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::shared_ptr<ASTNode>> body;

    FuncDef(const std::string &n,
            std::vector<std::string> p,
            std::vector<std::shared_ptr<ASTNode>> b) {
        type = ASTNodeType::FUNC_DEF;
        name = n;
        params = std::move(p);
        body = std::move(b);
    }
};

// Function call → foo(42, "yo")
struct CallExpr : public ASTNode {
    std::string callee;
    std::vector<std::shared_ptr<ASTNode>> args;

    CallExpr(const std::string &c, std::vector<std::shared_ptr<ASTNode>> a) {
        type = ASTNodeType::CALL_EXPR;
        callee = c;
        args = std::move(a);
    }
};

// Return statement
struct ReturnStmt : public ASTNode {
    std::shared_ptr<ASTNode> value;
    ReturnStmt(std::shared_ptr<ASTNode> v) {
        type = ASTNodeType::RETURN_STMT;
        value = v;
    }
};

// Class definition → rizz MyClass: ...
struct ClassDef : public ASTNode {
    std::string name;
    std::vector<std::shared_ptr<FuncDef>> methods;

    ClassDef(const std::string &n, std::vector<std::shared_ptr<FuncDef>> m) {
        type = ASTNodeType::CLASS_DEF;
        name = n;
        methods = std::move(m);
    }
};
struct NewObjectExpr : public ASTNode {
    std::string className;
    std::vector<std::shared_ptr<ASTNode>> args;

    NewObjectExpr(const std::string &c, std::vector<std::shared_ptr<ASTNode>> a) {
        type = ASTNodeType::NEW_OBJECT_EXPR;
        className = c;
        args = std::move(a);
    }
};

// Method call → obj.method(args...)
// Represents x.y (property access)
struct MemberAccessExpr : public ASTNode {
    std::shared_ptr<ASTNode> object;  // the "x"
    std::string member;               // the "y"

    MemberAccessExpr(std::shared_ptr<ASTNode> obj, const std::string &mem) {
        type = ASTNodeType::MEMBER_ACCESS_EXPR;
        object = std::move(obj);
        member = mem;
    }
};

// Represents x.func(args...)
struct MethodCallExpr : public ASTNode {
    std::shared_ptr<ASTNode> object;  // the "x"
    std::string method;               // the "func"
    std::vector<std::shared_ptr<ASTNode>> arguments;

    MethodCallExpr(std::shared_ptr<ASTNode> obj,
                   const std::string &meth,
                   std::vector<std::shared_ptr<ASTNode>> args) {
        type = ASTNodeType::METHOD_CALL_EXPR;
        object = std::move(obj);
        method = meth;
        arguments = std::move(args);
    }
};
