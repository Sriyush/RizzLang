#include "interpreter.hpp"
#include <iostream>
#include <stdexcept>

void Interpreter::execute(const std::vector<std::shared_ptr<ASTNode>>& statements) {
    for (auto& stmt : statements) {
        executeNode(stmt);
    }
}

void Interpreter::executeNode(const std::shared_ptr<ASTNode>& node) {
    switch (node->type) {
        case ASTNodeType::PRINT_STMT: {
            auto stmt = std::dynamic_pointer_cast<PrintStmt>(node);
            auto val = evalExpression(stmt->value);
            if (std::holds_alternative<int>(val))
                std::cout << std::get<int>(val) << std::endl;
            else if (std::holds_alternative<std::string>(val))
                std::cout << std::get<std::string>(val) << std::endl;
            else if (std::holds_alternative<bool>(val))
                std::cout << (std::get<bool>(val) ? "true" : "false") << std::endl;
            break;
        }

        case ASTNodeType::ASSIGN_STMT: {
            auto stmt = std::dynamic_pointer_cast<AssignStmt>(node);
            auto val = evalExpression(stmt->value);
            variables[stmt->name] = val;
            break;
        }

        case ASTNodeType::IF_STMT: {
            auto stmt = std::dynamic_pointer_cast<IfStmt>(node);
            auto cond = evalExpression(stmt->condition);
            if (std::holds_alternative<int>(cond) && std::get<int>(cond)) {
                for (auto& child : stmt->body) {
                    executeNode(child);
                }
            } else if (std::holds_alternative<bool>(cond) && std::get<bool>(cond)) {
                for (auto& child : stmt->body) {
                    executeNode(child);
                }
            }
            break;
        }

        default:
            throw std::runtime_error("Unknown AST node type in interpreter");
    }
}

Interpreter::Value Interpreter::evalExpression(const std::shared_ptr<ASTNode>& node) {
    switch (node->type) {
        case ASTNodeType::NUMBER: {
            auto expr = std::dynamic_pointer_cast<NumberExpr>(node);
            return expr->value;
        }
        case ASTNodeType::STRING: {
            auto expr = std::dynamic_pointer_cast<StringExpr>(node);
            return expr->value;
        }
        case ASTNodeType::IDENT: {
            auto expr = std::dynamic_pointer_cast<IdentExpr>(node);
            if (variables.find(expr->name) == variables.end())
                throw std::runtime_error("Undefined variable: " + expr->name);
            return variables[expr->name];
        }
        case ASTNodeType::BINARY_EXPR: {
            auto expr = std::dynamic_pointer_cast<BinaryExpr>(node);
            auto left = evalExpression(expr->left);
            auto right = evalExpression(expr->right);

            if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
                int l = std::get<int>(left);
                int r = std::get<int>(right);
                if (expr->op == ">") return l > r;
                if (expr->op == "<") return l < r;
                if (expr->op == "==") return l == r;
                throw std::runtime_error("Unknown binary operator: " + expr->op);
            }
            throw std::runtime_error("Invalid operands for binary operator: " + expr->op);
        }
        default:
            throw std::runtime_error("Unknown expression node in interpreter");
    }
}
