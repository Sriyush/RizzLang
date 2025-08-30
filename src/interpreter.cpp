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
        if (!stmt)
            throw std::runtime_error("Failed to cast node to IfStmt");

        auto current = stmt;

        while (current)
        {
            bool conditionResult = false;

            if (current->condition)
            {
                auto condVal = evalExpression(current->condition);

                if (std::holds_alternative<bool>(condVal))
                    conditionResult = std::get<bool>(condVal);
                else if (std::holds_alternative<double>(condVal))
                    conditionResult = (std::get<double>(condVal) != 0.0);
                else if (std::holds_alternative<std::string>(condVal))
                    conditionResult = !std::get<std::string>(condVal).empty();
                else
                    throw std::runtime_error("Unsupported type in if-condition");
            }
            else
            {
                conditionResult = true; // ELSE
            }

            if (conditionResult)
            {
                for (auto &child : current->thenBranch)
                    executeNode(child);
                return; // stop chain
            }

            current = current->next;
        }
        break;
    }

    case ASTNodeType::FUNC_DEF:
    {
        auto fn = std::dynamic_pointer_cast<FuncDef>(node);
        functions[fn->name] = fn;
        break;
    }

    case ASTNodeType::CLASS_DEF:
    {
        auto cl = std::dynamic_pointer_cast<ClassDef>(node);
        classes[cl->name] = cl;
        break;
    }

    case ASTNodeType::RETURN_STMT:
    {
        auto stmt = std::dynamic_pointer_cast<ReturnStmt>(node);
        throw evalExpression(stmt->value); // hack: throw value to unwind
    }

    case ASTNodeType::EXPR_STMT:
    {
        auto stmt = std::dynamic_pointer_cast<ExprStmt>(node);

        // ✅ Special case: if it's a call, execute it directly
        if (stmt->expr->type == ASTNodeType::CALL_EXPR)
            evalExpression(stmt->expr);
        else
            evalExpression(stmt->expr);

        break;
    }

    case ASTNodeType::INPUT_STMT:
        executeInput(std::static_pointer_cast<InputStmt>(node));
        break;

    default:
        throw std::runtime_error("Unknown AST node type in interpreter");
    }
}

void Interpreter::executeInput(const std::shared_ptr<InputStmt> &stmt)
{
    std::cout << "📝 spill " << stmt->varName << ": ";
    std::string userInput;
    std::getline(std::cin, userInput);

    try
    {
        double num = std::stod(userInput);
        variables[stmt->varName] = num;
    }
    catch (...)
    {
        variables[stmt->varName] = userInput;
    }
}

Interpreter::Value Interpreter::evalExpression(const std::shared_ptr<ASTNode> &node)
{
    switch (node->type)
    {
    case ASTNodeType::NUMBER:
        return std::dynamic_pointer_cast<NumberExpr>(node)->value;

    case ASTNodeType::STRING:
        return std::dynamic_pointer_cast<StringExpr>(node)->value;

    case ASTNodeType::IDENT:
    {
        auto expr = std::dynamic_pointer_cast<IdentExpr>(node);
        if (variables.find(expr->name) == variables.end())
            throw std::runtime_error("Undefined variable: " + expr->name);
        return variables[expr->name];
    }

    case ASTNodeType::CALL_EXPR:
    {
        auto call = std::dynamic_pointer_cast<CallExpr>(node);
        // std::cout << "⚡ calling function: " << call->callee << std::endl;

        if (functions.find(call->callee) == functions.end())
            throw std::runtime_error("Undefined function: " + call->callee);

        auto fn = functions[call->callee];
        if (call->args.size() != fn->params.size())
            throw std::runtime_error("Argument count mismatch in call to " + call->callee);

        Interpreter local;
        local.functions = this->functions;
        local.classes   = this->classes;

        for (size_t i = 0; i < fn->params.size(); i++)
            local.variables[fn->params[i]] = evalExpression(call->args[i]);

        try {
            local.execute(fn->body);
        } catch (Interpreter::Value &retVal) {
            return retVal; // handle return
        }
        return 0.0; // default return
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
            if (expr->op == ">")  return (l > r);
            if (expr->op == "<")  return (l < r);
            if (expr->op == "==") return (l == r);
            if (expr->op == "!=") return (l != r);
            if (expr->op == ">=") return (l >= r);
            if (expr->op == "<=") return (l <= r);
            if (expr->op == "+")  return l + r;
            if (expr->op == "-")  return l - r;
            if (expr->op == "*")  return l * r;
            if (expr->op == "/")  return l / r;
            if (expr->op == "%")  return std::fmod(l, r);

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
            if (std::holds_alternative<bool>(val)) return !std::get<bool>(val);
            if (std::holds_alternative<double>(val)) return std::get<double>(val) == 0.0;
            throw std::runtime_error("Invalid operand type for '!'");
        }

        if (expr->op == "-")
        {
            if (std::holds_alternative<double>(val)) return -std::get<double>(val);
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
            throw std::runtime_error("Index must be a number");

        int i = static_cast<int>(std::get<double>(indexVal));

        if (std::holds_alternative<std::string>(targetVal))
        {
            const std::string &s = std::get<std::string>(targetVal);
            if (i < 0) i = static_cast<int>(s.size()) + i;
            if (i < 0 || i >= (int)s.size())
                throw std::runtime_error("String index out of range");
            return std::string(1, s[i]);
        }
        else if (std::holds_alternative<std::vector<Value>>(targetVal))
        {
            const auto &arr = std::get<std::vector<Value>>(targetVal);
            if (i < 0) i = static_cast<int>(arr.size()) + i;
            if (i < 0 || i >= (int)arr.size())
                throw std::runtime_error("Array index out of range");
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
