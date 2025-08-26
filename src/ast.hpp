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
    IDENT,
    NUMBER,
    STRING,
    BINARY_EXPR,
};

struct ASTNode {
    ASTNodeType type;
    virtual ~ASTNode() = default;
};

// Print statement → bruh "hello"
struct PrintStmt : public ASTNode {
    std::shared_ptr<ASTNode> value;
    PrintStmt(std::shared_ptr<ASTNode> v) { 
        type = ASTNodeType::PRINT_STMT; 
        value = v; 
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
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    IfStmt(std::shared_ptr<ASTNode> cond) {
        type = ASTNodeType::IF_STMT;
        condition = cond;
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
