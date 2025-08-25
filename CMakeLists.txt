cmake_minimum_required(VERSION 3.10)
project(RizzLang)

set(CMAKE_CXX_STANDARD 17)

add_executable(rizz
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)
