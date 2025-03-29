#include "boolparse.h"

#include <cctype>
#include <optional>
#include <stack>
#include <string>

#include "include/error.hpp"
#include "include/types.hpp"

using namespace Types;

namespace BoolParse { 

namespace {

[[nodiscard]]
std::optional<std::string> bool_loop_body(BoolParseState& state, std::vector<Token>& prefix_expression,
                                          std::stack<Token>& operator_stack) {
    // Grab the current token
    // If the token is an operand, simply add it to the string
    // if the token is an operator or a closing parentheses, add it to the stack
    // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
    // found
    if (!is_valid_bool_token(*state.itr)) {
        return Error::invalid_character_error_bool(*state.itr);
    }
    state.current_token = static_cast<Token>(*state.itr);
    // Check for various errors
    const auto checker_result = Error::error_bool(state.current_token, state.previous_token);
    if (checker_result) {
        return checker_result;
    }

    if (is_bool_operand(state.current_token)) {
        prefix_expression.push_back(state.current_token);
    } else if (isnot(state.current_token) || isoperator(state.current_token) || state.current_token == Token::RIGHT_PAREN) {
        operator_stack.push(static_cast<Token>(state.current_token));
    } else if (state.current_token == Token::LEFT_PAREN) {
        while (!operator_stack.empty() && operator_stack.top() != Token::RIGHT_PAREN) {
            prefix_expression.push_back(operator_stack.top());
            operator_stack.pop();
        }

        // Pop the closing parentheses off the stack
        if (!operator_stack.empty()) {
            operator_stack.pop();
        } else {
            return std::optional<std::string>("Missing closing parentheses\n");
        }
    } else {
        return Error::invalid_character_error_bool(*state.itr);
    }
    state.previous_token = state.current_token;
    return std::nullopt;
}

}

[[nodiscard]]
std::optional<std::string> parse_bool(const std::string_view infix_expression, std::vector<Token>& prefix_expression,
                                      std::stack<Token>& operator_stack) {
    BoolParseState state;

    // Traverse the string in reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        state.itr = itr;
        const auto loop_result = bool_loop_body(state, prefix_expression, operator_stack);
        if (loop_result) return loop_result;
    }

    return std::nullopt;
}

}
