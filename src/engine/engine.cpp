// Author: Caden LeCluyse

#include "engine/engine.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <mpfr.h>
#include <optional>
#include <readline/readline.h>
#include <readline/history.h>
#include <span>
#include <string_view>
#include <unordered_map>

#include "ast/ast.h"
#include "engine/signal.h"
#include "file/file.h"
#include "include/types.hpp"
#include "include/util.hpp"
#include "parser/parser.h"
#include "startup/startup.h"
#include "ui/ui.h"

namespace Engine {

namespace {

[[nodiscard]] int check_argc(const int argc) {
    if (argc > 2) {
        UI::print_excessive_arguments(argc - 1);
        return 1;
    } else if (argc == 1) {
        UI::print_insufficient_arguments();
        return 1;
    }
    return 0;
}

inline void cleanup_history() {
    while (history_length) {
        HIST_ENTRY* entry = remove_history(0); 

        if (!entry) {
            UI::print_error("NULL pointer reached in cleanup_history! This should not happen");
            return;
        }
        Util::free_history_entry(entry);
        entry = nullptr;
    }
}
nn
inline void shutdown(const std::vector<std::pair<std::string, std::string> >& history,
                     const std::unordered_map<char, std::string>& var_map) {
    std::ofstream output;
    output.open(Startup::history_location, std::ios::trunc);
    if(output.is_open()) File::write_history(history, output);
    output.close();
    output.open(Startup::var_map_location);
    if(output.is_open()) File::write_vars(var_map, output);
    cleanup_history();
}

bool check_signal_flags(const std::vector<std::pair<std::string, std::string> >& history,
                        const std::unordered_map<char, std::string>& var_map) {
    if (Signal::signal_received()) {
        rl_free_line_state();
        rl_cleanup_after_signal();
        std::cout << '\n';
        shutdown(history, var_map);
        return true;
    }
    return false;
}

[[nodiscard]] bool save_history(const std::vector<std::pair<std::string, std::string> >& history) {
    // Get the file from the user, then output the history to it
    const std::optional<std::string> filename = Util::get_filename(true);
    if (!filename) [[unlikely]] {
        return false;
    }

    std::ofstream output_file(*filename);
    if (!output_file.is_open()) [[unlikely]] {
        UI::print_error("Output file could not be created!");
        return false;
    }

    File::output_history(history, output_file);
    return true;
}

[[nodiscard]] bool is_empty_history(const std::vector<std::pair<std::string, std::string> >& history) {
    if (history.empty()) {
        UI::print_error("You haven't evaluated any expressions yet");
        return true;
    }
    return false;
}

// Determines the status of the program based on the user input, return an enum defined in Types.hpp 
[[nodiscard]] InputResult handle_input(std::string_view input_expression,
                                       std::vector<std::pair<std::string, std::string> >& history) {
    if (input_expression == "help") {
        UI::print_help_continuous();
        return InputResult::CONTINUE;
    } else if (input_expression == "history") {
        if (is_empty_history(history)) return InputResult::CONTINUE;
        UI::print_history(history);
        return InputResult::CONTINUE;
    } else if (input_expression == "save") {
        if (is_empty_history(history)) return InputResult::CONTINUE;

        if (!save_history(history)) {
            return InputResult::QUIT_FAILURE;
        }
        std::cout << "History saved";
        return InputResult::CONTINUE;
    } else if (input_expression == "clear") {
        cleanup_history();
        history.clear();
        std::cout << "History cleared";
        return InputResult::CONTINUE;
    } else if (input_expression == "quit" || input_expression == "exit" || input_expression == "q") {
        std::cout << "Exiting...\n";
        return InputResult::QUIT_SUCCESS;
    }

    return InputResult::CONTINUE_TO_EVALUATE;
}

inline void add_to_history(std::string& orig_input, std::string&& final_value,
                           std::vector<std::pair<std::string, std::string> >& history) {
    if (history.size() + 1 > static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY))) {
        history.erase(history.begin());
    }
    add_history(orig_input.c_str());
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_value)));
}

inline void add_to_history(std::string& orig_input, std::string& final_value,
                           std::vector<std::pair<std::string, std::string> >& history) {
    if (history.size() + 1 > static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY))) {
        history.erase(history.begin());
    }
    add_history(orig_input.c_str());
    history.emplace_back(std::make_pair(std::move(orig_input), std::move(final_value)));
}

inline std::string print_pi(std::string& orig_input, std::vector<std::pair<std::string, std::string> >& history) {
    mpfr_t pi;
    mpfr_init2(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    mpfr_const_pi(pi, MPFR_RNDN); 
    std::string pi_str = UI::print_mpfr(pi, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
    std::string pi_copy = pi_str;
    add_to_history(orig_input, pi_str, history);
    mpfr_free_cache();
    mpfr_clear(pi);
    return pi_copy;
}

inline std::string trim_euler() {
    std::string euler_retval = std::string(euler);

    // The "+ 2" is to keep the 2 and the decimal of e
    euler_retval.erase(static_cast<std::size_t>(Startup::settings.at(Setting::DISPLAY_PREC) + 2), std::string::npos);
    return euler_retval;
}

inline std::string print_euler(std::string& orig_input, std::vector<std::pair<std::string, std::string> >& history) {
    std::string euler = trim_euler();
    UI::print_result(euler);
    const std::string euler_copy = euler;
    add_to_history(orig_input, euler, history);
    return euler_copy;
}

// Moving the strings into the map/history means that we have to remember to make copies
[[nodiscard]] std::string check_num_input(std::string& orig_input, std::string& expression,
                                          std::vector<std::pair<std::string, std::string> >& history,
                                          std::unordered_map<char, std::string>& var_map) {
    if (std::ranges::all_of(expression, ::isdigit)) {
        UI::print_result(expression);
        const std::string expr_copy = expression;
        add_to_history(orig_input, expression, history);
        return expr_copy;
    } else if (expression == "E") {
        return print_euler(orig_input, history);
    } else if (expression == "PI") {
        return print_pi(orig_input, history);
    } else if (expression.size() == 1 && var_map.contains(expression[0])) {
        UI::print_result(var_map.at(expression[0]));
        return var_map.at(expression[0]);
    }
    return "";
}

// Make the tree, evaluate, print the result, then add it to the history
std::string math_float_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                                 std::vector<std::pair<std::string, std::string> >& history) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(prefix_input, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        std::string final_val = UI::print_mpfr(final_value,
                                           static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)));
        if (final_val.empty()) [[unlikely]] return "";
        std::string final_val_copy = final_val;
        add_to_history(orig_input, final_val, history);
        return final_val_copy;
    } catch (const std::exception& err) {
        UI::print_error(err.what());
    }
    return "";
}

std::string math_int_procedure(std::string& orig_input, const std::span<const Token> prefix_input,
                               std::vector<std::pair<std::string, std::string> >& history) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(prefix_input, false);
        const mpz_class final_value = tree->evaluate();
        std::string final_val_copy = final_value.get_str();
        UI::print_result(final_value.get_str());
        add_to_history(orig_input, final_value.get_str(), history);
        return final_val_copy;
    } catch (const std::bad_alloc& err) {
        UI::print_error("The number grew too big");
    } catch (const std::exception& err) {
        UI::print_error(err.what());
    }
    return "";
}

// Calls the float or int procedure based on float_point status
std::string math_procedure(std::string& orig_input, const ParseResult& result,
                           std::vector<std::pair<std::string, std::string> >& history) {
    if (result.is_floating_point) {
        return math_float_procedure(orig_input, result.result, history);
    } else {
        return math_int_procedure(orig_input, result.result, history);
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

// \0 is what I decided to store ANS in. So we always need to save the answer in the var map to update ANS
void evaluate_expression(std::string& orig_input, std::string& expression,
                         std::vector<std::pair<std::string, std::string> >& history,
                         std::unordered_map<char, std::string>& var_map) {
    const char var_char = expression[1] == '=' ? static_cast<char>(std::toupper(expression[0])) : '\0';
    if (var_char != '\0') expression = expression.substr(2);
    if (expression.empty()) {
        UI::print_error("Empty input received");
        return;
    }
    std::string num_check = check_num_input(orig_input, expression, history, var_map);
    if (!num_check.empty()) {
        var_map.insert_or_assign(var_char, std::move(num_check));
        return;
    }
    const ParseResult result = Parse::create_prefix_expression(expression, var_map);
    if (!result.success) {
        UI::print_error(result.error_msg);
        return;
    }
    if(result.is_math) {
        std::string result_copy = math_procedure(orig_input, result, history);
        if (!result_copy.empty()) var_map.insert_or_assign(var_char, std::move(result_copy));
    } else {
        bool_procedure(orig_input, result.result, history);
    }
}

[[nodiscard]] int program_loop() {
    std::vector<std::pair<std::string, std::string> > history;
    history.reserve(static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY)));
    std::unordered_map<char, std::string> var_map;
    Startup::startup(history, var_map);

    while (true) {
        char* const input_expression = readline("Please enter your expression, or enter help to see all available commands: ");
        if (check_signal_flags(history, var_map)) return 1;

        // If the input fails
        if (!input_expression) [[unlikely]] {
            std::cerr << "Unknown error ocurred in receiving input. Aborting...\n";
            shutdown(history, var_map);
            return 1;
        }

        std::string input_expression_string(input_expression);
        free(input_expression);

        if (input_expression_string.empty()) {
            UI::print_error("Empty input received");
            continue;
        }
        std::string orig_input = input_expression_string;

        // Remove spaces from the user's input
        input_expression_string.erase(remove(input_expression_string.begin(), input_expression_string.end(), ' '), input_expression_string.end());
        const Engine::InputResult result = handle_input(input_expression_string, history);

        // Based upon the input the program exits, continues, or evaluates the expression
        switch (result) {
            case Engine::InputResult::QUIT_SUCCESS:
                shutdown(history, var_map);
                return 0;
            case Engine::InputResult::QUIT_FAILURE:
                shutdown(history, var_map);
                return 1;
            case Engine::InputResult::CONTINUE:
                continue;
            default:
                std::ranges::transform(input_expression_string, input_expression_string.begin(), [](const auto c){ return std::toupper(c); });
                evaluate_expression(orig_input, input_expression_string, history, var_map);
        }
    }
    
}

// Non continuous mode
void evaluate_expression(std::string& expression) {
    std::vector<std::pair<std::string, std::string> > history;
    history.reserve(static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY)));
    std::unordered_map<char, std::string> var_map;
    Startup::startup(history, var_map);

    std::string orig_input = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    if (expression.empty()) {
        UI::print_error("Empty input received");
        return;
    }

    std::ranges::transform(expression, expression.begin(), [](const auto c){ return std::toupper(c); });
    evaluate_expression(orig_input, expression, history, var_map);
    shutdown(history, var_map);
}

void history_flag() {
    std::vector<std::pair<std::string, std::string> > history;
    history.reserve(static_cast<std::size_t>(Startup::settings.at(Setting::MAX_HISTORY)));
    using_history();
    stifle_history(static_cast<int>(Startup::settings.at(Setting::MAX_HISTORY)));

    std::ifstream file;
    file.open(Startup::history_location);
    if(file.is_open()) File::read_history(history, file);

    if (!is_empty_history(history)) UI::print_history(history);
    cleanup_history();
}

}

[[nodiscard]] int start_engine(const int argc, const char* const argv[]) {
    if (check_argc(argc)) return 1;

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
    } else if (expression == "-H" || expression == "--history") {
        history_flag();
        return 0;
    } else if (expression[0] == '-' && (expression.size() == 1 ||
              (expression.size() >= 2 && (!std::isdigit(expression[1]) && expression[1] != '(')))) {
        UI::print_invalid_flag(expression);
        return 1;
    }

    evaluate_expression(expression);
    return 0;
}

}
