// Author: Caden LeCluyse

#include "file.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <mpfr.h>
#include <string>
#include <stdio.h>
#include <unordered_map>
#include <vector>

#include "ast/ast.h"
#include "parser/parser.h"
#include "startup/startup.h"


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
    std::ifstream input_file("expressions.txt");
    std::string current_expression;

    if (input_file.is_open()) {
        while (std::getline(input_file, current_expression)) {
            expressions.emplace_back(std::move(current_expression));
        }
    } else {
        std::cout << "Couldn't find expressions.txt!\n";
    }

    return expressions;
}

void math_float_procedure(FILE*& output_file, const std::string_view result, const auto& settings) {
    try {
        const auto tree = std::make_unique<MathAST>(result, static_cast<mpfr_prec_t>(settings.at("precision")), true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        if (mpfr_integer_p(final_value)) {
            mpfr_fprintf(output_file, "Result: %.0Rf\n", final_value);
        } else {
            mpfr_fprintf(output_file, "Result: %.*Rf\n", static_cast<mpfr_prec_t>(settings.at("display_digits")), final_value);
        }
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_int_procedure(FILE*& output_file, const std::string_view result, const auto& settings) {
    try {
        const auto tree = std::make_unique<MathAST>(result, static_cast<mpfr_prec_t>(settings.at("precision")), false);
        const mpz_class final_value = tree->evaluate();
        gmp_fprintf(output_file, "Result: %Zd\n", final_value.get_mpz_t());
    } catch (const std::bad_alloc& err) {
        fprintf(output_file, "Error: The number grew too big!\n"); 
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_procedure(FILE*& output_file, const std::string_view result, const bool is_floating_point) {
    static const std::unordered_map<std::string, long> settings = Startup::source_ini();
    if (is_floating_point) {
        math_float_procedure(output_file, result, settings);
    } else {
        math_int_procedure(output_file, result, settings);
    }
}

void bool_procedure(FILE*& output_file, const std::string_view result) {
    const auto syntax_tree = std::make_unique<BoolAST>(result);
    if (syntax_tree->evaluate()) {
        fprintf(output_file, "Result: True!\n");
    } else {
        fprintf(output_file, "Result: False!\n");
    }
}

void main_loop(FILE*& output_file, std::string& expression) {
    const std::string orig_expression = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    const auto [result, success, is_math, is_floating_point] = Parse::create_prefix_expression(expression);

    fprintf(output_file, "Expression: %s\n", orig_expression.c_str());
    if (!success) {
        fprintf(output_file, "Error: %s", result.c_str());
        return;
    }
    if(is_math) {
        math_procedure(output_file, result, is_floating_point);
    } else {
        bool_procedure(output_file, result);
    }
}

}  // namespace

void initiate_file_mode() {
    std::vector<std::string> expressions = get_expressions();
    if (expressions.empty()) return;
    FILE* output_file;
    output_file = fopen("results.txt", "w");
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
