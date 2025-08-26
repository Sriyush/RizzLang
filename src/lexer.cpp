//this is my lexer here my program will go through its first steps where the program will get tokenized for further steps
#include "lexer.hpp"
#include <cctype>
#include <iostream>
#include <vector>


//Now this is the contructor this will take src code as string checks the current char at current pos if empty then marks as \0
Lexer::Lexer(std::string input) : text(std::move(input)), pos(0) {
    currentChar = text.empty() ? '\0' : text[0];
}

//this function moves the lexer forward by 1 char
void Lexer::advance() {
    pos++;
    currentChar = (pos < text.size()) ? text[pos] : '\0';
}

//this is to skip whitespaces so that these spaces dont produce tokens , but keeps newLine in the game
void Lexer::skipWhitespace() {
    while (std::isspace(currentChar) && currentChar != '\0' && currentChar != '\n')
        advance();
}

//this reads number from the i/p and collects integer from it , if it sees '.' in it then switches to float and collects the decimal digits based on it.
Token Lexer::number() {
    std::string result;
    //taking it as integer in start
    bool isFloat = false;

    // integer part
    while (std::isdigit(currentChar)) {
        result += currentChar;
        advance();
    }

    // check for decimal point
    if (currentChar == '.') {
        isFloat = true;
        result += currentChar;
        advance();

        while (std::isdigit(currentChar)) {
            result += currentChar;
            advance();
        }
    }

    if (isFloat) {
        return {TokenType::FLOAT, result};
    } else {
        return {TokenType::INT, result};
    }
}

//this reads variables and functions then links them to their resp keywords "bruh" -> print and more like this , if no keyword matches then its generic IDENT
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

//handles string literals , it skips the opening an closing qoutes and taes up the string 
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

//this is the main part in the lexer, it detects space and newlines and skips them also this detects number alpha and symbols
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
        if (currentChar == '=') {
            advance();
            if (currentChar == '=') { advance(); return {TokenType::EQ, "=="}; }
            return {TokenType::ASSIGN, "="};
        }
        if (currentChar == '>') {
            advance();
            if (currentChar == '=') { advance(); return {TokenType::GE, ">="}; }
            return {TokenType::GT, ">"};
        }
        if (currentChar == '<') {
            advance();
            if (currentChar == '=') { advance(); return {TokenType::LE, "<="}; }
            return {TokenType::LT, "<"};
        }
        if (currentChar == '!') {
            advance();
            if (currentChar == '=') { advance(); return {TokenType::NEQ, "!="}; }
            throw std::runtime_error("This shi is Unexpected my brotha'!'");
        }
        if (currentChar == ':') { advance(); return {TokenType::COLON, ":"}; }
        if (currentChar == '+') { advance(); return {TokenType::PLUS, "+"}; }
        if (currentChar == '-') { advance(); return {TokenType::MINUS, "-"}; }
        if (currentChar == '*') { advance(); return {TokenType::MUL, "*"}; }
        if (currentChar == '/') { advance(); return {TokenType::DIV, "/"}; }
        if (currentChar == '%') { advance(); return {TokenType::MOD, "%"}; }
        if (currentChar == '(') { advance(); return {TokenType::LPAREN, "("}; }
        if (currentChar == ')') { advance(); return {TokenType::RPAREN, ")"}; }
        // if (currentChar == '==') { ad}
        std::cerr << "Unexpected char: " << currentChar << "\n";
        advance();
    }
    return {TokenType::ENDOFFILE, ""};

}
//this is called every time until the EOF , then stores all the tokens in vector and later return them
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = getNextToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::ENDOFFILE) break;
    }
    return tokens;
}