#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <memory>

class Interpreter {
public:
    struct Value;

    using Array = std::vector<Value>;

    struct Instance {
        std::unordered_map<std::string, Value> fields;
        std::unordered_map<std::string, std::shared_ptr<FuncDef>> methods;
    };

    struct Value : std::variant<
        double,
        std::string,
        bool,
        Array,
        std::shared_ptr<Instance>,
        std::shared_ptr<FuncDef> 
    > {
        using variant::variant;
    };

    void execute(const std::vector<std::shared_ptr<ASTNode>>& statements);

private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::shared_ptr<FuncDef>> functions;
    std::unordered_map<std::string, std::shared_ptr<ClassDef>> classes;

    void executeNode(const std::shared_ptr<ASTNode>& node);
    void executeInput(const std::shared_ptr<InputStmt>& stmt);
    Value evalExpression(const std::shared_ptr<ASTNode>& node);

    Value callFunction(const std::shared_ptr<FuncDef>& fn,
                       const std::vector<Value>& args,
                       const std::shared_ptr<Instance>& self);
};
