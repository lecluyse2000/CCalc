// Author: Caden LeCluyse

#include "engine.h"

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
#include "ui/ui.h"

namespace Engine {

namespace {

[[nodiscard]] bool save_history(const std::span<const std::pair<std::string, std::string> > history) {
    // Get the file from the user, then output the history to it
    const std::optional<std::string> filename = Util::get_filename(true);
    if (!filename) [[unlikely]] {
        return false;
    }

    std::ofstream output_file(*filename);
    if (!output_file.is_open()) [[unlikely]] {
        UI::print_error("Output file could not be created!\n\n");
        return false;
    }

    File::output_history(history, output_file);
    return true;
}

// Determines the status of the program based on the user input, return an enum defined above
[[nodiscard]] InputResult handle_input(std::string_view input_expression, 
                                       std::vector<std::pair<std::string, std::string> >& program_history) {
    if (input_expression == "help") {
        UI::print_help_continuous();
        return InputResult::CONTINUE;
    } else if (input_expression == "history") {
        if (program_history.empty()) {
            UI::print_error("You haven't evaluated any expressions yet\n");
            return InputResult::CONTINUE;
        }
        UI::print_history(program_history);
        return InputResult::CONTINUE;
    } else if (input_expression == "save") {
        if (program_history.empty()) {
            UI::print_error("You haven't evaluated any expressions yet\n");
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
        std::cout << "Exiting...\n\n";
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

inline void print_pi(std::string& orig_input,
                     std::vector<std::pair<std::string, std::string> >& history) {
    mpfr_t pi;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    mpfr_const_pi(pi, MPFR_RNDN); 
    std::string pi_str = UI::print_mpfr(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
    add_to_history(orig_input, pi_str, history);
    mpfr_free_cache();
    mpfr_clear(pi);
}

inline std::string trim_euler() {
    std::string euler_retval = std::string(euler);

    // The "+ 2" is to keep the 2 and the decimal of e
    euler_retval.erase(static_cast<std::size_t>(Startup::settings.at(Setting::DISPLAY_PREC) + 2), std::string::npos);
    return euler_retval;
}

inline void print_euler(std::string& orig_input, auto& history) {
    std::string euler = trim_euler();
    UI::print_result(euler);
    add_to_history(orig_input, std::string(euler), history);
}

[[nodiscard]] bool check_num_input(std::string& orig_input, std::string& expression,
                                   std::vector<std::pair<std::string, std::string> >& history) {
    if (std::ranges::all_of(expression, ::isdigit)) {
        UI::print_result(expression);
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
        std::string final_val = UI::print_mpfr(final_value,
                                           static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        if (final_val.empty()) [[unlikely]] return;
        add_to_history(orig_input, final_val, history);
    } catch (const std::exception& err) {
        UI::print_error(err.what());
    }
    return;
}

void math_int_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                        std::vector<std::pair<std::string, std::string> >& history) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(prefix_input, false);
        const mpz_class final_value = tree->evaluate();
        UI::print_result(final_value.get_str());
        add_to_history(orig_input, final_value.get_str(), history);
    } catch (const std::bad_alloc& err) {
        UI::print_error("The number grew too big\n");
    } catch (const std::exception& err) {
        UI::print_error(err.what());
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
    if (syntax_tree->evaluate()) {
        UI::print_result("True");
        add_to_history(orig_input, "True", history);
    } else {
        UI::print_result("False");
        add_to_history(orig_input, "False", history);
    }
}

[[nodiscard]] bool check_num_input(std::string& expression) {
    if (std::ranges::all_of(expression, ::isdigit)) {
        UI::print_result(expression);
        return true;
    } else if (expression == "E") {
        UI::print_result(trim_euler());
        return true;
    } else if (expression == "PI") {
        mpfr_t pi;
        mpfr_init2(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
        mpfr_const_pi(pi, MPFR_RNDN); 
        UI::print_mpfr(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        mpfr_free_cache();
        mpfr_clear(pi);
        return true;
    }
    return false;
}

void evaluate_expression(std::string& orig_input, std::string& expression,
                         std::vector<std::pair<std::string, std::string> >& history) {
    if (check_num_input(orig_input, expression, history)) return;
    const ParseResult result = Parse::create_prefix_expression(expression);
    if (!result.success) {
        UI::print_error(result.error_msg);
        return;
    }
    if(result.is_math) {
        math_procedure(orig_input, result, history);
    } else {
        bool_procedure(orig_input, result.result, history);
    }
}

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
            UI::print_error("Empty input received");
            continue;
        }
        std::string orig_input = input_expression;
        // Remove spaces from the user's input
        input_expression.erase(remove(input_expression.begin(), input_expression.end(), ' '), input_expression.end());
        const Engine::InputResult result = handle_input(input_expression, program_history);

        // Based upon the input the program exits, continues, or evaluates the expression
        switch (result) {
            case Engine::InputResult::QUIT_SUCCESS:
                return 0;
            case Engine::InputResult::QUIT_FAILURE:
                return 1;
            case Engine::InputResult::CONTINUE:
                continue;
            default:
                std::ranges::transform(input_expression, input_expression.begin(), [](const auto c){ return std::toupper(c); });
                evaluate_expression(orig_input, input_expression, program_history);
        }
    }
}

void math_procedure_float(const ParseResult& result) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(result.result, result.is_floating_point);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        UI::print_mpfr(final_value, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
    } catch (const std::exception& err) {
        UI::print_error(err.what());
    }
}

void math_procedure_int(const ParseResult& result) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(result.result, result.is_floating_point);
        const mpz_class final_value = tree->evaluate();
        UI::print_result(final_value.get_str());
    } catch (const std::bad_alloc& err) {
        UI::print_error("Error: The number grew too big!");
    } catch (const std::exception& err) {
        UI::print_error(err.what());
    }
}

// Overloaded functions for non-continuous mode
void math_procedure(const ParseResult& result) {
    if (result.is_floating_point) {
        math_procedure_float(result);
    } else {
        math_procedure_int(result);
    }
}

void bool_procedure(const std::span<const Token> result) {
    const auto syntax_tree = std::make_unique<BoolAST>();
    syntax_tree->build_ast(result);
    if (syntax_tree->evaluate()) {
        UI::print_result("True");
    } else {
        UI::print_result("False");
    }
}

}

[[nodiscard]] int start_engine(const int argc, const char* const argv[]) {
    if (argc > 2) {
        UI::print_excessive_arguments(argc - 1);
        return 1;
    } else if (argc == 1) {
        UI::print_insufficient_arguments();
        return 1;
    }

    std::string expression = argv[1];
    if (expression == "-c" || expression == "--continuous") {
        return program_loop();
    } else if (expression == "-v" || expression == "--version") {
        UI::print_version();
        return 0;
    } else if (expression == "-f" || expression == "--file") {
        File::initiate_file_mode();
        return 0;
    } else if (expression == "-h" || expression == "--help") {
        UI::print_help();
        return 0;
    } else if (expression[0] == '-' && (expression.size() == 1 ||
              (expression.size() >= 2 && (!std::isdigit(expression[1]) && expression[1] != '(')))) {
        UI::print_invalid_flag(expression);
        return 1;
    }

    evaluate_expression(expression);
    return 0;
}

// Non continuous mode
void evaluate_expression(std::string& expression) {
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    std::ranges::transform(expression, expression.begin(), [](const auto c){ return std::toupper(c); });
    if (check_num_input(expression)) return;

    const ParseResult result = Parse::create_prefix_expression(expression);
    if (!result.success) {
        UI::print_error(result.error_msg);
        return;
    }
    if(result.is_math) {
        math_procedure(result);
    } else {
        bool_procedure(result.result);
    }
}

}
