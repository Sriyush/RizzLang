#include "lexer.hpp"
#include <cctype>
#include <iostream>
#include <vector>
Lexer::Lexer(std::string input) : text(std::move(input)), pos(0) {
    currentChar = text.empty() ? '\0' : text[0];
}

void Lexer::advance() {
    pos++;
    currentChar = (pos < text.size()) ? text[pos] : '\0';
}

void Lexer::skipWhitespace() {
    while (std::isspace(currentChar) && currentChar != '\0' && currentChar != '\n')
        advance();
}

Token Lexer::number() {
    std::string result;
    while (std::isdigit(currentChar)) {
        result += currentChar;
        advance();
    }
    return {TokenType::NUMBER, result};
}

Token Lexer::identifier() {
    std::string result;
    while (std::isalnum(currentChar)) {
        result += currentChar;
        advance();
    }

    if (result == "bruh") return {TokenType::PRINT, result};
    if (result == "sus") return {TokenType::IF, result};
    if (result == "bet") return {TokenType::ELSE, result};
    if (result == "drip") return {TokenType::FUNC, result};
    if (result == "loop") return {TokenType::LOOP, result};
    if (result == "no_cap") return {TokenType::TRUE, result};
    if (result == "cap") return {TokenType::FALSE, result};
    if (result == "move_it") return {TokenType::CONTINUE, result};
    if (result == "calm_down") return {TokenType::BREAK, result};

    return {TokenType::IDENT, result};
}

Token Lexer::stringLiteral() {
    advance(); // skip opening "
    std::string result;
    while (currentChar != '"' && currentChar != '\0') {
        result += currentChar;
        advance();
    }
    advance(); // skip closing "
    return {TokenType::STRING, result};
}

Token Lexer::getNextToken() {
    while (currentChar != '\0') {
        if (std::isspace(currentChar)) {
            if (currentChar == '\n') {
                advance();
                return {TokenType::NEWLINE, "\\n"};
            }
            skipWhitespace();
            continue;
        }
        if (std::isdigit(currentChar)) return number();
        if (std::isalpha(currentChar)) return identifier();
        if (currentChar == '"') return stringLiteral();
        if (currentChar == '=') { advance(); return {TokenType::ASSIGN, "="}; }
        if (currentChar == '>') { advance(); return {TokenType::GT, ">"}; }
        if (currentChar == ':') { advance(); return {TokenType::COLON, ":"}; }

        std::cerr << "Unexpected char: " << currentChar << "\n";
        advance();
    }
    return {TokenType::ENDOFFILE, ""};
}


std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = getNextToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::ENDOFFILE) break;
    }
    return tokens;
}