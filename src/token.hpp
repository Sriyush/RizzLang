#pragma once
#include <string>
//defining all kind of identifiers which will be used in the language
enum class TokenType {
    PRINT, IF, ELSE, FUNC, LOOP, TRUE, FALSE,
    IDENT, INT, FLOAT, STRING,
    ASSIGN, GT,LT,GE, LE, EQ, NEQ, 
    COLON, NEWLINE,CONTINUE,BREAK,
    PLUS,MINUS,MUL,DIV,MOD,LPAREN,RPAREN,
    ENDOFFILE
};

struct Token {
    TokenType type;
    std::string value;
};
