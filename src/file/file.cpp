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

void output_history(const std::span<const std::pair<std::string, std::string> > history, 
                    std::ofstream& output_file) noexcept {
    std::ranges::for_each(history, [&output_file](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        output_file << "Expression: " << expression << std::endl <<"Result: " << result << std::endl;
    });
}

namespace {

#ifdef _WIN32
    inline constexpr std::string_view escape_sequence = "\r\n";
#else
    inline constexpr std::string_view escape_sequence = "\n";
#endif

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

// Prints an MPFR float using the two functions defined above
bool mpfr_to_file(FILE*& output_file, const mpfr_t& final_value, const mpfr_prec_t display_precision) {
    std::vector<char> buffer(Util::buffer_size);
    if(!Util::convert_mpfr_char_vec(buffer, final_value, display_precision)) [[unlikely]] return false;
    fprintf(output_file, "Result: ");
    for (const auto c : buffer) {
        fprintf(output_file, "%c", c);
    }
    fprintf(output_file, "%s", escape_sequence.data());
    return true;
}

void math_float_procedure(FILE*& output_file, const std::span<const Types::Token> result) {
    try {
        const auto tree = std::make_unique<const MathAST>(result, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        if (mpfr_integer_p(final_value)) {
            mpfr_fprintf(output_file, "Result: %.0Rf%s", final_value, escape_sequence.data());
        } else {
            if(!mpfr_to_file(output_file, final_value,
                             static_cast<mpfr_prec_t>(Startup::settings.at(Types::Setting::DISPLAY_PREC)))) [[unlikely]] return;
        }
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s%s", err.what(), escape_sequence.data()); 
    }
}

void math_int_procedure(FILE*& output_file, const std::span<const Types::Token> result) {
    try {
        const auto tree = std::make_unique<const MathAST>(result, false);
        const mpz_class final_value = tree->evaluate();
        gmp_fprintf(output_file, "Result: %Zd%s", final_value.get_mpz_t(), escape_sequence.data());
    } catch (const std::bad_alloc& err) {
        fprintf(output_file, "Error: The number grew too big!%s", escape_sequence.data()); 
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s%s", err.what(), escape_sequence.data()); 
    }
}

void math_procedure(FILE*& output_file, const Types::ParseResult& result) {
    if (result.is_floating_point) {
        math_float_procedure(output_file, result.result);
    } else {
        math_int_procedure(output_file, result.result);
    }
}

void bool_procedure(FILE*& output_file, const std::span<const Types::Token> result) {
    const auto syntax_tree = std::make_unique<BoolAST>(result);
    if (syntax_tree->evaluate()) {
        fprintf(output_file, "Result: True%s", escape_sequence.data());
    } else {
        fprintf(output_file, "Result: False%s", escape_sequence.data());
    }
}

void main_loop(FILE*& output_file, std::string& expression) {
    const std::string orig_expression = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    std::ranges::transform(expression, expression.begin(), [](const auto c){ return std::toupper(c); });
    const Types::ParseResult result = Parse::create_prefix_expression(expression);

    fprintf(output_file, "Expression: %s%s", orig_expression.c_str(), escape_sequence.data());
    if (!result.success) {
        fprintf(output_file, "Error: %s%s", result.error_msg.c_str(), escape_sequence.data());
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
        std::cerr << "Error opening file!\n";
        return;
    }

    for (auto& expression : expressions) {
        main_loop(output_file, expression);
    }
    fclose(output_file);
}

}  // namespace File
