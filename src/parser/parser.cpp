// Author: Caden LeCluyse
#include "parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <utility>

#include "error.hpp"
#include "types.hpp"

namespace Parse {

namespace {

[[nodiscard]]
constexpr std::optional<bool> is_math_equation(const std::string_view infix_expression) noexcept {
    for (const auto c : infix_expression) {
        if (Types::is_bool_operator(c)) {
            return false;
        } else if (Types::is_math_operator(c)) {
            return true;
        } else {
            continue;
        }
    }
    return std::nullopt;
} 

[[nodiscard]] constexpr std::optional<std::string>
check_for_number(std::string& num_buffer, const char current_token, bool& in_number, bool& floating_point) {
    if (std::isdigit(current_token)) {
        in_number = true;
        num_buffer += current_token;
        return std::optional<std::string>("");
    } else if (current_token == '.') {
        if (num_buffer.find('.') != std::string::npos) {
            return std::optional<std::string>("Multiple decimal points in number!");
        }
        in_number = true;
        num_buffer += current_token;
        floating_point = true;
        return std::optional<std::string>("");
    }

    return std::nullopt;
}

constexpr void 
check_for_unary(const auto itr, char& current_token, const char previous_token) {
    const char next_token = *(itr + 1);
    if (((Types::is_math_operand(previous_token) || previous_token == '(') && next_token == '(') ||
         Types::is_math_operator(next_token)) {
        current_token = '~';
    }
}

constexpr void check_for_floating_point(const char current_token, const auto reverse_itr, std::string& infix_expression,
                                        bool& floating_point) {
    if (current_token == '/') {
        floating_point = true;
        return;
    }
    if (current_token == '^') {
        for (auto new_itr = reverse_itr.base(); new_itr != infix_expression.end(); ++new_itr) {
            if (*new_itr == '-' || *new_itr == '~') {
                floating_point = true;
                return;
            } else if(*new_itr == '(') continue;
            return;
        }
    }
}

inline void closing_parentheses_math(const char current_token, bool& in_number, std::string& prefix_expression,
                                     std::string& num_buffer,std::stack<char>& op_stack) {
    if (in_number) {
        prefix_expression += num_buffer + ',';
        clear_num_buffer(num_buffer, in_number);
    }
    op_stack.push(current_token);
}

// When parsing a math expression a comma is used as a delimiter
[[nodiscard]]
std:: optional<std::string> open_parentheses_math(bool& in_number, std::string& prefix_expression,
                                                  std::string& num_buffer, std::stack<char>& op_stack) {
    if (in_number) {
        prefix_expression += num_buffer + ',';
        clear_num_buffer(num_buffer, in_number);
    }
    while (!op_stack.empty() && op_stack.top() != ')') {
        prefix_expression.push_back(op_stack.top());
        prefix_expression.push_back(',');
        op_stack.pop();
    }
    if (!op_stack.empty()) {
        op_stack.pop();
    } else {
        return std::optional<std::string>("Missing closing parentheses!\n");
    }
    return std::nullopt;
}

bool math_operator_found(const char current_token, bool& floating_point, bool& in_number, std::string& prefix_expression,
                         std::string& infix_expression, auto itr, std::string& num_buffer,
                         std::stack<char>& op_stack) {
    if (in_number) {
        prefix_expression += num_buffer + ',';
        clear_num_buffer(num_buffer, in_number);
        if (current_token == '+' && (Types::is_math_operator(*(itr + 1)) || *(itr + 1) == '(')) return true;
    }
    check_for_floating_point(current_token, itr, infix_expression, floating_point);
    while (!op_stack.empty() && op_stack.top() != ')' && 
           Types::get_precedence(op_stack.top()) > Types::get_precedence(current_token)) {
        prefix_expression.push_back(op_stack.top());
        prefix_expression.push_back(',');
        op_stack.pop();
    }
    op_stack.push(current_token);
    return false;
}

[[nodiscard]]
std::optional<std::string> math_loop_body(char& current_token, char& previous_token, bool& floating_point,
                                          bool& in_number, std::string& prefix_expression,
                                          std::string& infix_expression, auto itr, std::string& num_buffer,
                                          std::stack<char>& op_stack) {
    current_token = *itr;
    if (current_token == '!') return std::optional<std::string>("! operator is not supported yet!\n");
    
    if (current_token == '-') check_for_unary(itr, current_token, previous_token);
    const auto num_check = check_for_number(num_buffer, current_token, in_number, floating_point); 
    if (num_check && *num_check == "") {
        previous_token = current_token;
        return std::optional<std::string>("");
    } else if (num_check && *num_check != "") return num_check;
    const auto checker_result = Error::error_math(current_token, previous_token);
    if (checker_result) return checker_result;

    if (Types::is_math_operator(current_token)) {
        const bool cont = math_operator_found(current_token, floating_point, in_number, prefix_expression,
                                              infix_expression, itr, num_buffer, op_stack);
        if (cont) return std::optional<std::string>("");
    } else if (current_token == ')') {
        closing_parentheses_math(current_token, in_number, prefix_expression, num_buffer, op_stack);
    } else if (current_token == '(') {
        const auto result = open_parentheses_math(in_number, prefix_expression, num_buffer, op_stack);
        if (result) return result;
    } else {
        return Error::invalid_character_error_math(current_token);
    }
    previous_token = current_token;
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_math(std::string& infix_expression, std::string& prefix_expression,
                                           std::stack<char>& operator_stack, bool& floating_point) {
    std::string num_buffer;
    char current_token = '\0';
    char previous_token = '\0';
    bool in_number = false;
    if (infix_expression[0] == '-') infix_expression[0] = '~';
    
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        const auto loop_result = math_loop_body(current_token, previous_token, floating_point, in_number,
                                                prefix_expression, infix_expression, itr, num_buffer, operator_stack);
        if (loop_result && !loop_result->empty() ) return loop_result;
    }
    if (in_number) prefix_expression += num_buffer;
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> bool_loop_body(char& current_token, char& previous_token, auto itr,
                                          std::string& prefix_expression, std::stack<char>& operator_stack) {
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
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_bool(const std::string_view infix_expression, std::string& prefix_expression,
                                      std::stack<char>& operator_stack) {
    char current_token = '\0';
    char previous_token = '\0';

    // Traverse the string in reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        const auto loop_result = bool_loop_body(current_token, previous_token, itr, prefix_expression, operator_stack);
        if (loop_result) return loop_result;
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<std::string_view>
clear_stack(std::string& prefix_expression, std::stack<char>& operator_stack, const bool is_math) {
    while (!operator_stack.empty()) {
        if (operator_stack.top() == ')') {
            empty_stack(operator_stack);
            return std::optional<std::string_view>("Missing open parentheses!\n");
        }
        prefix_expression.push_back(operator_stack.top());
        if(is_math) prefix_expression.push_back(',');
        operator_stack.pop();
    }

    return std::nullopt;
}

}

[[nodiscard]]
ParseResult create_prefix_expression(std::string& infix_expression) {
    std::stack<char> operator_stack;
    std::string prefix_expression;
    bool floating_point = false;

    const auto is_math = is_math_equation(infix_expression);
    if (!is_math) return ParseResult(std::string_view("No valid operators detected!\n"), false, false, floating_point);

    const auto initial_checks = Error::initial_checks(infix_expression, *is_math);
    if (initial_checks) {
        return ParseResult(*initial_checks, false, *is_math, floating_point);
    }
    const auto parse_result = *is_math ? parse_math(infix_expression, prefix_expression, operator_stack, floating_point)
                                       : parse_bool(infix_expression, prefix_expression, operator_stack);
    if (parse_result) {
        return ParseResult(*parse_result, false, *is_math, floating_point); 
    }
    const auto stack_result = clear_stack(prefix_expression, operator_stack, *is_math);
    if (stack_result) {
        return ParseResult(*stack_result, false, *is_math, floating_point); 
    }
    std::ranges::reverse(prefix_expression);

    return ParseResult(std::move(prefix_expression),true, *is_math, floating_point);
}

}
