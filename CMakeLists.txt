cmake_minimum_required(VERSION 3.21)
project(ccalc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(GMP_LIBRARIES NAMES gmp libgmp)
find_library(MPFR_LIBRARIES NAMES mpfr libmpfr)

if(NOT GMP_LIBRARIES)
    message(FATAL_ERROR "GMP library not found. Please install GMP.")
endif()
if(NOT MPFR_LIBRARIES)
    message(FATAL_ERROR "MPFR library not found. Please install MPFR. GMP is also required for MPFR.")
endif()

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
    "src/parser"
    "src/file"
)

target_link_libraries(ccalc PRIVATE
    ${GMP_LIBRARIES}
    ${MPFR_LIBRARIES}
)

if(MSVC)
    target_compile_options(ccalc PRIVATE 
        /O2 /W4
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(ccalc PRIVATE 
        -g 
        -O2 
        -Wall 
        -Wextra 
        -Wshadow 
        -Wconversion 
        -Wpedantic
        -fno-fast-math
        -fcomplex-arithmetic=full
    )
else()
    target_compile_options(ccalc PRIVATE 
        -g 
        -O2 
        -Wall 
        -Wextra 
        -Wshadow 
        -Wconversion 
        -Wpedantic
        -fno-fast-math
        -fexcess-precision=standard
        -frounding-math
        -fsignaling-nans
    )
endif()
#Use this command to build: cmake -B build && make -C build
