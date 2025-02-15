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

namespace {

[[nodiscard]]
constexpr std::optional<bool> is_math_equation(const std::string_view infix_expression) {
    for (const auto i : infix_expression) {
        if (Types::is_bool_operator(i)) {
            return false;
        } else if (Types::is_math_operator(i)) {
            return true;
        } else {
            continue;
        }
    }
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_math(const std::string_view infix_expression, std::string& prefix_expression,
                                           std::stack<char>& operator_stack, bool& floating_point) {
    std::string num_buffer;
    char current_token = '\0';
    char previous_token = '\0';
    bool in_number = false;
    
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        if (std::isspace(*itr)) {
            if (in_number) {
                prefix_expression += num_buffer + ',';
                in_number = false; 
                num_buffer.clear();
            }
            continue;
        } else if (*itr == '!') return std::optional<std::string>("! operator is not supported yet!\n");
        
        current_token = *itr;
        if (std::isdigit(current_token)) {
            in_number = true;
            num_buffer += current_token;
            continue;
        } else if (current_token == '.') {
            if (num_buffer.find('.') != std::string::npos) {
                return std::optional<std::string>("Multiple decimal points in number!");
            }
            in_number = true;
            num_buffer += current_token;
            floating_point = true;
            continue;
        }
        
        const auto checker_result = Error::error_math(current_token, previous_token);
        if (checker_result) {
            return checker_result;
        }

        if (Types::isoperator(current_token)) {
            if (in_number) {
                prefix_expression += num_buffer + ',';
                in_number = false;
                num_buffer.clear();
            }
            if (current_token == '/') {
                floating_point = true;
            }
            while (!operator_stack.empty() && operator_stack.top() != ')' && 
                   Types::get_precedence(operator_stack.top()) > Types::get_precedence(current_token)) {

                prefix_expression.push_back(operator_stack.top());
                prefix_expression.push_back(',');
                operator_stack.pop();
            }
            operator_stack.push(current_token);
        } else if (current_token == ')') {
            if (in_number) {
                prefix_expression += num_buffer + ',';
                in_number = false;
                num_buffer.clear();
            }
            operator_stack.push(current_token);
        } else if (current_token == '(') {
            if (in_number) {
                prefix_expression += num_buffer + ',';
                in_number = false;
                num_buffer.clear();
            }
            while (!operator_stack.empty() && operator_stack.top() != ')') {
                prefix_expression.push_back(operator_stack.top());
                prefix_expression.push_back(',');
                operator_stack.pop();
            }
            if (!operator_stack.empty()) {
                operator_stack.pop();
            } else {
                return std::optional<std::string>("Missing closing parentheses!\n");
            }
        } else {
            return Error::invalid_character_error_math(current_token);
        }
        
        previous_token = current_token;
    }
    if (in_number) {
        prefix_expression += num_buffer + ',';
    }
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_bool(const std::string_view infix_expression, std::string& prefix_expression,
                                 std::stack<char>& operator_stack) {
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
        const auto checker_result = Error::error_bool(current_token, previous_token);
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
                return std::optional<std::string>("Missing closing parentheses!\n");
            }
        } else {
            return Error::invalid_character_error_bool(current_token);
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
        prefix_expression.push_back(',');
        operator_stack.pop();
    }

    return std::nullopt;
}
}

[[nodiscard]]
ParseResult create_prefix_expression(const std::string_view infix_expression) {
    std::stack<char> operator_stack;
    std::string prefix_expression;
    bool floating_point = false;

    const auto is_math = is_math_equation(infix_expression);
    if (!is_math) return ParseResult(std::string_view("No valid operators detected!\n"), false, *is_math, floating_point);

    const auto initial_checks = Error::initial_checks(infix_expression, *is_math);
    if (initial_checks) {
        return ParseResult(*initial_checks, false, *is_math, floating_point);
    }
    const auto parse_result = *is_math ? parse_math(infix_expression, prefix_expression, operator_stack, floating_point)
                                       : parse_bool(infix_expression, prefix_expression, operator_stack);
    if (parse_result) {
        return ParseResult(*parse_result, false, *is_math, floating_point); 
    }
    const auto stack_result = clear_stack(prefix_expression, operator_stack);
    if (stack_result) {
        return ParseResult(*stack_result, false, *is_math, floating_point); 
    }
    std::ranges::reverse(prefix_expression);
    prefix_expression.erase(prefix_expression.begin());

    return ParseResult(std::move(prefix_expression),true, *is_math, floating_point);
}

}
