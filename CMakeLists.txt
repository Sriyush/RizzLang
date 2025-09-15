cmake_minimum_required(VERSION 3.10)
project(RizzLang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files automatically
file(GLOB SOURCES "src/*.cpp")

add_executable(rizz ${SOURCES})

# Add include dir for headers
target_include_directories(rizz PRIVATE src include)

install(TARGETS rizz DESTINATION bin)