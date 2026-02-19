// Author: Caden LeCluyse

#include "ui/ui.h"

#include <algorithm>
#include <gmpxx.h>
#include <iostream>
#include <mpfr.h>
#include <span>
#include <string>
#include <utility>

#include "include/util.hpp"
#include "version.hpp"

namespace UI {

void print_excessive_arguments(const int arguments) {
    std::cerr << "Expected 1 argument, received " << arguments
              << ". Use the --help flag to see all flags, or pass in an expression.\n"
              << "Make sure to wrap the expression in quotes.\n\n";
}

void print_insufficient_arguments() {
    std::cerr << "Expected an argument to be passed in. Use the --help flag to see all available flags, or pass in an "
                 "expression to be evaluated.\nWrap the expression in single quotes.\n\n";
}

void print_help_continuous() {
    std::cout << "* Enter 'history' to view your history.\n"
              << "* Enter 'save' to save your program history to a file.\n"
              << "* Enter 'clear' to clear your history.\n"
              << "* Enter 'exit', 'quit', or 'q' to exit the program.\n\n";
}

void print_result(const std::string_view result) {
    std::cout << "Result: " << result << '\n';
}

void print_error(const std::string_view error) {
    std::cerr << "Error: " << error << '\n';
}

std::string print_mpfr(const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::string buffer;
    buffer.resize(Util::buffer_size);
    if(!Util::convert_mpfr_char_vec(buffer, final_value, display_precision)) [[unlikely]] return "";
    UI::print_result(buffer);
    return buffer; 
}

void print_history(const std::span<const std::pair<std::string, std::string> > history) {
    std::ranges::for_each(history, [](const auto& expression_result) {
        const auto& [expression, result] = expression_result;
        std::cout << "Expression: " << expression << "\nResult: " << result << "\n";
    });
    std::cout << std::endl;
}

void print_version() {
    std::cout << "Version: " << PROGRAM_VERSION_MAJOR << "." << PROGRAM_VERSION_MINOR << "." << PROGRAM_VERSION_PATCH
              << "\n\n";
}

void print_help() {
    std::cout << "* Available flags:\n"
              << "\t - The [-c|--continuous] flag starts the program in continuous mode. You will be prompted for "
                 "expressions until you exit.\n"
              << "\t - The [-f|--file] flag runs the program in file mode. Launching the program in this mode will "
                 "take a list of expressions from expressions.txt and place the results in results.txt.\n\t   The "
                 "expressions.txt file must be placed in the current working directory."
              << std::endl
              << "\t - The [-v|--version] flag prints the version of the program.\n"
              << "\t - The [-h|--help] flag prints this screen.\n\n"
              << "* If no flags are passed in, the program expects an expression to be passed in. Wrap the expression "
                 "in single quotes.\n"
              << "\t - An expression is any valid combination of parentheses, boolean values or numbers (T and F for True and "
                 "False), and boolean/arithmetic operations.\n\n"
              << "* Boolean operations:\n"
              << "\t - AND (&) results in True when both values are True. (T & F = F).\n"
              << "\t - OR (|) returns True when at least one of the values is True. (T | F = T).\n"
              << "\t - XOR (^) reults in True only when one of the values is True. (F ^ F = F).\n"
              << "\t - NAND (@) returns True when both values are not True simultaneously. (T @ F = T).\n"
              << "\t - NOR ($) returns True when both values are false. (F $ F = T).\n"
              << "\t - NOT (!) negates the value it is in front of. (!F = T).\n\n"
              << "* Arithmetic operations:\n"
              << "\t - Addition (+) Adds two numbers together (2 + 2 = 4).\n"
              << "\t - Subtraction (-) Subtracts two numbers (3 - 2 = 1).\n"
              << "\t - Multiplication (*) Performs repeated addition (3 * 3 = 3 + 3 + 3 = 9).\n"
              << "\t - Division (/) Performs repeated subtraction, and counts the number of times it takes to get the divend to equal 0 (9 / 3 = 9 - 3 - 3 - 3 = 3).\n"
              << "\t - Exponent (^) Multiplies a number by itself a certain number of times (3^3 = 3 * 3 * 3 = 27).\n"
              << "\t - Factorial (!) Multiplies a number by every integer less than itself counting to 1 (4! = 4 * 3 * 2 * 1 = 24).\n"
              << "\t - e and pi are supported built-in variables.\n"
              << "\t - sin(), cos(), tan() are supported trig operations.\n\n"
              << "* Program settings:\n"
              << "\t - You can modify the settings of the program by editing ~/.config/ccalc/settings.ini\n"
              << "\t - The 'precision=' field is set in bits, and it modifies the precision of internal computations (default = 320).\n"
              << "\t - The 'display_digits=' field is set in digits, and it modifies the precision when printing the result (default = 15).\n"
              << "\t - The 'max_history=' field sets the maximum entries of the history when in continuous mode (default = 50).\n"
              << "\t - The 'angle=' setting specifies whether the program is using radians or degrees. 0 means radians, 1 means degrees (default = 0).\n"

              << std::endl;
}

void print_invalid_flag(const std::string_view expression) {
    std::cerr << "Error: " << expression << " is an invalid flag\n\n";
}

}  // namespace UI
