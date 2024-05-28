// Author: Caden LeCluyse

#include "file.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.h"
#include "parser/parser.h"

namespace File {

namespace {

std::vector<std::string> get_expressions() {
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

void initiate_tests() {
    const std::vector<std::string> expressions = get_expressions();
    std::ofstream output_file("results.txt");

    for (const auto& expression : expressions) {
        Parser expression_parser;
        const auto [result, status] = expression_parser.create_prefix_expression(expression);
        
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
