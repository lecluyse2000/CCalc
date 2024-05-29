// Author: Caden LeCluyse

#include "ui/ui.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "ast/ast.h"
#include "parser/parser.h"
#include "version.hpp"

namespace UI {

void print_excessive_arguments(int arguments) {
    std::cerr << "Expected 1 argument, received " << arguments
              << ". Use the --help flag to see all flags, or pass in an expression.\n"
              << "Make sure to wrap the expression in quotes.\n\n";
}

void print_insufficient_arguments() {
    std::cerr << "Expected an argument to be passed in. Use the --help flag to see all available flags, or pass in an "
                 "expression to be evaluated.\nWrap the expression in single quotes.\n\n";
}

namespace {

void evaluate_expression(const std::string_view expression, auto& history) {
    static Parser expression_parser;
    const auto [result, status] = expression_parser.create_prefix_expression(expression);
    if (!status) {
        std::cerr << "Error: " << result << std::endl;
        return;
    }
    const auto syntax_tree = std::make_unique<AST>(result);

    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True!\n\n";
        history.emplace_back(std::make_pair(expression, "True!"));
    } else {
        std::cout << "False!\n\n";
        history.emplace_back(std::make_pair(expression, "False!"));
    }
}

void print_history(const auto& history) {
    std::ranges::for_each(history, [](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        std::cout << "Expression: " << expression << "\nResult: " << result << "\n";
    });
    std::cout << std::endl;
}

}  // namespace

[[nodiscard]] int program_loop() {
    std::string input_expression;
    std::vector<std::pair<std::string, std::string> > program_history;

    while (true) {
        std::cout << "Please enter your boolean expression, or enter history to see all prior evaluated expressions "
                     "(enter exit, quit, or q to exit the program): ";
        // If the input fails for some reason
        if (!std::getline(std::cin, input_expression)) [[unlikely]] {
            std::cerr << "Unknown error ocurred in receiving input. Aborting...\n\n";
            return 1;
        } else if (input_expression == "history") {
            if (program_history.empty()) {
                std::cerr << "You haven't evaluated any expressions yet!\n\n";
            } else {
                print_history(program_history);
            }
            continue;
        } else if (input_expression.empty()) {
            std::cerr << "Error: Empty expression received!\n\n";
            continue;
        } else if (input_expression == "quit" || input_expression == "exit" || input_expression == "q") {
            std::cout << "Exiting...\n" << std::endl;
            return 0;
        }

        evaluate_expression(input_expression, program_history);
    }
}

void print_version() {
    std::cout << "Version: " << PROGRAM_VERSION_MAJOR << "." << PROGRAM_VERSION_MINOR << "." << PROGRAM_VERSION_PATCH
              << "\n\n";
}

void print_help() {
    std::cout << " * Available flags:\n"
              << "\t - The [-c|--continuous] flag starts the program in continuous mode. You will be prompted for "
              << "expressions until you exit.\n"
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
              << " * Boolean expressions:\n"
              << "\t - AND (&) results in True when both value are True. (T & F = F).\n"
              << "\t - OR (|) returns True when at lest one of the values is True. (T | F = T).\n"
              << "\t - XOR ($) reults in True only when one of the values is True. (F $ F = F).\n"
              << "\t - NAND (@) returns True when both values are not True simultaneously. (F @ F = T).\n"
              << "\t - NOT (!) negates the value it is in front of. (!F = T).\n"

              << std::endl;
}

void print_invalid_flag(const std::string_view expression) {
    std::cerr << "Error: " << expression << " is an invalid flag!\n\n";
}

void evaluate_expression(const std::string_view expression) {
    Parser expression_parser;
    const auto [result, status] = expression_parser.create_prefix_expression(expression);
    if (!status) {
        std::cerr << "Error: " << result << std::endl;
        return;
    }
    const auto syntax_tree = std::make_unique<AST>(result);

    std::cout << "Result: ";
    if (syntax_tree->evaluate()) {
        std::cout << "True!\n\n";
    } else {
        std::cout << "False!\n\n";
    }
}
}  // namespace UI
