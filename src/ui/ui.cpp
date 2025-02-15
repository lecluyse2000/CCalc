// Author: Caden LeCluyse

#include "ui.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "../ast/ast.h"
#include "../file/file.h"
#include "../parser/parser.h"
#include "../version.hpp"

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

[[nodiscard]] std::optional<std::string> get_filename() {
    std::string filename;
    char filename_flag = 'f';

    while (toupper(filename_flag) != 'Y') {
        std::cout << "What is the name of the file you would like to save to? ";
        if (!std::getline(std::cin, filename)) [[unlikely]] {
            std::cerr << "Unable to receive input! Aborting...\n\n";
            return std::nullopt;
        }

        std::cout << "You entered " << filename << " is this correct? (Y/N): ";
        if (!std::cin.get(filename_flag)) [[unlikely]] {
            std::cerr << "Unable to receive input! Aborting...\n\n";
            return std::nullopt;
        }

        while (toupper(filename_flag) != 'Y' && toupper(filename_flag) != 'N') {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Incorrect input. Try again (Y/N): ";
            if (!std::cin.get(filename_flag)) [[unlikely]] {
                std::cerr << "Unable to receive input! Aborting...\n\n";
                return std::nullopt;
            }
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return std::optional<std::string>(filename);
}

[[nodiscard]] bool save_history(const auto& history) {
    const std::optional<std::string> filename = get_filename();
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

enum class InputResult {
    CONTINUE_TO_EVALUATE,
    CONTINUE,
    QUIT_FAILURE,
    QUIT_SUCCESS
};

[[nodiscard]] InputResult handle_input(std::string_view input_expression, auto& program_history) {
    if (input_expression == "help") {
        print_help_continuous();
        return InputResult::CONTINUE;
    } else if (input_expression == "history") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet!\n\n";
            return InputResult::CONTINUE;
        }

        print_history(program_history);
        return InputResult::CONTINUE;
    } else if (input_expression == "save") {
        if (program_history.empty()) {
            std::cerr << "You haven't evaluated any expressions yet!\n\n";
            return InputResult::CONTINUE;
        }

        if (!save_history(program_history)) {
            return InputResult::QUIT_FAILURE;
        }
        std::cout << "History saved!\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "clear") {
        program_history.clear();
        std::cout << "History cleared!\n\n";
        return InputResult::CONTINUE;
    } else if (input_expression == "quit" || input_expression == "exit" || input_expression == "q") {
        std::cout << "Exiting...\n" << std::endl;
        return InputResult::QUIT_SUCCESS;
    }

    return InputResult::CONTINUE_TO_EVALUATE;
}

void evaluate_expression(std::string& expression, auto& history) {
    const auto [result, status, is_math, is_floating_point] = Parse::create_prefix_expression(expression);
    if (!status) {
        std::cerr << "Error: " << result << std::endl;
        return;
    }
    if(is_math) {
        const auto tree = std::make_unique<MathAST>(result, is_floating_point);
        if (is_floating_point) {
            const long double final_value = tree->evaluate_floating_point();
            std::cout << "Result: " << final_value << '\n';
            history.emplace_back(std::make_pair(std::move(expression), std::to_string(final_value)));
            return;
        }
        const long long final_value = tree->evaluate();
        std::cout << "Result: " << final_value << '\n';
        history.emplace_back(std::make_pair(std::move(expression), std::to_string(final_value)));
        return;
    }

    const auto syntax_tree = std::make_unique<BoolAST>(result);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True!\n\n";
        history.emplace_back(std::make_pair(std::move(expression), "True!"));
    } else {
        std::cout << "False!\n\n";
        history.emplace_back(std::make_pair(std::move(expression), "False!"));
    }
}

}  // namespace

[[nodiscard]] int program_loop() {
    std::string input_expression;
    std::vector<std::pair<std::string, std::string> > program_history;

    while (true) {
        std::cout << "Please enter your boolean expression, or enter help to see all available commands: ";

        // If the input fails for some reason
        if (!std::getline(std::cin, input_expression)) [[unlikely]] {
            std::cerr << "Unknown error ocurred in receiving input. Aborting...\n\n";
            return 1;
        } else if (input_expression.empty()) {
            std::cerr << "Error: Empty expression received!\n\n";
            continue;
        }

        const InputResult result = handle_input(input_expression, program_history);

        switch (result) {
            case InputResult::QUIT_SUCCESS:
                return 0;
            case InputResult::QUIT_FAILURE:
                return 1;
            case InputResult::CONTINUE:
                continue;
            default:
                evaluate_expression(input_expression, program_history);
        }

    }
}

void print_version() {
    std::cout << "Version: " << PROGRAM_VERSION_MAJOR << "." << PROGRAM_VERSION_MINOR << "." << PROGRAM_VERSION_PATCH
              << "\n\n";
}

void print_help() {
    std::cout << " * Available flags:\n"
              << "\t - The [-c|--continuous] flag starts the program in continuous mode. You will be prompted for "
                 "expressions until you exit.\n"
              << "\t - The [-f|--file] flag runs the program in file mode. Launching the program in this mode will "
                 "take a list of expressions from expressions.txt and place the results in results.txt.\n\t   The "
                 "expressions.txt file must be placed in the current working directory."
              << std::endl
              << "\t - The [-v|--version] flag prints the version of the program.\n"
              << "\t - The [-h|--help] flag prints this screen.\n\n"
              << " * If no flags are passed in, the program expects an expression to be passed in. Wrap the expression "
                 "in single quotes.\n"
              << "\t - An expression is any valid combination of parentheses, boolean values (T and F for True and "
                 "False), and boolean operations.\n\n"
              << " * Boolean operations:\n"
              << "\t - AND (&) results in True when both value are True. (T & F = F).\n"
              << "\t - OR (|) returns True when at least one of the values is True. (T | F = T).\n"
              << "\t - XOR ($) reults in True only when one of the values is True. (F $ F = F).\n"
              << "\t - NAND (@) returns True when both values are not True simultaneously. (F @ F = T).\n"
              << "\t - NOT (!) negates the value it is in front of. (!F = T).\n"

              << std::endl;
}

void print_invalid_flag(const std::string_view expression) {
    std::cerr << "Error: " << expression << " is an invalid flag!\n\n";
}

void evaluate_expression(const std::string_view expression) {
    const auto [result, status, is_math, is_floating_point] = Parse::create_prefix_expression(expression);
    if (!status) {
        std::cerr << "Error: " << result << std::endl;
        return;
    }
    std::cout << "\n\nPrefix: " << result << "\n\n";
    if(is_math) {
        const auto tree = std::make_unique<MathAST>(result, is_floating_point);
        if (is_floating_point) {
            const long double final_value = tree->evaluate_floating_point();
            std::cout << "Result: " << final_value << "\n\n";
            return;
        }
        const long long final_value = tree->evaluate();
        std::cout << "Result: " << final_value << "\n\n";
        return;
    }

    const auto syntax_tree = std::make_unique<BoolAST>(result);
    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True!\n\n";
    } else {
        std::cout << "False!\n\n";
    }
}
}  // namespace UI
