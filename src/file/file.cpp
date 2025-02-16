// Author: Caden LeCluyse

#include "file.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
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
    std::ofstream output_file("results.txt");

    for (auto& expression : expressions) {
        const auto [result, success, is_math, is_floating_point] = Parse::create_prefix_expression(expression);

        output_file << "Expression: " << expression << '\n';
        if (!success) {
            output_file << "Error: " << result;
            continue;
        }
        if(is_math) {
            const auto tree = std::make_unique<MathAST>(result, is_floating_point);
            if (is_floating_point) {
                const long double final_value = tree->evaluate_floating_point();
                output_file<< "Result: " << final_value;
                continue;
            }
            const long long final_value = tree->evaluate();
            output_file << "Result: " << final_value;
            continue;
        }

        const auto syntax_tree = std::make_unique<BoolAST>(result);
        if (syntax_tree->evaluate()) {
            output_file << "Result: True!\n";
        } else {
            output_file << "Result: False!\n";
        }
    }
}

}  // namespace File
