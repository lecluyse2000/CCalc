cmake_minimum_required(VERSION 3.21)
project(ccalc)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_INSTALL_PREFIX "/usr/local/bin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_PREFIX "/usr/bin")
else()
    set(CMAKE_INSTALL_PREFIX "C:\\Program Files\\ccalc\\bin")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(GMP_LIBRARIES NAMES gmp libgmp gmpxx)
find_library(MPFR_LIBRARIES NAMES mpfr libmpfr)
find_path(GMP_INCLUDE_DIR NAMES gmpxx.h gmp.h)
find_path(MPFR_INCLUDE_DIR NAMES mpfr.h)

if(NOT GMP_INCLUDE_DIR)
    message(FATAL_ERROR "GMP headers not found. Please install GMP correctly.")
endif()
if(NOT MPFR_INCLUDE_DIR)
    message(FATAL_ERROR "MPFR headers not found. Please install MPFR correctly.")
endif()
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
    "src/ast/bnode.cpp"
    "src/ast/mnode.cpp"
    "src/parser/parser.cpp"
    "src/parser/boolparse.cpp"
    "src/parser/mathparse.cpp"
    "src/file/file.cpp"
    "src/startup/startup.cpp"
)

#Includes the header files from src
target_include_directories(ccalc PRIVATE 
    "src"
    "src/ui"
    "src/ast"
    "src/parser"
    "src/file"
    "src/startup"
    "src/include"
    ${GMP_INCLUDE_DIR}
    ${MPFR_INCLUDE_DIR}
)

target_link_libraries(ccalc PRIVATE
    ${GMP_LIBRARIES}
    ${MPFR_LIBRARIES}
)

if(MSVC)
    target_compile_options(ccalc PRIVATE 
        /O2
        /fp:strict
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

install(TARGETS ccalc
    DESTINATION . 
)
