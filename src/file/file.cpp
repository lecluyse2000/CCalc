// Author: Caden LeCluyse

#include "file/file.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <mpfr.h>
#include <optional>
#include <readline/history.h>
#include <span>
#include <string>
#include <stdio.h>
#include <unordered_map>
#include <vector>

#include "ast/ast.h"
#include "include/types.hpp"
#include "include/util.hpp"
#include "parser/parser.h"
#include "startup/startup.h"
#include "ui/ui.h"

using namespace Types;

// We have to use C style file output here since mpfr is a C library
namespace File {

// Utility function for outputting to a file for a user
void output_history(std::ofstream& output_file) {
    for (int i = history_base; i < history_base + history_length; ++i) {
        const HIST_ENTRY* const entry = history_get(i);

        if (!entry) {
            UI::print_error("NULL pointer reached in print_history! This should not happen");
            return;
        }
        
        output_file << "Expression: " << entry->line << "\nResult: " << static_cast<char*>(entry->data)
                    << "\nTime: " << entry->timestamp << '\n';
    }
}

// Utility function for outputting to a file on shutdown
void write_history(std::ofstream& output_file, const std::unordered_map<HIST_ENTRY*, std::string>& history) {
    while (history_length > 0) {
        HIST_ENTRY* entry = remove_history(0); 

        if (!entry) {
            UI::print_error("NULL pointer reached in print_history! This should not happen");
            return;
        }
        output_file << 'E' << (entry->line ? entry->line : "") << '\n'
                    << 'T' << (entry->timestamp ? entry->timestamp : "") << '\n'
                    << 'D' << history.at(entry) << '\n';
        Util::free_history_entry(entry);
    }
}

namespace { void add_entry(std::string& line, std::unordered_map<HIST_ENTRY*, std::string>& history) {
    if (line[0] == 'E') {
        add_history(line.c_str() + 1);
    } else if (line[0] == 'T') {
        add_history_time(line.c_str() + 1);
    } else if (line[0] == 'D') {
        line.erase(line.begin());
        HIST_ENTRY* const entry = history_get(history_base + history_length - 1);
        history.try_emplace(entry, std::move(line));
    }
}}

std::unordered_map<HIST_ENTRY*, std::string> read_history(std::ifstream& input_file) {
    std::unordered_map<HIST_ENTRY*, std::string> retval; 
    std::string line;
    while (std::getline(input_file, line)) {
        if (line.empty()) continue; // Skip blank lines
        add_entry(line, retval);
    }

    return retval;
}

namespace {

[[nodiscard]] std::vector<std::string> get_expressions() noexcept {
    std::vector<std::string> expressions;
    const std::optional<std::string> buffer = Util::get_filename(false);
    if (!buffer) [[unlikely]] return expressions;
    std::ifstream input_file(*buffer);
    std::string line;

    if (input_file.is_open()) {
        while (std::getline(input_file, line)) {
            if (line.empty()) continue; // Skip blank lines
            expressions.emplace_back(std::move(line));
        }
    } else {
        std::cerr << "Couldn't find " << *buffer << '\n';
    }

    return expressions;
}

bool mpfr_to_file(FILE*& output_file, const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::string buffer;
    buffer.reserve(Util::buffer_size);
    if(!Util::convert_mpfr_string(buffer, final_value, display_precision)) [[unlikely]] return false;
    fprintf(output_file, "Result: ");
    for (const auto c : buffer) {
        fprintf(output_file, "%c", c);
    }
    fprintf(output_file, "\n");
    return true;
}

void math_float_procedure(FILE*& output_file, const std::span<const Token> result) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(result, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        if (mpfr_integer_p(final_value)) {
            mpfr_fprintf(output_file, "Result: %.0Rf\n", final_value);
        } else {
            if(!mpfr_to_file(output_file, final_value,
                             static_cast<mpfr_prec_t>(Startup::settings.at(Setting::DISPLAY_PREC)))) [[unlikely]] return;
        }
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_int_procedure(FILE*& output_file, const std::span<const Token> result) {
    try {
        const auto tree = std::make_unique<MathAST>();
        tree->build_ast(result, false);
        const mpz_class final_value = tree->evaluate();
        gmp_fprintf(output_file, "Result: %Zd\n", final_value.get_mpz_t());
    } catch (const std::bad_alloc& err) {
        fprintf(output_file, "Error: The number grew too big!\n"); 
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_procedure(FILE*& output_file, const ParseResult& result) {
    if (result.is_floating_point) {
        math_float_procedure(output_file, result.result);
    } else {
        math_int_procedure(output_file, result.result);
    }
}

void bool_procedure(FILE*& output_file, const std::span<const Token> result) {
    const auto syntax_tree = std::make_unique<BoolAST>();
    syntax_tree->build_ast(result);
    if (syntax_tree->evaluate()) {
        fprintf(output_file, "Result: True\n");
    } else {
        fprintf(output_file, "Result: False\n");
    }
}

void main_loop(FILE*& output_file, std::string& expression) {
    const std::string orig_expression = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    std::ranges::transform(expression, expression.begin(), [](const auto c){ return std::toupper(c); });
    const ParseResult result = Parse::create_prefix_expression(expression);

    fprintf(output_file, "Expression: %s\n", orig_expression.c_str());
    if (!result.success) {
        fprintf(output_file, "Error: %s\n", result.error_msg.c_str());
        return;
    }
    if(result.is_math) {
        math_procedure(output_file, result);
    } else {
        bool_procedure(output_file, result.result);
    }
}

}  // namespace

void initiate_file_mode() {
    std::vector<std::string> expressions = get_expressions();
    if (expressions.empty()) [[unlikely]] return;
    const std::optional<std::string> output_file_name = Util::get_filename(true);
    if (!output_file_name) [[unlikely]] return;
    FILE* output_file;
    output_file = fopen(output_file_name->c_str(), "w");
    if(!output_file) [[unlikely]] {
        UI::print_error("Error opening file");
        return;
    }

    for (auto& expression : expressions) {
        main_loop(output_file, expression);
    }
    fclose(output_file);
}

}  // namespace File
