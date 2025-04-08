// Author: Caden LeCluyse

#include "ui.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <mpfr.h>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <vector>

#include "ast/ast.h"
#include "file/file.h"
#include "include/types.hpp"
#include "include/util.hpp"
#include "parser/parser.h"
#include "startup/startup.h"
#include "version.hpp"

using namespace Types;

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

namespace {

void print_help_continuous() {
    std::cout << "* Enter 'history' to view your history.\n"
              << "* Enter 'save' to save your program history to a file.\n"
              << "* Enter 'clear' to clear your history.\n"
              << "* Enter 'exit', 'quit', or 'q' to exit the program.\n\n";
}

void print_history(const std::span<const std::pair<std::string, std::string> > history) {
    std::ranges::for_each(history, [](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        std::cout << "Expression: " << expression << "\nResult: " << result << "\n";
    });
    std::cout << std::endl;
}

[[nodiscard]] bool save_history(const std::span<const std::pair<std::string, std::string> > history) {
    // Get the file from the user, then output the history to it
    const std::optional<std::string> filename = Util::get_filename(true);
    if (!filename) [[unlikely]] {
        return false;
    }

    std::ofstream output_file(*filename);
    if (!output_file.is_open()) [[unlikely]] {
        std::cerr << "Output file could not be created!\n\n";
        return false;
    }

    File::output_history(history, output_file);
    return true;
}

enum struct InputResult {
    CONTINUE_TO_EVALUATE,
    CONTINUE,
    QUIT_FAILURE,
    QUIT_SUCCESS
};

// Determines the status of the program based on the user input, return an enum defined above
[[nodiscard]] InputResult handle_input(std::string_view input_expression, 
                                       std::vector<std::pair<std::string, std::string> >& program_history) {
    if (input_expression == "help") {
        print_help_continuous();
        return InputResult::CONTINUE;
    } else if (input_expression == "history") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet\n";
            return InputResult::CONTINUE;
        }
        print_history(program_history);
        return InputResult::CONTINUE;
    } else if (input_expression == "save") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet\n";
            return InputResult::CONTINUE;
        }

        if (!save_history(program_history)) {
            return InputResult::QUIT_FAILURE;
        }
        std::cout << "History saved\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "clear") {
        program_history.clear();
        std::cout << "History cleared\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "quit" || input_expression == "exit" || input_expression == "q") {
        std::cout << "Exiting...\n" << std::endl;
        return InputResult::QUIT_SUCCESS;
    }

    return InputResult::CONTINUE_TO_EVALUATE;
}

inline void add_to_history(std::string& orig_input, std::string& final_val,
                           std::vector<std::pair<std::string, std::string> >& history) {
    if (history.size() + 1 >= static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY))) {
        history.erase(history.begin()); 
    }
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_val)));
}

inline void add_to_history(std::string& orig_input, std::string&& final_val, 
                           std::vector<std::pair<std::string, std::string> >& history) {
    if (history.size() + 1 >= static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY))) {
        history.erase(history.begin()); 
    }
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_val)));
}

// Prints an MPFR float using the two functions defined above
std::string print_mpfr(const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::vector<char> buffer(Util::buffer_size);
    if(!Util::convert_mpfr_char_vec(buffer, final_value, display_precision)) [[unlikely]] return "";
    std::cout << "Result: ";
    for (const char c : buffer) {
        std::cout << c;
    }
    std::cout << '\n';

    // Have to return using iterators since it's not null teriminated
    return std::string(buffer.begin(), buffer.end()); 
}

inline std::string trim_euler() {
    std::string euler_retval = std::string(euler);
    euler_retval.erase(static_cast<std::size_t>(Startup::settings.at(Setting::DISPLAY_PREC) + 2), std::string::npos);
    return euler_retval;
}

inline void print_euler(std::string& orig_input, auto& history) {
    std::string euler = trim_euler();
    std::cout << "Result: " << euler << '\n';
    add_to_history(orig_input, std::string(euler), history);
}

inline void print_pi(std::string& orig_input,
                     std::vector<std::pair<std::string, std::string> >& history) {
    mpfr_t pi;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    mpfr_const_pi(pi, MPFR_RNDN); 
    std::string pi_str = print_mpfr(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
    add_to_history(orig_input, pi_str, history);
    mpfr_free_cache();
    mpfr_clear(pi);
}

[[nodiscard]] bool check_num_input(std::string& orig_input, std::string& expression,
                                   std::vector<std::pair<std::string, std::string> >& history) {
    if (std::ranges::all_of(expression, ::isdigit)) {
        std::cout << "Result: " << expression << '\n';
        add_to_history(orig_input, expression, history);
        return true;
    } else if (expression == "E") {
        print_euler(orig_input, history);
        return true;
    } else if (expression == "PI") {
        print_pi(orig_input, history);
        return true;
    }
    return false;
}

// Make the tree, evaluate, print the result, then add it to the history
void math_float_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                          std::vector<std::pair<std::string, std::string> >& history) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(prefix_input, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        std::string final_val = print_mpfr(final_value,
                                           static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        if (final_val.empty()) [[unlikely]] return;
        add_to_history(orig_input, final_val, history);
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
    return;
}

void math_int_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                        std::vector<std::pair<std::string, std::string> >& history) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(prefix_input, false);
        const mpz_class final_value = tree->evaluate();
        std::cout << "Result: " << final_value.get_str() << '\n';
        add_to_history(orig_input, final_value.get_str(), history);
    } catch (const std::bad_alloc& err) {
        std::cerr << "Error: The number grew too big\n";
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
}

// Calls the float or int procedure based on float_point status
void math_procedure(std::string& orig_input, const ParseResult& result,
                    std::vector<std::pair<std::string, std::string> >& history) {
    if (result.is_floating_point) {
        math_float_procedure(orig_input, result.result, history);
    } else {
        math_int_procedure(orig_input, result.result, history);
    }
}

// Bool is easier than math, just solve and add to history
void bool_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                    std::vector<std::pair<std::string, std::string> >& history) {
    const auto syntax_tree = std::make_unique<BoolAST>();
    syntax_tree->build_ast(prefix_input);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True\n";
        add_to_history(orig_input, "True", history);
    } else {
        std::cout << "False\n";
        add_to_history(orig_input, "False", history);
    }
}

void evaluate_expression(std::string& orig_input, std::string& expression,
                         std::vector<std::pair<std::string, std::string> >& history) {
    if (check_num_input(orig_input, expression, history)) return;
    const ParseResult result = Parse::create_prefix_expression(expression);
    if (!result.success) {
        std::cerr << "Error: " << result.error_msg << '\n';
        return;
    }
    if(result.is_math) {
        math_procedure(orig_input, result, history);
    } else {
        bool_procedure(orig_input, result.result, history);
    }
}

}  // Anon namespace, all these functions are essentially defined as static 

[[nodiscard]] int program_loop() {
    std::string input_expression;
    std::vector<std::pair<std::string, std::string> > program_history;
    program_history.reserve(static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY)));

    while (true) {
        std::cout << "Please enter your expression, or enter help to see all available commands: ";

        // If the input fails for some reason
        if (!std::getline(std::cin, input_expression)) [[unlikely]] {
            std::cerr << "Unknown error ocurred in receiving input. Aborting...\n\n";
            return 1;
        } else if (input_expression.empty()) {
            std::cerr << "Error: Empty input received\n";
            continue;
        }
        std::string orig_input = input_expression;
        // Remove spaces from the user's input
        input_expression.erase(remove(input_expression.begin(), input_expression.end(), ' '), input_expression.end());
        const InputResult result = handle_input(input_expression, program_history);

        // Based upon the input the program exits, continues, or evaluates the expression
        switch (result) {
            case InputResult::QUIT_SUCCESS:
                return 0;
            case InputResult::QUIT_FAILURE:
                return 1;
            case InputResult::CONTINUE:
                continue;
            default:
                std::ranges::transform(input_expression, input_expression.begin(), [](const auto c){ return std::toupper(c); });
                evaluate_expression(orig_input, input_expression, program_history);
        }
    }
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
              << "\t - XOR ($) reults in True only when one of the values is True. (F $ F = F).\n"
              << "\t - NAND (@) returns True when both values are not True simultaneously. (F @ F = T).\n"
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

// Overloaded functions for when the program is not in continuous mode
// When just passing expressions in at runtime there is no need to worry about the history
namespace {


[[nodiscard]] bool check_num_input(std::string& expression) {
    if (std::ranges::all_of(expression, ::isdigit)) {
        std::cout << "Result: " << expression << '\n';
        return true;
    } else if (expression == "E") {
        std::cout << "Result: " << trim_euler() << '\n';
        return true;
    } else if (expression == "PI") {
        mpfr_t pi;
        mpfr_init2(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
        mpfr_const_pi(pi, MPFR_RNDN); 
        print_mpfr(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        mpfr_free_cache();
        mpfr_clear(pi);
        return true;
    }
    return false;
}

void math_procedure(const ParseResult& result) {
    if (result.is_floating_point) {
        try {
            const auto tree = std::make_unique<MathAST>();
            tree->build_ast(result.result, result.is_floating_point);
            const mpfr_t& final_value = tree->evaluate_floating_point();
            print_mpfr(final_value, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        } catch (const std::exception& err) {
            std::cerr << "Error: " << err.what() << '\n';
        }
        return;
    }
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(result.result, result.is_floating_point);
        const mpz_class final_value = tree->evaluate();
        std::cout << "Result: " << final_value.get_str() << '\n';
    } catch (const std::bad_alloc& err) {
        std::cerr << "Error: The number grew too big!\n";
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
}

void bool_procedure(const std::span<const Token> result) {
    const auto syntax_tree = std::make_unique<BoolAST>();
    syntax_tree->build_ast(result);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True\n";
    } else {
        std::cout << "False\n";
    }
}

}

// Non continuous mode
void evaluate_expression(std::string& expression) {
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    std::ranges::transform(expression, expression.begin(), [](const auto c){ return std::toupper(c); });
    if (check_num_input(expression)) return;

    const ParseResult result = Parse::create_prefix_expression(expression);
    if (!result.success) {
        std::cerr << "Error: " << result.error_msg << '\n';
        return;
    }
    if(result.is_math) {
        math_procedure(result);
    } else {
        bool_procedure(result.result);
    }
}

}  // namespace UI
