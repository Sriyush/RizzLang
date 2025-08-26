#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
int main()
{
    std::string code = R"(bruh "hello world"
x = 10
y= 103.2
bruh x + y * 3
bruh (x + y) % 10
bruh x >= y
bruh x <= y
z = no_cap
bruh !z
bruh !cap
bruh -x
bruh y -(-x)
sus x > 5:
    bruh "big clout"
)";

    Lexer lexer(code);
    // Token tok;
    // while ((tok = lexer.getNextToken()).type != TokenType::ENDOFFILE) {
    //     std::cout << "Token(" << (int)tok.type << ", " << tok.value << ")\n";
    // }
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();
    // std::cout << "Parsed " << ast.size() << " statements!\n";
    Interpreter interpreter;
    interpreter.execute(ast);

    return 0;
}
/*
#include "lexer.hpp"
#include "parser.hpp"
#include <iostream>

int main() {
    std::string source = R"(
        bruh "hello world"
        x = 10
        sus x > 5:
            bruh "big clout"
    )";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "Parsed " << ast.size() << " statements!\n";
}

*/