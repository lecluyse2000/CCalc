// Author: Caden LeCluyse

#include "file.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <mpfr.h>
#include <optional>
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

// We have to use C style file output here since mpfr is a C library
namespace File {

void output_history(const std::vector<std::pair<std::string, std::string> >& history, 
                    std::ofstream& output_file) noexcept {
    std::ranges::for_each(history, [&output_file](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        output_file << "Expression: " << expression << "\nResult: " << result << "\n";
    });
}

namespace {

std::vector<std::string> get_expressions() noexcept {
    std::vector<std::string> expressions;
    const std::optional<std::string> buffer = Util::get_filename(false);
    if (!buffer) return expressions;
    std::ifstream input_file(*buffer);
    std::string line;

    if (input_file.is_open()) {
        while (std::getline(input_file, line)) {
            if (line.empty()) continue; // Skip blank lines
            expressions.emplace_back(std::move(line));
        }
    } else {
        std::cout << "Couldn't find " << *buffer << '\n';
    }

    return expressions;
}


// Prints an MPFR float using the two functions defined above
bool mpfr_to_file(FILE*& output_file, const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::vector<char> buffer(Util::buffer_size);
    if(!Util::convert_mpfr_char_vec(buffer, final_value, display_precision)) return false;
    fprintf(output_file, "Result: ");
    for (const auto c : buffer) {
        fprintf(output_file, "%c", c);
    }
    fprintf(output_file, "\n");
    return true;
}

void math_float_procedure(FILE*& output_file, const std::span<Types::Token> result) {
    try {
        const auto tree = std::make_unique<MathAST>(result, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        if (mpfr_integer_p(final_value)) {
            mpfr_fprintf(output_file, "Result: %.0Rf\n", final_value);
        } else {
            if(!mpfr_to_file(output_file, final_value, static_cast<mpfr_prec_t>(Startup::settings.at(Types::Setting::DISPLAY_PREC)))) return;
        }
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_int_procedure(FILE*& output_file, const std::span<Types::Token> result) {
    try {
        const auto tree = std::make_unique<MathAST>(result, false);
        const mpz_class final_value = tree->evaluate();
        gmp_fprintf(output_file, "Result: %Zd\n", final_value.get_mpz_t());
    } catch (const std::bad_alloc& err) {
        fprintf(output_file, "Error: The number grew too big!\n"); 
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_procedure(FILE*& output_file, const std::span<Types::Token> result, const bool is_floating_point) {
    if (is_floating_point) {
        math_float_procedure(output_file, result);
    } else {
        math_int_procedure(output_file, result);
    }
}

void bool_procedure(FILE*& output_file, const std::span<Types::Token> result) {
    const auto syntax_tree = std::make_unique<BoolAST>(result);
    if (syntax_tree->evaluate()) {
        fprintf(output_file, "Result: True\n");
    } else {
        fprintf(output_file, "Result: False\n");
    }
}

void main_loop(FILE*& output_file, std::string& expression) {
    const std::string orig_expression = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    Types::ParseResult result = Parse::create_prefix_expression(expression);

    fprintf(output_file, "Expression: %s\n", orig_expression.c_str());
    if (!result.success) {
        fprintf(output_file, "Error: %s", result.error_msg.c_str());
        return;
    }
    if(result.is_math) {
        math_procedure(output_file, result.result, result.is_floating_point);
    } else {
        bool_procedure(output_file, result.result);
    }
}

}  // namespace

void initiate_file_mode() {
    std::vector<std::string> expressions = get_expressions();
    if (expressions.empty()) return;
    const std::optional<std::string> output_file_name = Util::get_filename(true);
    if (!output_file_name) return;
    FILE* output_file;
    output_file = fopen(output_file_name->c_str(), "w");
    if(!output_file) [[unlikely]] {
        std::cerr << "Error opening file!\n";
        return;
    }
    for (auto& expression : expressions) {
        main_loop(output_file, expression);
    }
    fclose(output_file);
}

}  // namespace File
