#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

std::string runCode(const std::string &code) {
    try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        Interpreter interpreter;

        // Capture output printed by interpreter
        std::ostringstream outputCapture;
        std::streambuf* oldCout = std::cout.rdbuf(outputCapture.rdbuf());

        interpreter.execute(ast);

        std::cout.rdbuf(oldCout); // restore cout
        return outputCapture.str();
    } catch (const std::exception &e) {
        return std::string("Error: ") + e.what();
    }
}

// ✅ Expose to JavaScript
extern "C" {
    const char* runCodeC(const char* code) {
        static std::string result;
        result = runCode(std::string(code));
        return result.c_str();
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "--version") {
        std::cout << "RizzLang v1.0.12" << std::endl;
        return 0;
    }
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << runCode(code); // reuse runCode
    return 0;
}
