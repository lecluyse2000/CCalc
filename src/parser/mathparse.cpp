#include "mathparse.h"

#include <optional>
#include <stack>
#include <string>

#include "include/error.hpp"
#include "include/types.hpp"

namespace MathParse {

namespace {

// This function will go through and add multiplication signs for implicit multiplication
// This is easier than having to rework the logic of the whole program
constexpr void add_mult_signs(std::string& infix) {
    for (std::size_t i = 0; i < infix.size() - 1; ++i) {
        const Types::Token current_token = static_cast<Types::Token>(infix[i]);
        const Types::Token next_token = static_cast<Types::Token>(infix[i + 1]);
        if (current_token == Types::Token::PI) continue;
        
        const auto insert_increment = [&i, &infix]() {
            infix.insert(i + 1, 1,'*');
            ++i; 
        };

        if (current_token == Types::Token::RIGHT_PAREN && (next_token == Types::Token::LEFT_PAREN || 
                                                           Types::is_math_operand(next_token))) {
            insert_increment();
        } else if (current_token == Types::Token::FAC && Types::is_math_operand(next_token)) {
            insert_increment();
        } else if (Types::is_math_var(current_token) && Types::is_math_var(next_token)) {
            insert_increment();
        } else if (Types::is_math_operand(current_token)
            && (Types::is_math_var(next_token) || next_token == Types::Token::LEFT_PAREN)) {
            insert_increment();
        }
    }
}

[[nodiscard]] constexpr
std::optional<std::string> parse_euler(MathParseState& state, Types::ParseResult& result) {
    const auto error_check = Error::variable_error(state.previous_token);
    if (error_check) return error_check;
    result.is_floating_point = true;
    state.in_number = true;
    state.num_buffer.push_back(Types::Token::EULER);
    return std::nullopt;
}

constexpr Types::Token get_next_token(const MathParseState& state) {
    return (*state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(*state.itr + 1))
                                          : Types::Token::NULLCHAR;
}

[[nodiscard]] constexpr
std::optional<std::string> parse_pi(MathParseState& state, Types::ParseResult& result) {
    const char next_token = static_cast<char>(get_next_token(state));
    if (next_token != 'P') return Error::invalid_character_error_math(**state.itr);
    const auto error_check = Error::variable_error(state.previous_token);
    if (error_check) return error_check;
    result.is_floating_point = true;
    state.in_number = true;
    state.num_buffer.push_back(Types::Token::PI);
    return std::nullopt;
}

[[nodiscard]] constexpr
std::optional<std::string> parse_var(MathParseState& state, Types::ParseResult& result) {
    if (**state.itr == 'E') {
        const auto euler_check = parse_euler(state, result); 
        if (euler_check) return euler_check;
    }

    // PI check
    if (**state.itr == 'I') {
        const auto pi_check = parse_pi(state, result); 
        if (pi_check) return pi_check;
        else (*state.itr)++;
    }
    return std::nullopt;
}

// Checks if the parser is currently in a number based on if the current token is a digit or decimal
// If a decimal place is found, the program goes into floating point mode
[[nodiscard]] constexpr std::optional<std::string> 
check_for_number(MathParseState& state, bool& floating_point) {
    if (std::isdigit(static_cast<char>(state.current_token))) {
        state.in_number = true;
        state.num_buffer.push_back(state.current_token);
        return std::optional<std::string>("");
    } else if (state.current_token == Types::Token::DOT) {
        state.in_number = true;
        state.num_buffer.push_back(state.current_token);
        floating_point = true;
        return std::optional<std::string>("");
    }

    return std::nullopt;
}

// Unary cases: 1) [not operand, close paren, or factorial] - (
constexpr bool unary_first_case(const MathParseState& state) {
    const Types::Token next_token = get_next_token(state);
    return state.previous_token == Types::Token::LEFT_PAREN && !Types::isoperand(next_token) &&
           next_token != Types::Token::RIGHT_PAREN && next_token != Types::Token::FAC;
}

// 2) ( - [operand]
constexpr bool unary_second_case(const MathParseState& state) {
    const Types::Token next_token = get_next_token(state);
    return next_token == Types::Token::LEFT_PAREN && Types::is_math_operand(state.previous_token);
}

// 3) [not factorial operator] - [operand]
constexpr bool unary_third_case(const MathParseState& state) {
    const Types::Token next_token = get_next_token(state); 
    return (Types::is_math_operator(next_token) && next_token != Types::Token::FAC)
            && Types::is_math_operand(state.previous_token);
}

// 4) Exponent raised to the negative power
constexpr bool unary_fourth_case(const MathParseState& state) {
    const Types::Token next_token = get_next_token(state); 
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
constexpr void check_for_floating_point(const MathParseState& state, bool& floating_point) {
    if (state.current_token == Types::Token::DIV) {
        floating_point = true;
        return;
    }
    // If raised to a negative exponent, we evaluate as a float
    // Have to iterate back in the other direction towards the end of the string
    if (state.current_token == Types::Token::POW) {
        for (auto new_itr = state.itr->base(); new_itr != state.end; ++new_itr) {
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

// This function returns true if unary plus is found
bool math_operator_found(MathParseState& state, Types::ParseResult& result,
                        std::stack<Types::Token>& op_stack) {
    if (state.in_number) {
        for (const auto token : state.num_buffer) {
            result.result.push_back(token);
        }
        result.result.push_back(Types::Token::COMMA);
        clear_num_buffer(state);
        const Types::Token next_token = (*state.itr + 1 != state.rend) ? static_cast<Types::Token>(*(*state.itr + 1))
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
    const auto var_check = parse_var(state, result);
    if (var_check) return var_check;
    if (!Types::is_valid_math_token(**state.itr)) {
        return Error::invalid_character_error_math(**state.itr);
    }
    state.current_token = static_cast<Types::Token>(**state.itr);
    if (state.current_token == Types::Token::SUB) check_for_unary(state);
    const auto num_check = check_for_number(state, result.is_floating_point); 
    if (num_check && num_check->empty()) {
        state.previous_token = state.current_token;
        return std::nullopt;
    } else if (num_check) {
        return num_check;
    }
    const auto checker_result = Error::error_math(state.current_token, state.previous_token);
    if (checker_result) return checker_result;

    if (Types::is_math_operator(state.current_token)) {
        const bool unary_plus = math_operator_found(state, result, op_stack);
        // If unary plus is found, return early and do not set previous token
        if (unary_plus) return std::nullopt; 
    } else if (state.current_token == Types::Token::RIGHT_PAREN) {
        closing_parentheses_math(state, result.result, op_stack);
    } else if (state.current_token == Types::Token::LEFT_PAREN) {
        const auto open_parenthese_result = open_parentheses_math(state, result.result, op_stack);
        if (open_parenthese_result) return open_parenthese_result;
    } else if (!Types::is_math_var(state.current_token)){
        return Error::invalid_character_error_math(**state.itr);
    }
    state.previous_token = state.current_token;
    return std::nullopt;
}

} // Anon namespace

[[nodiscard]]
std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                      std::stack<Types::Token>& operator_stack) {
    if (infix_expression[0] == '-') infix_expression[0] = '~';
    add_mult_signs(infix_expression);
    MathParseState state(infix_expression);
    
    // All the algorithms I discovered for converting to prefix started by reversing the string,
    // so I thought why not just parse from right to left so we don't have to reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = &itr;
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
