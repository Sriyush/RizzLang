#pragma once
#include <string>
//defining all kind of identifiers which will be used in the language
enum class TokenType {
    PRINT,INPUT, COND, FUNC, LOOP, TRUE, FALSE,CLASS,CLASSEND,COMMA,FUNCEND,CONDEND,COMMENT,
    IDENT, INT, FLOAT, STRING,RETURN,OBJECT,DOT,
    ASSIGN, GT,LT,GE, LE, EQ, NEQ, REV,
    COLON, NEWLINE,CONTINUE,BREAK,
    PLUS,MINUS,MUL,DIV,MOD,
    LPAREN,RPAREN,LBRACKET, RBRACKET,SEMI,
    ENDOFFILE
};

enum class CondType{ IF , ELSEIF , ELSE};

struct Token {
    TokenType type;
    std::string value;
    CondType condType;
};
