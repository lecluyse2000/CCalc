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

void print_history(const auto& history) {
    std::ranges::for_each(history, [](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        std::cout << "Expression: " << expression << "\nResult: " << result << "\n";
    });
    std::cout << std::endl;
}

[[nodiscard]] bool save_history(const auto& history) {
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
[[nodiscard]] InputResult handle_input(std::string_view input_expression, auto& program_history) {
    if (input_expression == "help") {
        print_help_continuous();
        return InputResult::CONTINUE;
    } else if (input_expression == "history") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet!\n";
            return InputResult::CONTINUE;
        }
        print_history(program_history);
        return InputResult::CONTINUE;
    } else if (input_expression == "save") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet!\n";
            return InputResult::CONTINUE;
        }

        if (!save_history(program_history)) {
            return InputResult::QUIT_FAILURE;
        }
        std::cout << "History saved!\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "clear") {
        program_history.clear();
        std::cout << "History cleared!\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "quit" || input_expression == "exit" || input_expression == "q") {
        std::cout << "Exiting...\n" << std::endl;
        return InputResult::QUIT_SUCCESS;
    }

    return InputResult::CONTINUE_TO_EVALUATE;
}

// Prints an MPFR float using the two functions defined above
std::string print_mpfr(const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::vector<char> buffer(Util::buffer_size);
    if(!Util::convert_mpfr_char_vec(buffer, final_value, display_precision)) return std::string("");
    std::cout << "Result: ";
    for (const char c : buffer) {
        std::cout << c;
    }
    std::cout << '\n';

    // Have to return using iterators since it's not null teriminated
    return std::string(buffer.begin(), buffer.end()); 
}

inline void add_to_history(std::string& orig_input, std::string& final_val, auto& history) {
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_val)));
    if (history.size() >= static_cast<std::size_t>(Startup::settings.at(Types::Setting::MAX_HISTORY))) {
        history.erase(history.begin()); 
    }
}

inline void add_to_history(std::string& orig_input, std::string&& final_val, auto& history) {
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_val)));
    if (history.size() >= static_cast<std::size_t>(Startup::settings.at(Types::Setting::MAX_HISTORY))) {
        history.erase(history.begin()); 
    }
}

// Make the tree, evaluate, print the result, then add it to the history
void math_float_procedure(std::string& orig_input, const std::string_view prefix_input, auto& history) {
    try {
        const auto tree = std::make_unique<MathAST>(prefix_input, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        std::string final_val = print_mpfr(final_value, static_cast<mpfr_prec_t>(Startup::settings.at(Types::Setting::DISPLAY_PREC)));
        if (final_val.empty()) return;
        add_to_history(orig_input, final_val, history);
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
    return;
}

void math_int_procedure(std::string& orig_input, const std::string_view prefix_input, auto& history) {
    try {
        const auto tree = std::make_unique<MathAST>(prefix_input, false);
        const mpz_class final_value = tree->evaluate();
        std::cout << "Result: " << final_value.get_str() << '\n';
        add_to_history(orig_input, final_value.get_str(), history);
    } catch (const std::bad_alloc& err) {
        std::cerr << "Error: The number grew too big!\n";
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
}

// Calls the float or int procedure based on float_point status
void math_procedure(std::string& orig_input, const std::string_view prefix_input, const bool floating_point, auto& history) {
    if (floating_point) {
        math_float_procedure(orig_input, prefix_input, history);
    } else {
        math_int_procedure(orig_input, prefix_input, history);
    }
}

// Bool is easier than math, just solve and add to history
void bool_procedure(std::string& orig_input, const std::string_view prefix_input, auto& history) {
    const auto syntax_tree = std::make_unique<BoolAST>(prefix_input);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True\n";
        add_to_history(orig_input, "True", history);
    } else {
        std::cout << "False\n";
        add_to_history(orig_input, "False", history);
    }
}

void evaluate_expression(std::string& orig_input, std::string& expression, auto& history) {
    Types::ParseResult result = Parse::create_prefix_expression(expression);
    if (!result.success) {
        std::cerr << "Error: " << result.result;
        return;
    }
    if(result.is_math) {
        math_procedure(orig_input, result.result, result.is_floating_point, history);
    } else {
        bool_procedure(orig_input, result.result, history);
    }
}

}  // Anon namespace, all these functions are essentially defined as static 

[[nodiscard]] int program_loop() {
    std::string input_expression;
    std::vector<std::pair<std::string, std::string> > program_history;
    program_history.reserve(static_cast<std::size_t>(Startup::settings.at(Types::Setting::MAX_HISTORY)));

    while (true) {
        std::cout << "Please enter your expression, or enter help to see all available commands: ";

        // If the input fails for some reason
        if (!std::getline(std::cin, input_expression)) [[unlikely]] {
            std::cerr << "Unknown error ocurred in receiving input. Aborting...\n\n";
            return 1;
        } else if (input_expression.empty()) {
            std::cerr << "Error: Empty expression received!\n";
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
              << "\t - Division (/) Performs repeated subtraction (9 / 3 = 9 - 3 - 3 = 3).\n"
              << "\t - Exponent (^) Multiplies a number by itself a certain number of times (3^3 = 3 * 3 * 3 = 27).\n"
              << "\t - Factorial (!) Multiplies a number by every integer less than itself counting to 1 (4! = 4 * 3 * 2 * 1 = 24).\n\n"
              << "* Precision settings:\n"
              << "\t - You can modify the precision of the program by editing ~/.config/ccalc/settings.ini\n"
              << "\t - The 'precision=' field is set in bits, and it modifies the precision of internal computations (default = 320).\n"
              << "\t - The 'display_digits=' field is set in digits, and it modifies the precision when printing the result (default = 15)."

              << std::endl;
}

void print_invalid_flag(const std::string_view expression) {
    std::cerr << "Error: " << expression << " is an invalid flag!\n\n";
}

// Overloaded functions for when the program is not in continuous mode
// When just passing expressions in at runtime there is no need to worry about the history
namespace {

void math_procedure(const std::string_view result, const auto& settings, const bool floating_point) {
    const auto tree = std::make_unique<MathAST>(result, floating_point);
    if (floating_point) {
        try {
            const mpfr_t& final_value = tree->evaluate_floating_point();
            print_mpfr(final_value, static_cast<mpfr_prec_t>(settings.at(Types::Setting::DISPLAY_PREC)));
        } catch (const std::exception& err) {
            std::cerr << "Error: " << err.what() << '\n';
        }
        return;
    }
    try {
        const mpz_class final_value = tree->evaluate();
        std::cout << "Result: " << final_value.get_str() << '\n';
    } catch (const std::bad_alloc& err) {
        std::cerr << "Error: The number grew too big!\n";
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
    }
}

void bool_procedure(const std::string_view result) {
    const auto syntax_tree = std::make_unique<BoolAST>(result);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True!\n";
    } else {
        std::cout << "False!\n";
    }
}

}

// Non continuous mode
void evaluate_expression(std::string& expression) {
    const std::unordered_map<Types::Setting, long> settings = Startup::source_ini();
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    const auto [result, status, is_math, is_floating_point] = Parse::create_prefix_expression(expression);
    if (!status) {
        std::cerr << "Error: " << result;
        return;
    }
    if(is_math) {
        math_procedure(result, settings, is_floating_point);
    } else {
        bool_procedure(result);
    }
}

}  // namespace UI
