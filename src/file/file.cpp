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
#include <unistd.h>
#include <vector>

#include "../ast/ast.h"
#include "../parser/parser.h"

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

}  // namespace

void initiate_file_mode() noexcept {
    std::vector<std::string> expressions = get_expressions();
    FILE* output_file;
    output_file = fopen("expressions.txt", "w");
    if(!output_file) {
        std::cerr << "Error opening file!\n";
        return;
    }

    for (auto& expression : expressions) {
        expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
        const auto [result, success, is_math, is_floating_point] = Parse::create_prefix_expression(expression);

        fprintf(output_file, "Expression: %s\n", expression.c_str());
        if (!success) {
            fprintf(output_file, "Error: %s", result.c_str());
            continue;
        }
        if(is_math) {
            const auto tree = std::make_unique<MathAST>(result, is_floating_point);
            if (is_floating_point) {
                try {
                    const mpfr_t& final_value = tree->evaluate_floating_point();
                    mpfr_fprintf(output_file, "Result: %Rg\n", final_value);
                } catch (std::exception& err) {
                    fprintf(output_file, "Error: %s", err.what()); 
                }
                continue;
            }
            const mpz_class final_value = tree->evaluate();
            gmp_fprintf(output_file, "Result: %Zd\n", final_value.get_mpz_t());
            continue;
        }

        const auto syntax_tree = std::make_unique<BoolAST>(result);
        if (syntax_tree->evaluate()) {
            fprintf(output_file, "Result: True!\n");
        } else {
            fprintf(output_file, "Result: False!\n");
        }
    }
    fclose(output_file);
}

}  // namespace File
