// src/parser.cpp
#include "parser.hpp"
#include <stdexcept>
#include <iostream>

std::vector<std::shared_ptr<ASTNode>> Parser::parse()
{
    std::vector<std::shared_ptr<ASTNode>> program;
    while (!isAtEnd())
    {
        if (peek().type == TokenType::NEWLINE) {
            advance(); 
            continue;
        }
        if (peek().type == TokenType::SEMI) {
            advance();
            continue;
        }
        if (peek().type == TokenType::ENDOFFILE) {
            break;
        }

        auto stmt = statement();
        if (stmt) program.push_back(stmt);
    }
    return program;
}

std::shared_ptr<ASTNode> Parser::statement()
{
    Token tok = peek();

    switch (tok.type)
    {
    case TokenType::PRINT:
        advance();
        return std::make_shared<PrintStmt>(expression());

    case TokenType::COND:   // delegate to helper
        return parseIfStatement();
         
    case TokenType::COLON:
        advance();
        return nullptr;

    case TokenType::SEMI:
        advance(); 
        return nullptr;

case TokenType::IDENT:
{
    std::string name = tok.value;
    size_t lookaheadPos = pos + 1;
    while (lookaheadPos < tokens.size() && tokens[lookaheadPos].type == TokenType::NEWLINE)
        lookaheadPos++;

    // Assignment: x = expr
    if (lookaheadPos < tokens.size() && tokens[lookaheadPos].type == TokenType::ASSIGN) {
        advance(); // consume IDENT
        while (peek().type == TokenType::NEWLINE) advance();
        advance(); // consume '='
        auto val = expression();
        if (peek().type == TokenType::SEMI) advance(); // optional ';'
        return std::make_shared<AssignStmt>(name, val);
    }

    // Function call: foo(...)
    if (lookaheadPos < tokens.size() && tokens[lookaheadPos].type == TokenType::LPAREN) {
        advance(); // consume IDENT
        advance(); // consume '('
        std::vector<std::shared_ptr<ASTNode>> args;
        if (peek().type != TokenType::RPAREN) {
            do {
                args.push_back(expression());
                if (peek().type == TokenType::COMMA) {
                    advance(); // consume ','
                } else {
                    break;
                }
            } while (true);
        }
        advance();
        if (peek().type == TokenType::SEMI) advance(); // optional ';'
        return std::make_shared<ExprStmt>(
            std::make_shared<CallExpr>(name, args)
        );
    }

    // Fallback: just identifier reference
    advance();
    if (peek().type == TokenType::SEMI) advance();
    return std::make_shared<ExprStmt>(std::make_shared<IdentExpr>(name));
}

    case TokenType::CONTINUE:
        advance();
        return nullptr;

    case TokenType::BREAK:
        advance();
        return nullptr;
    
    case TokenType::FUNCEND:
        advance();
        return nullptr;

    case TokenType::FUNC:
        return parseFunction();

    case TokenType::CLASS:
        return parseClass();

    case TokenType::INPUT:
        return parseInput();

    default:
        auto expr = expression();
        return std::make_shared<ExprStmt>(expr);
    }
}

// ---------------- expressions ----------------

std::shared_ptr<ASTNode> Parser::primary()
{
    Token tok = advance();

    switch (tok.type)
    {
    case TokenType::INT:
        return std::make_shared<NumberExpr>(std::stoi(tok.value));
    case TokenType::FLOAT:
        return std::make_shared<NumberExpr>(std::stod(tok.value));
    case TokenType::STRING:
        return std::make_shared<StringExpr>(tok.value);

case TokenType::IDENT:
{
    std::string name = tok.value;
    std::shared_ptr<ASTNode> expr;

    // Function call? IDENT '(' ...
    if (peek().type == TokenType::LPAREN) {
        advance(); // consume '('
        std::vector<std::shared_ptr<ASTNode>> args;
        if (peek().type != TokenType::RPAREN) {
            do {
    args.push_back(expression());
    if (peek().type == TokenType::COMMA) {
        advance(); // consume comma
    } else {
        break;
    }
} while (true);

        }
        if (peek().type != TokenType::RPAREN) {
            throw std::runtime_error("Expected ')' after function arguments");
        }
        advance(); // consume ')'
        expr = std::make_shared<CallExpr>(name, args);
    } else {
        expr = std::make_shared<IdentExpr>(name);
    }

    // Handle array indexing after either variable or call
    while (peek().type == TokenType::LBRACKET) {
        advance(); // consume '['
        auto indexExpr = expression();
        if (peek().type != TokenType::RBRACKET)
            throw std::runtime_error("Error!! Mah brotha did ']' went to buy milk?");
        advance(); // consume ']'
        expr = std::make_shared<IndexExpr>(expr, indexExpr);
    }

    return expr;
}

    case TokenType::TRUE:  return std::make_shared<NumberExpr>(1);
    case TokenType::FALSE: return std::make_shared<NumberExpr>(0);

    case TokenType::LPAREN:
    {
        auto expr = expression();
        if (peek().type != TokenType::RPAREN)
            throw std::runtime_error("Expected ')'");
        advance();
        return expr;
    }

    default:
        if (tok.type == TokenType::RPAREN)
            throw std::runtime_error("Unexpected ')'");
        throw std::runtime_error("Unexpected token in expression: " + tok.value);
    }
}

std::shared_ptr<ASTNode> Parser::expression()
{
    auto left = parseTerm();
    while (!isAtEnd() &&
           (peek().type == TokenType::GT ||
            peek().type == TokenType::LT ||
            peek().type == TokenType::EQ ||
            peek().type == TokenType::NEQ ||
            peek().type == TokenType::GE ||
            peek().type == TokenType::LE))
    {
        Token op = advance();
        auto right = parseTerm();
        left = std::make_shared<BinaryExpr>(left, op.value, right);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm()
{
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

std::shared_ptr<ASTNode> Parser::parseFactor()
{
    auto left = parseUnary();
    while (!isAtEnd() &&
           (peek().type == TokenType::MUL ||
            peek().type == TokenType::DIV ||
            peek().type == TokenType::MOD))
    {
        Token op = advance();
        auto right = parseUnary();
        left = std::make_shared<BinaryExpr>(left, op.value, right);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseUnary()
{
    if (!isAtEnd() &&
        (peek().type == TokenType::REV ||  // !
         peek().type == TokenType::MINUS)) // -x
    {
        Token op = advance();
        auto right = parseUnary();
        return std::make_shared<UnaryExpr>(op.value, right);
    }
    return primary();
}

const char* ASTNodeTypeToString(ASTNodeType type) {
    switch(type) {
        case ASTNodeType::PRINT_STMT: return "PRINT_STMT";
        case ASTNodeType::ASSIGN_STMT: return "ASSIGN_STMT";
        case ASTNodeType::IF_STMT: return "IF_STMT";
        case ASTNodeType::FUNC_DEF: return "FUNC_DEF";
        case ASTNodeType::CLASS_DEF: return "CLASS_DEF";
        case ASTNodeType::RETURN_STMT: return "RETURN_STMT";
        case ASTNodeType::EXPR_STMT: return "EXPR_STMT";
        case ASTNodeType::INPUT_STMT: return "INPUT_STMT";
        case ASTNodeType::NUMBER: return "NUMBER";
        case ASTNodeType::STRING: return "STRING";
        case ASTNodeType::IDENT: return "IDENT";
        case ASTNodeType::CALL_EXPR: return "CALL_EXPR";
        case ASTNodeType::BINARY_EXPR: return "BINARY_EXPR";
        case ASTNodeType::UNARY_EXPR: return "UNARY_EXPR";
        case ASTNodeType::INDEX_EXPR: return "INDEX_EXPR";
        // Add all possible enum values for completeness
        default: return "UNKNOWN";
    }
}

std::shared_ptr<ASTNode> Parser::parseInput() {
    advance(); // consume 'spill'
    auto nameTok = peek();
    if (nameTok.type != TokenType::IDENT) {
        throw std::runtime_error("Expected variable name after 'spill'");
    }
    advance();
    return std::make_shared<InputStmt>(nameTok.value);
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    std::shared_ptr<IfStmt> head = nullptr;
    std::shared_ptr<IfStmt> current = nullptr;

    while (!isAtEnd() && peek().type == TokenType::COND) {
        Token token = advance();
        std::shared_ptr<ASTNode> condExpr = nullptr;

        if (token.condType == CondType::IF || token.condType == CondType::ELSEIF) {
            condExpr = expression();
        }

        if (peek().type != TokenType::COLON) {
            throw std::runtime_error("Expected ':' after if/elseif/else condition.");
        }
        advance(); // consume ':'

        std::vector<std::shared_ptr<ASTNode>> body;
        while (!isAtEnd() &&
               !(peek().type == TokenType::COND &&
                 (peek().condType == CondType::ELSEIF || peek().condType == CondType::ELSE)) &&
               peek().type != TokenType::SEMI &&
               peek().type != TokenType::ENDOFFILE) 
        {
            if (peek().type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            auto stmt = statement();
            if (stmt) body.push_back(stmt);
        }

        auto newNode = std::make_shared<IfStmt>(condExpr, body);

        if (!head) {
            head = newNode;
            current = newNode;
        } else {
            current->next = newNode;
            current = newNode;
        }

        if (token.condType == CondType::ELSE) break;
    }

    return head;
}

std::shared_ptr<ASTNode> Parser::parseFunction() {
    advance(); // consume 'drip'

    Token nameTok = peek();
    if (nameTok.type != TokenType::IDENT)
        throw std::runtime_error("Expected function name after 'drip'");
    advance();

    if (peek().type != TokenType::LPAREN)
        throw std::runtime_error("Expected '(' after function name");
    advance(); // consume '('

    std::vector<std::string> params;
    while (peek().type != TokenType::RPAREN) {
        if (peek().type != TokenType::IDENT)
            throw std::runtime_error("Expected parameter name");
        params.push_back(peek().value);
        advance();
        if (peek().type == TokenType::COMMA) advance();
    }
    advance(); // consume ')'

    if (peek().type != TokenType::COLON)
        throw std::runtime_error("Expected ':' after function header");
    // advance();

    std::vector<std::shared_ptr<ASTNode>> body;

    while (!isAtEnd() &&
           peek().type != TokenType::ENDOFFILE &&
           peek().type != TokenType::FUNC &&
           peek().type != TokenType::CLASS &&
           !(peek().type == TokenType::COND && peek().condType == CondType::ELSE) && peek().type != TokenType::FUNCEND) 
    {
        if (peek().type == TokenType::NEWLINE) { advance(); continue; }
        if (peek().type == TokenType::SEMI) { advance(); continue; }
        auto stmt = statement();
        if (stmt)   {
            // std::cout << "parsed stmt of type: " << ASTNodeTypeToString(stmt->type) << std::endl;
            body.push_back(stmt);
        }
    }

    return std::make_shared<FuncDef>(nameTok.value, params, body);
}

std::shared_ptr<ASTNode> Parser::parseClass() {
    advance(); // consume 'rizz'

    Token nameTok = peek();
    if (nameTok.type != TokenType::IDENT)
        throw std::runtime_error("Expected class name after 'rizz'");
    advance();

    if (peek().type != TokenType::COLON)
        throw std::runtime_error("Expected ':' after class name");
    advance();

    std::vector<std::shared_ptr<FuncDef>> methods;
    while (!isAtEnd() &&
           peek().type != TokenType::ENDOFFILE &&
           peek().type != TokenType::FUNC) {
        if (peek().type == TokenType::NEWLINE) { advance(); continue; }
        if (peek().type == TokenType::CLASS) {
            auto fn = std::dynamic_pointer_cast<FuncDef>(parseFunction());
            methods.push_back(fn);
        } else {
            statement();
        }
    }

    return std::make_shared<ClassDef>(nameTok.value, methods);
}
