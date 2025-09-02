#include "interpreter.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>

// Helper: call a function (global or method). self == nullptr for globals.
Interpreter::Value Interpreter::callFunction(
    const std::shared_ptr<FuncDef>& fn,
    const std::vector<Value>& args,
    const std::shared_ptr<Instance>& self)
{
    if (!fn) throw std::runtime_error("callFunction: null function");
    if (args.size() != fn->params.size())
        throw std::runtime_error("Argument count mismatch in call to " + fn->name);

    // Save current variables and swap in a fresh local env
    auto oldVars = variables;
    // Bind parameters
    for (size_t i = 0; i < fn->params.size(); ++i) {
        variables[fn->params[i]] = args[i];
    }
    // Bind `self` for methods as variable "self"
    if (self) variables["self"] = self;

    // Execute function body and catch return by throwing Value (if you want that pattern).
    try {
        for (auto &stmt : fn->body) {
            executeNode(stmt);
        }
    } catch (Interpreter::Value &retVal) {
        // restore environment and propagate return value
        variables = oldVars;
        return retVal;
    }

    // restore environment and return default (0.0)
    variables = oldVars;
    return 0.0;
}

// Entry point
void Interpreter::execute(const std::vector<std::shared_ptr<ASTNode>> &statements) {
    for (auto &stmt : statements) executeNode(stmt);
}

void Interpreter::executeNode(const std::shared_ptr<ASTNode> &node) {
    switch (node->type) {
    case ASTNodeType::PRINT_STMT: {
        auto stmt = std::dynamic_pointer_cast<PrintStmt>(node);
        auto val = evalExpression(stmt->value);
        if (std::holds_alternative<double>(val)) std::cout << std::get<double>(val) << std::endl;
        else if (std::holds_alternative<std::string>(val)) std::cout << std::get<std::string>(val) << std::endl;
        else if (std::holds_alternative<bool>(val)) std::cout << (std::get<bool>(val) ? "no_cap" : "cap") << std::endl;
        else if (std::holds_alternative<std::shared_ptr<Instance>>(val)) std::cout << "<object>" << std::endl;
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
        auto current = stmt;
        while (current) {
            bool condResult = false;
            if (current->condition) {
                auto condVal = evalExpression(current->condition);
                if (std::holds_alternative<bool>(condVal)) condResult = std::get<bool>(condVal);
                else if (std::holds_alternative<double>(condVal)) condResult = (std::get<double>(condVal) != 0.0);
                else if (std::holds_alternative<std::string>(condVal)) condResult = !std::get<std::string>(condVal).empty();
                else condResult = true; // treat objects/arrays truthy if needed
            } else {
                condResult = true;
            }
            if (condResult) {
                for (auto &c : current->thenBranch) executeNode(c);
                return;
            }
            current = current->next;
        }
        break;
    }

    case ASTNodeType::FUNC_DEF: {
        auto fn = std::dynamic_pointer_cast<FuncDef>(node);
        functions[fn->name] = fn;
        break;
    }

    case ASTNodeType::CLASS_DEF: {
        auto cl = std::dynamic_pointer_cast<ClassDef>(node);
        classes[cl->name] = cl;
            // std::cerr << "[DEBUG] Registered class: " << cl->name << " with " << cl->methods.size() << " methods\n";
            break;
    }

    case ASTNodeType::RETURN_STMT: {
        auto stmt = std::dynamic_pointer_cast<ReturnStmt>(node);
        // Evaluate return expression and throw it to unwind (existing pattern)
        throw evalExpression(stmt->value);
    }

    case ASTNodeType::EXPR_STMT: {
        auto stmt = std::dynamic_pointer_cast<ExprStmt>(node);
        evalExpression(stmt->expr);
        break;
    }

    case ASTNodeType::INPUT_STMT:
        executeInput(std::dynamic_pointer_cast<InputStmt>(node));
        break;

    default:
        throw std::runtime_error("Unknown AST node type in executeNode");
    }
}

void Interpreter::executeInput(const std::shared_ptr<InputStmt> &stmt) {
    std::cout << "📝 spill " << stmt->varName << ": ";
    std::string s;
    std::getline(std::cin, s);
    try {
        variables[stmt->varName] = std::stod(s);
    } catch (...) {
        variables[stmt->varName] = s;
    }
}

// Evaluate expressions (numbers, strings, id, calls, new, method call, binary, unary, index)
Interpreter::Value Interpreter::evalExpression(const std::shared_ptr<ASTNode> &node) {
    switch (node->type) {
    case ASTNodeType::NUMBER:
        return std::dynamic_pointer_cast<NumberExpr>(node)->value;

    case ASTNodeType::STRING:
        return std::dynamic_pointer_cast<StringExpr>(node)->value;

    case ASTNodeType::IDENT: {
        auto e = std::dynamic_pointer_cast<IdentExpr>(node);
        if (variables.find(e->name) == variables.end())
            throw std::runtime_error("Undefined variable: " + e->name);
        return variables[e->name];
    }

    case ASTNodeType::CALL_EXPR: {
        auto call = std::dynamic_pointer_cast<CallExpr>(node);

        // 1. Look in global functions
        auto it = functions.find(call->callee);
        if (it != functions.end()) {
            std::vector<Value> argVals;
            for (auto &a : call->args) argVals.push_back(evalExpression(a));
            return callFunction(it->second, argVals, nullptr);
        }

        // 2. If not found, maybe it's a variable holding an instance
        auto varIt = variables.find(call->callee);
        if (varIt != variables.end() && 
            std::holds_alternative<std::shared_ptr<Instance>>(varIt->second)) {
            
            auto instance = std::get<std::shared_ptr<Instance>>(varIt->second);
            // look for a constructor-style method "init"
            auto methodIt = instance->methods.find("init");
            if (methodIt != instance->methods.end()) {
                std::vector<Value> argVals;
                for (auto &a : call->args) argVals.push_back(evalExpression(a));
                return callFunction(methodIt->second, argVals, instance);
            }
        }

        throw std::runtime_error("Undefined lol: " + call->callee);
    }

    case ASTNodeType::NEW_OBJECT_EXPR: {
        auto no = std::dynamic_pointer_cast<NewObjectExpr>(node);
        auto cit = classes.find(no->className);
        if (cit == classes.end()) throw std::runtime_error("Class not found: " + no->className);
        auto cl = cit->second;
        auto inst = std::make_shared<Instance>();
        // copy methods into instance
        for (auto &m : cl->methods) {
        inst->methods[m->name] = m;
        // std::cerr << "[DEBUG] Instance of " << cl->name << " stored method: " << m->name << "\n";
    }
        // (optional) you could call initializer here if you had one
        return inst;
    }

    case ASTNodeType::BINARY_EXPR: {
        auto be = std::dynamic_pointer_cast<BinaryExpr>(node);
        auto L = evalExpression(be->left);
        auto R = evalExpression(be->right);

        if (std::holds_alternative<double>(L) && std::holds_alternative<double>(R)) {
            double l = std::get<double>(L);
            double r = std::get<double>(R);
            if (be->op == "+") return l + r;
            if (be->op == "-") return l - r;
            if (be->op == "*") return l * r;
            if (be->op == "/") return l / r;
            if (be->op == "%") return std::fmod(l, r);
            if (be->op == ">") return l > r;
            if (be->op == "<") return l < r;
            if (be->op == ">=") return l >= r;
            if (be->op == "<=") return l <= r;
            if (be->op == "==") return l == r;
            if (be->op == "!=") return l != r;
        }
            if (be->op == "+" && std::holds_alternative<std::string>(L) && std::holds_alternative<std::string>(R)) {
                return std::get<std::string>(L) + std::get<std::string>(R);
            }

            // string + number
            if (be->op == "+" && std::holds_alternative<std::string>(L) && std::holds_alternative<double>(R)) {
                return std::get<std::string>(L) + std::to_string(std::get<double>(R));
            }
            if (be->op == "+" && std::holds_alternative<double>(L) && std::holds_alternative<std::string>(R)) {
                return std::to_string(std::get<double>(L)) + std::get<std::string>(R);
            }
        throw std::runtime_error("Invalid operands for binary operator: " + be->op);
    }

    case ASTNodeType::UNARY_EXPR: {
        auto ue = std::dynamic_pointer_cast<UnaryExpr>(node);
        auto v = evalExpression(ue->operand);
        if (ue->op == "!") {
            if (std::holds_alternative<bool>(v)) return !std::get<bool>(v);
            if (std::holds_alternative<double>(v)) return std::get<double>(v) == 0.0;
            throw std::runtime_error("Invalid operand type for '!'");
        } else if (ue->op == "-") {
            if (std::holds_alternative<double>(v)) return -std::get<double>(v);
            throw std::runtime_error("Invalid operand type for unary '-'");
        }
        throw std::runtime_error("Unknown unary operator: " + ue->op);
    }
    case ASTNodeType::MEMBER_ACCESS_EXPR: {
    auto ma = std::dynamic_pointer_cast<MemberAccessExpr>(node);
    auto objVal = evalExpression(ma->object);

    if (!std::holds_alternative<std::shared_ptr<Instance>>(objVal)) {
        throw std::runtime_error("Tried to access member on non-object");
    }
    auto instance = std::get<std::shared_ptr<Instance>>(objVal);

    // Look for a field first
    auto it = instance->fields.find(ma->member);
    if (it != instance->fields.end()) {
        return it->second;
    }

    // Or look for a method reference (optional)
    auto mit = instance->methods.find(ma->member);
    if (mit != instance->methods.end()) {
        return mit->second; // if your Value can hold callable functions
    }

    throw std::runtime_error("Unknown member: " + ma->member);
}

case ASTNodeType::METHOD_CALL_EXPR: {
    auto mc = std::dynamic_pointer_cast<MethodCallExpr>(node);
    auto objVal = evalExpression(mc->object);

    if (!std::holds_alternative<std::shared_ptr<Instance>>(objVal)) {
        throw std::runtime_error("Tried to call method on non-object");
    }
    auto instance = std::get<std::shared_ptr<Instance>>(objVal);

    auto mit = instance->methods.find(mc->method);
    if (mit == instance->methods.end()) {
        throw std::runtime_error("Unknown method: " + mc->method);
    }

    std::vector<Value> argVals;
    for (auto &a : mc->arguments) {
        argVals.push_back(evalExpression(a));
    }

    return callFunction(mit->second, argVals, instance);
}

    case ASTNodeType::INDEX_EXPR: {
        auto ie = std::dynamic_pointer_cast<IndexExpr>(node);
        auto target = evalExpression(ie->target);
        auto index = evalExpression(ie->index);
        if (!std::holds_alternative<double>(index)) throw std::runtime_error("Index must be a number");
        int i = static_cast<int>(std::get<double>(index));

        if (std::holds_alternative<std::string>(target)) {
            const auto &s = std::get<std::string>(target);
            if (i < 0) i = (int)s.size() + i;
            if (i < 0 || i >= (int)s.size()) throw std::runtime_error("String index out of range");
            return std::string(1, s[i]);
        } else if (std::holds_alternative<Array>(target)) {
            const auto &arr = std::get<Array>(target);
            if (i < 0) i = (int)arr.size() + i;
            if (i < 0 || i >= (int)arr.size()) throw std::runtime_error("Array index out of range");
            return arr[i];
        }
        throw std::runtime_error("Brotha Check up with the Data type , cause i dont think so it works with this");
    }

    default:
        throw std::runtime_error("Unknown expression node in evalExpression");
    }
}
