// src/parser.cpp
#include "parser.hpp"
#include <stdexcept>
#include <iostream>

std::vector<std::shared_ptr<ASTNode>> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> program;
    while (!isAtEnd()) {
        if (peek().type == TokenType::NEWLINE) {
            advance(); // skip blank lines
            continue;
        }
        if (peek().type == TokenType::ENDOFFILE) {
            break; // end of file reached
        }

        auto stmt = statement();
        if (stmt) program.push_back(stmt);
    }
    return program;
}

std::shared_ptr<ASTNode> Parser::statement() {
    Token tok = peek();

    switch (tok.type) {
        case TokenType::PRINT:
            advance();
            return std::make_shared<PrintStmt>(expression());

        case TokenType::IF: {
            advance();
            auto cond = expression();
            auto ifNode = std::make_shared<IfStmt>(cond);

            if (peek().type == TokenType::COLON) advance();
            while (!isAtEnd() && peek().type != TokenType::ENDOFFILE) {
                if (peek().type == TokenType::NEWLINE) { advance(); continue; }
                if (peek().type == TokenType::ELSE) break; // stop at else
                ifNode->body.push_back(statement());
            }
            return ifNode;
        }

        case TokenType::IDENT: {
            std::string name = tok.value;
            advance();
            if (peek().type == TokenType::ASSIGN) {
                advance();
                auto val = expression();
                return std::make_shared<AssignStmt>(name, val);
            }
            throw std::runtime_error("Unexpected identifier without assignment: " + name);
        }

        case TokenType::CONTINUE:
            advance();
            // could add AST node for continue
            return nullptr;

        case TokenType::BREAK:
            advance();
            // could add AST node for break
            return nullptr;

        default:
            throw std::runtime_error("Unknown statement: " + tok.value);
    }
}

std::shared_ptr<ASTNode> Parser::primary() {
    Token tok = advance();

    switch (tok.type) {
        case TokenType::INT:
            return std::make_shared<NumberExpr>(std::stoi(tok.value));

        case TokenType::FLOAT:
            return std::make_shared<NumberExpr>(std::stod(tok.value));

        case TokenType::STRING:
            return std::make_shared<StringExpr>(tok.value);
        case TokenType::IDENT:
            return std::make_shared<IdentExpr>(tok.value);
        case TokenType::TRUE:
            return std::make_shared<NumberExpr>(1);
        case TokenType::FALSE:
            return std::make_shared<NumberExpr>(0);
                    case TokenType::LPAREN: {
            auto expr = expression();
            if (peek().type != TokenType::RPAREN)
                throw std::runtime_error("Expected ')'");
            advance(); // consume ')'
            return expr;
        }
        default:
            throw std::runtime_error("Unexpected token in expression: " + tok.value);
    }
}

// std::shared_ptr<ASTNode> Parser::expression() {
//     auto left = primary();

//     // very basic binary parsing (e.g., x > 5)
//     if (!isAtEnd()) {
//         if (peek().type == TokenType::GT) {
//             advance();
//             auto right = primary();
//             return std::make_shared<BinaryExpr>(left, ">", right);
//         }
//     }
//     return left;
// }

std::shared_ptr<ASTNode> Parser::expression() {
    auto left = parseTerm();

    // comparisons: < > == != <= >=
    while (!isAtEnd() &&
          (peek().type == TokenType::GT ||
           peek().type == TokenType::LT ||
           peek().type == TokenType::EQ ||
           peek().type == TokenType::NEQ ||
           peek().type == TokenType::GE ||
           peek().type == TokenType::LE
        )) 
    {
        Token op = advance();
        auto right = parseTerm();
        left = std::make_shared<BinaryExpr>(left, op.value, right);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (!isAtEnd() &&
          (peek().type == TokenType::PLUS ||
           peek().type == TokenType::MINUS)) 
    {
        Token op = advance();
        auto right = parseFactor();
        left = std::make_shared<BinaryExpr>(left, op.value, right);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    auto left = primary();

    while (!isAtEnd() &&
          (peek().type == TokenType::MUL ||
           peek().type == TokenType::DIV ||
           peek().type == TokenType::MOD)) 
    {
        Token op = advance();
        auto right = primary();
        left = std::make_shared<BinaryExpr>(left, op.value, right);
    }
    return left;
}

