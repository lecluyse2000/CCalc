#include "mathparse.h"

#include <optional>
#include <stack>
#include <string>

#include "include/error.hpp"
#include "include/types.hpp"

namespace MathParse {

namespace {

constexpr void add_mult_signs(std::string& infix) {
    for (auto itr = infix.begin(); itr < infix.end(); ++itr) {
            const char current_token = *itr;
            const char next_token = (itr + 1 != infix.end()) ? *(itr + 1) : '\0';
            if (current_token == ')' && next_token == '(') infix.insert(itr + 1, '*');
            if (current_token == '!' && std::isdigit(next_token)) infix.insert(itr + 1, '*');
        }
}

// Checks if the parser is currently in a number based on if the current token is a digit or decimal
// If a decimal place is found, the program goes into floating point mode
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

// Unary cases: 1) [not operand, close paren, or factorial] - (
constexpr bool unary_first_case(MathParseState& state, const std::string& infix) {
    const char next_token = (state.itr + 1 != infix.rend()) ? *(state.itr + 1) : '\0';
    return state.previous_token == '(' && !Types::isoperand(next_token) && next_token != ')' &&
           next_token != '!';
}

// 2) ( - [operand]
constexpr bool unary_second_case(MathParseState& state, const std::string& infix) {
    const char next_token = (state.itr + 1 != infix.rend()) ? *(state.itr + 1) : '\0';
    return next_token == '(' && Types::is_math_operand(state.previous_token);
}

// 3) [not factorial operator] - [operand]
constexpr bool unary_third_case(MathParseState& state, const std::string& infix) {
    const char next_token = (state.itr + 1 != infix.rend()) ? *(state.itr + 1) : '\0';
    return (Types::is_math_operator(next_token) && next_token != '!')
            && Types::is_math_operand(state.previous_token);
}

// 4) Exponent raised to the negative power
constexpr bool unary_fourth_case(MathParseState& state, const std::string& infix) {
    const char next_token = (state.itr + 1 != infix.rend()) ? *(state.itr + 1) : '\0';
    return next_token == '^';
}

constexpr void 
check_for_unary(MathParseState& state, const std::string& infix) {
    if (unary_first_case(state, infix) || unary_second_case(state, infix) || unary_third_case(state, infix) ||
        unary_fourth_case(state, infix)) {
        state.current_token = '~';
    }
}

// If there is division or an exponent raised to a negative power, go into floating point mode
constexpr void check_for_floating_point(MathParseState& state, const std::string& infix_expression,
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
    op_stack.push(')');
}

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

bool math_operator_found(MathParseState& state, Types::ParseResult& result,
                         const std::string& infix_expression, std::stack<char>& op_stack) {
    if (state.in_number) {
        result.result += state.num_buffer + ',';
        clear_num_buffer(state);
        if (state.current_token == '+' &&
            ((Types::is_math_operator(*(state.itr + 1)) && *(state.itr + 1) != '!') || *(state.itr + 1) == '(')) return true;
    }
    check_for_floating_point(state, infix_expression, result.is_floating_point);
    if (state.current_token == '!') {
        op_stack.push(state.current_token);
        return false;
    }

     while (!op_stack.empty() && op_stack.top() != ')' &&
           Types::get_precedence(op_stack.top()) > Types::get_precedence(state.current_token)) {
        result.result.push_back(op_stack.top());
        result.result.push_back(',');
        op_stack.pop();
    }
    op_stack.push(state.current_token);
    
    return false;
}

// Main body of the loop for parsing math equations
[[nodiscard]]
std::optional<std::string> math_loop_body(MathParseState& state, Types::ParseResult& result,
                                          std::string& infix_expression,
                                          std::stack<char>& op_stack) {
    // Grab the current token
    // If the token is an operand, simply add it to the string
    // if the token is an operator or a closing parentheses, add it to the stack
    // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
    // found, or an operator with a higher precedence
    state.current_token = *state.itr;
    if (state.current_token == '-') check_for_unary(state, infix_expression);
    const auto num_check = check_for_number(state, result.is_floating_point); 
    if (num_check && num_check->empty()) {
        state.previous_token = state.current_token;
        return std::nullopt;
    } else if (num_check && num_check->empty()) return num_check;
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

} // Anon namespace

[[nodiscard]]
std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                      std::stack<char>& operator_stack) {
    MathParseState state;
    if (infix_expression[0] == '-') infix_expression[0] = '~';
    add_mult_signs(infix_expression);
    
    // All the algorithms I discovered for converting to prefix started by reversing the string,
    // so I thought why not just parse from right to left so we don't have to reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = itr;
        const auto loop_result = math_loop_body(state, result, infix_expression, operator_stack);
        if (loop_result) return loop_result;
    }
    if (state.in_number) result.result += state.num_buffer;
    return std::nullopt;
}

}
