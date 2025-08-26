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

    struct Value : std::variant<
        double,
        std::string,
        bool,
        Array
    > {
        using variant::variant; // inherit constructors
    };

    void execute(const std::vector<std::shared_ptr<ASTNode>>& statements);

private:
    std::unordered_map<std::string, Value> variables;

    void executeNode(const std::shared_ptr<ASTNode>& node);
    Value evalExpression(const std::shared_ptr<ASTNode>& node);
};
