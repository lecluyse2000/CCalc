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

void output_history(const std::vector<std::pair<const std::string, const std::string> >& history, 
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
            expressions.push_back(current_expression);
        }
    } else {
        std::cout << "Couldn't find expressions.txt!\n";
    }

    return expressions;
}

}  // namespace

void initiate_file_mode() noexcept {
    const std::vector<std::string> expressions = get_expressions();
    std::ofstream output_file("results.txt");

    for (const auto& expression : expressions) {
        const auto [result, status] = Parse::create_prefix_expression(expression);

        output_file << "Expression: " << expression << '\n';
        if (!status) {
            output_file << "Error: " << result;
            continue;
        }
        const auto syntax_tree = std::make_unique<AST>(result);

        if (syntax_tree->evaluate()) {
            output_file << "Result: True!\n";
        } else {
            output_file << "Result: False!\n";
        }
    }
}

}  // namespace File
