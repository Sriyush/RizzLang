#pragma once
#include <string>

enum class TokenType {
    PRINT, IF, ELSE, FUNC, LOOP, TRUE, FALSE,
    IDENT, NUMBER, STRING,
    ASSIGN, GT, COLON, NEWLINE,CONTINUE,BREAK,
    ENDOFFILE
};

struct Token {
    TokenType type;
    std::string value;
};
