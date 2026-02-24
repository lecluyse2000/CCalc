cmake_minimum_required(VERSION 3.21)
project(ccalc)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_INSTALL_PREFIX "/usr/local")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_PREFIX "/usr")
else()
    set(CMAKE_INSTALL_PREFIX "C:\\Program Files\\ccalc")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(GMP_LIBRARIES NAMES gmp libgmp gmpxx)
find_library(MPFR_LIBRARIES NAMES mpfr libmpfr)
if(NOT APPLE)
    find_library(READLINE_LIBRARIES NAMES readline libreadline)
endif()
find_path(GMP_INCLUDE_DIR NAMES gmpxx.h gmp.h)
find_path(MPFR_INCLUDE_DIR NAMES mpfr.h)


if(NOT GMP_INCLUDE_DIR)
    message(FATAL_ERROR "GMP headers not found. Please install GMP correctly.")
elseif(NOT MPFR_INCLUDE_DIR)
    message(FATAL_ERROR "MPFR headers not found. Please install MPFR correctly.")
elseif(NOT GMP_LIBRARIES)
    message(FATAL_ERROR "GMP library not found. Please install GMP.")
elseif(NOT MPFR_LIBRARIES)
    message(FATAL_ERROR "MPFR library not found. Please install MPFR. GMP is also required for MPFR.")
elseif(NOT APPLE AND NOT READLINE_LIBRARIES)
    message(FATAL_ERROR "Readline library not found.")
endif()

# We have to manually find the files on mac since mac uses libedit by default

if(APPLE)
    execute_process(
        COMMAND brew --prefix readline
        OUTPUT_VARIABLE READLINE_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE BREW_READLINE_RES
    )
    if(NOT BREW_READLINE_RES EQUAL 0)
        message(FATAL_ERROR "Homebrew readline not found. Run: brew install readline")
    endif()

    find_library(READLINE_LIBRARIES NAMES readline libreadline 
        PATHS "${READLINE_PREFIX}/lib"
        NO_DEFAULT_PATH
    )
    find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h
        PATHS "${READLINE_PREFIX}/include"
        NO_DEFAULT_PATH
    )

    if(NOT READLINE_LIBRARIES OR NOT READLINE_INCLUDE_DIR)
        message(FATAL_ERROR "Failed to find Homebrew readline in ${READLINE_PREFIX}")
    endif()
endif()

add_executable(ccalc)

target_sources(ccalc PRIVATE
    "src/main.cpp"
    "src/engine/engine.cpp"
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
    ${GMP_INCLUDE_DIR}
    ${MPFR_INCLUDE_DIR}
    ${READLINE_INCLUDE_DIR}
)

target_link_libraries(ccalc PRIVATE
    ${GMP_LIBRARIES}
    ${MPFR_LIBRARIES}
    ${READLINE_LIBRARIES}
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
    DESTINATION bin 
)
