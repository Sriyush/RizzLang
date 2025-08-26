#include "interpreter.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
void Interpreter::execute(const std::vector<std::shared_ptr<ASTNode>> &statements)
{
    for (auto &stmt : statements)
    {
        executeNode(stmt);
    }
}

void Interpreter::executeNode(const std::shared_ptr<ASTNode> &node)
{
    switch (node->type)
    {
    case ASTNodeType::PRINT_STMT:
    {
        auto stmt = std::dynamic_pointer_cast<PrintStmt>(node);
        auto val = evalExpression(stmt->value);
        if (std::holds_alternative<double>(val))
            std::cout << std::get<double>(val) << std::endl;
        else if (std::holds_alternative<std::string>(val))
            std::cout << std::get<std::string>(val) << std::endl;
        else if (std::holds_alternative<bool>(val))
            std::cout << (std::get<bool>(val) ? "no_cap" : "cap") << std::endl;
        break;
    }

    case ASTNodeType::ASSIGN_STMT:
    {
        auto stmt = std::dynamic_pointer_cast<AssignStmt>(node);
        auto val = evalExpression(stmt->value);
        variables[stmt->name] = val;
        break;
    }

    case ASTNodeType::IF_STMT:
    {
        auto stmt = std::dynamic_pointer_cast<IfStmt>(node);
        auto cond = evalExpression(stmt->condition);
        if (std::holds_alternative<double>(cond) && std::get<double>(cond))
        {
            for (auto &child : stmt->body)
            {
                executeNode(child);
            }
        }
        else if (std::holds_alternative<bool>(cond) && std::get<bool>(cond))
        {
            for (auto &child : stmt->body)
            {
                executeNode(child);
            }
        }
        break;
    }

    default:
        throw std::runtime_error("Unknown AST node type in interpreter");
    }
}

Interpreter::Value Interpreter::evalExpression(const std::shared_ptr<ASTNode> &node)
{
    switch (node->type)
    {
    case ASTNodeType::NUMBER:
    {
        auto expr = std::dynamic_pointer_cast<NumberExpr>(node);
        return expr->value;
    }
    case ASTNodeType::STRING:
    {
        auto expr = std::dynamic_pointer_cast<StringExpr>(node);
        return expr->value;
    }
    case ASTNodeType::IDENT:
    {
        auto expr = std::dynamic_pointer_cast<IdentExpr>(node);
        if (variables.find(expr->name) == variables.end())
            throw std::runtime_error("Undefined variable: " + expr->name);
        return variables[expr->name];
    }
    case ASTNodeType::BINARY_EXPR:
    {
        auto expr = std::dynamic_pointer_cast<BinaryExpr>(node);
        auto left = evalExpression(expr->left);
        auto right = evalExpression(expr->right);

        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            double l = std::get<double>(left);
            double r = std::get<double>(right);
            if (expr->op == ">")
                return (l > r);
            if (expr->op == "<")
                return (l < r);
            if (expr->op == "==")
                return (l == r);
            if (expr->op == "!=")
                return (l != r);
            if (expr->op == ">=")
                return (l >= r);
            if (expr->op == "<=")
                return (l <= r);

            if (expr->op == "+")
                return l + r;
            if (expr->op == "-")
                return l - r;
            if (expr->op == "*")
                return l * r;
            if (expr->op == "/")
                return l / r;
            if (expr->op == "%")
                return std::fmod(l, r);

            throw std::runtime_error("Unknown binary operator: " + expr->op);
        }

        throw std::runtime_error("Invalid operands for binary operator: " + expr->op);
    }
    case ASTNodeType::UNARY_EXPR:
    {
        auto expr = std::dynamic_pointer_cast<UnaryExpr>(node);
        auto val = evalExpression(expr->operand);

        if (expr->op == "!")
        {
            if (std::holds_alternative<bool>(val))
                return !std::get<bool>(val);
            if (std::holds_alternative<double>(val))
                return std::get<double>(val) == 0.0;
            throw std::runtime_error("Invalid operand type for '!'");
        }

        if (expr->op == "-")
        {
            if (std::holds_alternative<double>(val))
                return -std::get<double>(val);
            throw std::runtime_error("Invalid operand type for unary '-'");
        }

        throw std::runtime_error("Unknown unary operator: " + expr->op);
    }
case ASTNodeType::INDEX_EXPR:
{
    auto idx = std::dynamic_pointer_cast<IndexExpr>(node);

    auto targetVal = evalExpression(idx->target);
    auto indexVal = evalExpression(idx->index);

    if (!std::holds_alternative<double>(indexVal))
    {
        throw std::runtime_error("Index must be a number");
    }
    int i = static_cast<int>(std::get<double>(indexVal));

    // String indexing
    if (std::holds_alternative<std::string>(targetVal))
    {
        const std::string &s = std::get<std::string>(targetVal);

        // 🔥 backward indexing
        if (i < 0) {
            i = static_cast<int>(s.size()) + i;
        }

        if (i < 0 || i >= (int)s.size())
        {
            throw std::runtime_error("String index out of range");
        }
        return std::string(1, s[i]);
    }
    // Array indexing
    else if (std::holds_alternative<std::vector<Value>>(targetVal))
    {
        const auto &arr = std::get<std::vector<Value>>(targetVal);

        // 🔥 backward indexing
        if (i < 0) {
            i = static_cast<int>(arr.size()) + i;
        }

        if (i < 0 || i >= (int)arr.size())
        {
            throw std::runtime_error("Array index out of range");
        }
        return arr[i];
    }
    else
    {
        throw std::runtime_error("Indexing not supported on this type");
    }
}

    default:
        throw std::runtime_error("Unknown expression node in interpreter");
    }
}
