#pragma once
#include "token.hpp"
#include <string>
#include <vector>
class Lexer {
    std::string text;
    size_t pos;
    char currentChar;

public:
    Lexer(std::string input);
    Token getNextToken();
    std::vector<Token> tokenize();

private:
    void advance();
    void skipWhitespace();
    Token number();
    Token identifier();
    Token stringLiteral();
};
