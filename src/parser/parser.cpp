// Author: Caden LeCluyse

#include "parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <utility>

#include "../error/error.hpp"
#include "../types/types.hpp"

namespace Parse {

[[nodiscard]]
std::optional<std::string_view> parse(const std::string_view infix_expression, std::string& prefix_expression,
                                 std::stack<char>& operator_stack) {
    bool math_equation = false;
    char current_token = '\0';
    char previous_token = '\0';

    // Traverse the string in reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        // Ignore white space
        if (std::isspace(*itr)) continue;

        // Grab the current token
        // If the token is an operand, simply add it to the string
        // if the token is an operator or a closing parentheses, add it to the stack
        // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
        // found
        current_token = *itr;

        // Check for various errors
        const auto checker_result = Error::error_checker(current_token, previous_token);
        if (checker_result) {
            return checker_result;
        }

        if (Types::isoperand(current_token)) {
            prefix_expression.push_back(current_token);
        } else if (Types::isnot(current_token) || Types::isoperator(current_token) || current_token == ')') {
            operator_stack.push(current_token);
        } else if (current_token == '(') {
            while (!operator_stack.empty() && operator_stack.top() != ')') {
                prefix_expression.push_back(operator_stack.top());
                operator_stack.pop();
            }

            // Pop the closing parentheses off the stack
            if (!operator_stack.empty()) {
                operator_stack.pop();
            } else {
                return std::optional<std::string_view>("Missing closing parentheses!\n");
            }
        } else {
            return Error::invalid_character_error(current_token);
        }
        previous_token = current_token;
    }

    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string_view> clear_stack(std::string& prefix_expression, std::stack<char>& operator_stack) {
    while (!operator_stack.empty()) {
        if (operator_stack.top() == ')') {
            empty_stack(operator_stack);
            return std::optional<std::string_view>("Missing open parentheses!\n");
        }
        prefix_expression.push_back(operator_stack.top());
        operator_stack.pop();
    }

    return std::nullopt;
}

[[nodiscard]]
std::pair<std::string, const bool> create_prefix_expression(const std::string_view infix_expression) {
    std::stack<char> operator_stack;
    std::string prefix_expression;

    const auto initial_checks = Error::initial_checks(infix_expression);
    if (initial_checks) {
        return std::make_pair(std::string(*initial_checks), false);
    }
    const auto parse_result = parse(infix_expression, prefix_expression, operator_stack);
    if (parse_result) {
        return std::make_pair(std::string(*parse_result), false);
    }
    const auto stack_result = clear_stack(prefix_expression, operator_stack);
    if (stack_result) {
        return std::make_pair(std::string(*stack_result), false);
    }
    std::ranges::reverse(prefix_expression);

    return std::make_pair(prefix_expression, true);
}

}
