// src/parser.hpp
#pragma once
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
    std::vector<Token> tokens;
    int pos = 0;

public:
    Parser(const std::vector<Token>& t) : tokens(t) {}

    bool isAtEnd() { return pos >= tokens.size(); }
    Token peek() { return tokens[pos]; }
    Token advance() { return tokens[pos++]; }

    std::vector<std::shared_ptr<ASTNode>> parse();
    std::shared_ptr<ASTNode> statement();
    std::shared_ptr<ASTNode> expression();
    std::shared_ptr<ASTNode> primary();
    // std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();

};
