#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <memory>

class Interpreter {
public:
    struct Value; // forward declaration of recursive type

    using Array = std::vector<Value>;

    struct Instance {
        std::unordered_map<std::string, Value> fields; // instance vars
        std::unordered_map<std::string, std::shared_ptr<FuncDef>> methods;
    };

    // Value variant: number, string, bool, array, instance pointer
    struct Value : std::variant<
        double,
        std::string,
        bool,
        Array,
        std::shared_ptr<Instance>,
        std::shared_ptr<FuncDef> 
    > {
        using variant::variant; // inherit constructors
    };

    void execute(const std::vector<std::shared_ptr<ASTNode>>& statements);

private:
    // environment
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::shared_ptr<FuncDef>> functions;
    std::unordered_map<std::string, std::shared_ptr<ClassDef>> classes;

    // core interpreter routines
    void executeNode(const std::shared_ptr<ASTNode>& node);
    void executeInput(const std::shared_ptr<InputStmt>& stmt);
    Value evalExpression(const std::shared_ptr<ASTNode>& node);

    // helper to call functions/methods
    Value callFunction(const std::shared_ptr<FuncDef>& fn,
                       const std::vector<Value>& args,
                       const std::shared_ptr<Instance>& self); // self == nullptr for globals
};
