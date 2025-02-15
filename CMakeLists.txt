cmake_minimum_required(VERSION 3.21)
project(ccalc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(ccalc)

target_sources(ccalc PRIVATE
    "src/main.cpp"
    "src/ui/ui.cpp"
    "src/ast/ast.cpp"
    "src/ast/node.cpp"
    "src/parser/parser.cpp"
    "src/file/file.cpp"
)

#Includes the header files from src
target_include_directories(ccalc PRIVATE 
    "src"
    "src/ui"
    "src/ast"
    "src/error"
    "src/parser"
    "src/file"
    "src/types"
)

target_compile_options(ccalc PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g -O2 -Wall -Wextra -Wpedantic -Wshadow -Werror > # Apply -O2 if the compiler is not MSVC
    $<$<CXX_COMPILER_ID:MSVC>:/O2>        # Apply /O2 if the compiler is MSVC
)
#Use this command to build: cmake -B build && make -C build
