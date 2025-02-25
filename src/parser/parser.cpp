// Author: Caden LeCluyse
#include "parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>

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
check_for_number(MathParseState& state,  bool& floating_point) {
    if (std::isdigit(state.current_token)) {
        state.in_number = true;
        state.num_buffer += state.current_token;
        return std::optional<std::string>("");
    } else if (state.current_token == '.') {
        if (state.num_buffer.find('.') != std::string::npos) {
            return std::optional<std::string>("Multiple decimal points in number!\n");
        }
        state.in_number = true;
        state.num_buffer += state.current_token;
        floating_point = true;
        return std::optional<std::string>("");
    }

    return std::nullopt;
}

constexpr void 
check_for_unary(MathParseState& state) {
    const char next_token = *(state.itr + 1);
    if (((Types::is_math_operand(state.previous_token) || state.previous_token == '(') && next_token == '(') ||
         Types::is_math_operator(next_token)) {
        state.current_token = '~';
    }
}

constexpr void check_for_floating_point(MathParseState& state, std::string& infix_expression,
                                        bool& floating_point) {
    if (state.current_token == '/') {
        floating_point = true;
        return;
    }
    if (state.current_token == '^') {
        for (auto new_itr = state.itr.base(); new_itr != infix_expression.end(); ++new_itr) {
            if (*new_itr == '-' || *new_itr == '~') {
                floating_point = true;
                return;
            } else if(*new_itr == '(') continue;
            return;
        }
    }
}

inline void closing_parentheses_math(MathParseState& state, std::string& prefix_expression,
                                     std::stack<char>& op_stack) {
    if (state.in_number) {
        prefix_expression += state.num_buffer + ',';
        clear_num_buffer(state);
    }
    op_stack.push(state.current_token);
}

// When parsing a math expression a comma is used as a delimiter
[[nodiscard]]
std:: optional<std::string> open_parentheses_math(MathParseState& state, std::string& prefix_expression,
                                                  std::stack<char>& op_stack) {
    if (state.in_number) {
        prefix_expression += state.num_buffer + ',';
        clear_num_buffer(state);
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

bool math_operator_found(MathParseState& state, ParseResult& result,
                         std::string& infix_expression, std::stack<char>& op_stack) {
    if (state.in_number) {
        result.result += state.num_buffer + ',';
        clear_num_buffer(state);
        if (state.current_token == '+' && (Types::is_math_operator(*(state.itr + 1)) || *(state.itr + 1) == '(')) return true;
    }
    check_for_floating_point(state, infix_expression, result.is_floating_point);
    while (!op_stack.empty() && op_stack.top() != ')' && 
           Types::get_precedence(op_stack.top()) > Types::get_precedence(state.current_token)) {
        result.result.push_back(op_stack.top());
        result.result.push_back(',');
        op_stack.pop();
    }
    op_stack.push(state.current_token);
    return false;
}

[[nodiscard]]
std::optional<std::string> math_loop_body(MathParseState& state, ParseResult& result,
                                          std::string& infix_expression,
                                          std::stack<char>& op_stack) {
    state.current_token = *state.itr;
    if (state.current_token == '!') return std::optional<std::string>("! operator is not supported yet!\n");
    
    if (state.current_token == '-') check_for_unary(state);
    const auto num_check = check_for_number(state, result.is_floating_point); 
    if (num_check && *num_check == "") {
        state.previous_token = state.current_token;
        return std::nullopt;
    } else if (num_check && *num_check != "") return num_check;
    const auto checker_result = Error::error_math(state.current_token, state.previous_token);
    if (checker_result) return checker_result;

    if (Types::is_math_operator(state.current_token)) {
        const bool cont = math_operator_found(state, result, infix_expression, op_stack);
        if (cont) return std::nullopt; 
    } else if (state.current_token == ')') {
        closing_parentheses_math(state, result.result, op_stack);
    } else if (state.current_token == '(') {
        const auto open_parenthese_result = open_parentheses_math(state, result.result, op_stack);
        if (open_parenthese_result) return open_parenthese_result;
    } else {
        return Error::invalid_character_error_math(state.current_token);
    }
    state.previous_token = state.current_token;
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_math(std::string& infix_expression, ParseResult& result,
                                      std::stack<char>& operator_stack) {
    MathParseState state;
    if (infix_expression[0] == '-') infix_expression[0] = '~';
    
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = itr;
        const auto loop_result = math_loop_body(state, result, infix_expression, operator_stack);
        if (loop_result) return loop_result;
    }
    if (state.in_number) result.result += state.num_buffer;
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> bool_loop_body(BoolParseState state, std::string& prefix_expression,
                                          std::stack<char>& operator_stack) {
    // Grab the current token
    // If the token is an operand, simply add it to the string
    // if the token is an operator or a closing parentheses, add it to the stack
    // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
    // found
    state.current_token = *state.itr;
    // Check for various errors
    const auto checker_result = Error::error_bool(state.current_token, state.previous_token);
    if (checker_result) {
        return checker_result;
    }

    if (Types::isoperand(state.current_token)) {
        prefix_expression.push_back(state.current_token);
    } else if (Types::isnot(state.current_token) || Types::isoperator(state.current_token) || state.current_token == ')') {
        operator_stack.push(state.current_token);
    } else if (state.current_token == '(') {
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
        return Error::invalid_character_error_bool(state.current_token);
    }
    state.previous_token = state.current_token;
    return std::nullopt;
}

[[nodiscard]]
std::optional<std::string> parse_bool(const std::string_view infix_expression, std::string& prefix_expression,
                                      std::stack<char>& operator_stack) {
    BoolParseState state;

    // Traverse the string in reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = itr;
        const auto loop_result = bool_loop_body(state, prefix_expression, operator_stack);
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
    ParseResult parse_result; 

    const auto is_math = is_math_equation(infix_expression);
    if (!is_math) {
        parse_result.result = "No valid operators detected!\n";
        return parse_result;
    }

    const auto initial_checks = Error::initial_checks(infix_expression, *is_math);
    if (initial_checks) {
        parse_result.result = *initial_checks;
        return parse_result;
    }
    const auto error_parsing = *is_math ? parse_math(infix_expression, parse_result, operator_stack)
                                        : parse_bool(infix_expression, parse_result.result, operator_stack);
    if (error_parsing) {
        parse_result.result = *error_parsing;
        return parse_result;
    }
    const auto stack_result = clear_stack(parse_result.result, operator_stack, *is_math);
    if (stack_result) {
        parse_result.result = *stack_result;
        parse_result.is_math = *is_math;
        return parse_result; 
    }

    std::ranges::reverse(parse_result.result);
    parse_result.success = true;
    parse_result.is_math = *is_math;
    return parse_result;
}

}
