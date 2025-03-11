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
            if (std::isdigit(current_token) && Types::is_math_var(static_cast<Types::Token>(next_token))) {
                infix.insert(itr + 1, '*');
            }
            if (Types::is_math_var(static_cast<Types::Token>(current_token)) &&
                Types::is_math_var(static_cast<Types::Token>(next_token))) {
                infix.insert(itr + 1, '*');
            }
        }
}

[[nodiscard]] constexpr
bool check_for_var(MathParseState& state, bool& floating_point) {
    if (state.current_token == Types::Token::EULER) {
        floating_point = true;
        state.in_number = true;
        state.num_buffer.push_back(state.current_token);
        return true;
    }
    return false;
}

// Checks if the parser is currently in a number based on if the current token is a digit or decimal
// If a decimal place is found, the program goes into floating point mode
[[nodiscard]] constexpr bool 
check_for_number(MathParseState& state, bool& floating_point) {
    if (check_for_var(state, floating_point)) return true;
    if (std::isdigit(static_cast<char>(state.current_token))) {
        state.in_number = true;
        state.num_buffer.push_back(state.current_token);
        return true;
    } else if (state.current_token == Types::Token::DOT) {
        state.in_number = true;
        state.num_buffer.push_back(state.current_token);
        floating_point = true;
        return true;
    }

    return false;
}

// Unary cases: 1) [not operand, close paren, or factorial] - (
constexpr bool unary_first_case(MathParseState& state) {
    const Types::Token next_token = (state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(state.itr + 1))
                                                                    : Types::Token::NULLCHAR;
    return state.previous_token == Types::Token::LEFT_PAREN && !Types::isoperand(next_token) &&
           next_token != Types::Token::RIGHT_PAREN && next_token != Types::Token::FAC;
}

// 2) ( - [operand]
constexpr bool unary_second_case(MathParseState& state) {
    const Types::Token next_token = (state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(state.itr + 1))
                                                                    : Types::Token::NULLCHAR;
    return next_token == Types::Token::LEFT_PAREN && Types::is_math_operand(state.previous_token);
}

// 3) [not factorial operator] - [operand]
constexpr bool unary_third_case(MathParseState& state) {
    const Types::Token next_token = (state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(state.itr + 1))
                                                                    : Types::Token::NULLCHAR;
    return (Types::is_math_operator(next_token) && next_token != Types::Token::FAC)
            && Types::is_math_operand(state.previous_token);
}

// 4) Exponent raised to the negative power
constexpr bool unary_fourth_case(MathParseState& state) {
    const Types::Token next_token = (state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(state.itr + 1))
                                                                    : Types::Token::NULLCHAR;
    return next_token == Types::Token::POW;
}

constexpr void 
check_for_unary(MathParseState& state) {
    if (unary_first_case(state) || unary_second_case(state) || unary_third_case(state) ||
        unary_fourth_case(state)) {
        state.current_token = Types::Token::UNARY;
    }
}

// If there is division or an exponent raised to a negative power, go into floating point mode
constexpr void check_for_floating_point(MathParseState& state, bool& floating_point) {
    if (state.current_token == Types::Token::DIV) {
        floating_point = true;
        return;
    }
    if (state.current_token == Types::Token::POW) {
        for (auto new_itr = state.itr.base(); new_itr != state.end; ++new_itr) {
            if (*new_itr == '-' || *new_itr == '~') {
                floating_point = true;
                return;
            } else if(*new_itr == '(') continue;
            return;
        }
    }
}

inline void closing_parentheses_math(MathParseState& state, std::vector<Types::Token>& prefix_expression,
                                     std::stack<Types::Token>& op_stack) {
    if (state.in_number) {
        for (const auto token : state.num_buffer) {
            prefix_expression.push_back(token);
        }
        prefix_expression.push_back(Types::Token::COMMA);
        clear_num_buffer(state);
    }
    op_stack.push(Types::Token::RIGHT_PAREN);
}

std::optional<std::string> open_parentheses_math(MathParseState& state, std::vector<Types::Token>& prefix_expression,
                                                  std::stack<Types::Token>& op_stack) {
    if (state.in_number) {
        for (const auto token : state.num_buffer) {
            prefix_expression.push_back(token);
        }
        prefix_expression.push_back(Types::Token::COMMA);
        clear_num_buffer(state);
    }
    while (!op_stack.empty() && op_stack.top() != Types::Token::RIGHT_PAREN) {
        prefix_expression.push_back(op_stack.top());
        prefix_expression.push_back(Types::Token::COMMA);
        op_stack.pop();
    }
    if (!op_stack.empty()) {
        op_stack.pop();
    } else {
        return std::optional<std::string>("Missing closing parentheses\n");
    }
    return std::nullopt;
}

bool math_operator_found(MathParseState& state, Types::ParseResult& result,
                        std::stack<Types::Token>& op_stack) {
    if (state.in_number) {
        for (const auto token : state.num_buffer) {
            result.result.push_back(token);
        }
        result.result.push_back(Types::Token::COMMA);
        clear_num_buffer(state);
        const Types::Token next_token = (state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(state.itr + 1))
                                                                      : Types::Token::NULLCHAR;
        if (state.current_token == Types::Token::ADD &&
            ((Types::is_math_operator(static_cast<Types::Token>(next_token)) && next_token != Types::Token::FAC)
            || next_token == Types::Token::LEFT_PAREN)) return true;
    }
    check_for_floating_point(state, result.is_floating_point);
    if (state.current_token == Types::Token::FAC) {
        op_stack.push(static_cast<Types::Token>(state.current_token));
        return false;
    }

     while (!op_stack.empty() && op_stack.top() != Types::Token::RIGHT_PAREN &&
           Types::get_precedence(op_stack.top()) > Types::get_precedence(state.current_token)) {
        result.result.push_back(op_stack.top());
        result.result.push_back(Types::Token::COMMA);
        op_stack.pop();
    }
    op_stack.push(static_cast<Types::Token>(state.current_token));
    
    return false;
}

// Main body of the loop for parsing math equations
[[nodiscard]]
std::optional<std::string> math_loop_body(MathParseState& state, Types::ParseResult& result,
                                          std::stack<Types::Token>& op_stack) {
    // Grab the current token
    // If the token is an operand, simply add it to the string
    // if the token is an operator or a closing parentheses, add it to the stack
    // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
    // found, or an operator with a higher precedence
    if (!Types::is_valid_math_token(*state.itr)) {
        return Error::invalid_character_error_math(*state.itr);
    }
    state.current_token = static_cast<Types::Token>(*state.itr);
    if (state.current_token == Types::Token::SUB) check_for_unary(state);
    const auto num_check = check_for_number(state, result.is_floating_point); 
    if (num_check) {
        state.previous_token = state.current_token;
        return std::nullopt;
    } 
    const auto checker_result = Error::error_math(state.current_token, state.previous_token);
    if (checker_result) return checker_result;

    if (Types::is_math_operator(state.current_token)) {
        const bool cont = math_operator_found(state, result, op_stack);
        if (cont) return std::nullopt; 
    } else if (state.current_token == Types::Token::RIGHT_PAREN) {
        closing_parentheses_math(state, result.result, op_stack);
    } else if (state.current_token == Types::Token::LEFT_PAREN) {
        const auto open_parenthese_result = open_parentheses_math(state, result.result, op_stack);
        if (open_parenthese_result) return open_parenthese_result;
    } 
    state.previous_token = state.current_token;
    return std::nullopt;
}

} // Anon namespace

[[nodiscard]]
std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                      std::stack<Types::Token>& operator_stack) {
    MathParseState state;
    state.rend = infix_expression.rend();
    state.end = infix_expression.end();
    if (infix_expression[0] == '-') infix_expression[0] = '~';
    add_mult_signs(infix_expression);
    
    // All the algorithms I discovered for converting to prefix started by reversing the string,
    // so I thought why not just parse from right to left so we don't have to reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = itr;
        const auto loop_result = math_loop_body(state, result, operator_stack);
        if (loop_result) return loop_result;
    }
    if (state.in_number) {
        for (const auto token : state.num_buffer) {
            result.result.push_back(token);
        }
    }
    return std::nullopt;
}

}
