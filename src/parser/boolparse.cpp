#include "parser/boolparse.h"

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
std::optional<std::string> bool_loop_body(BoolParseState& state, std::vector<Token>& prefix_expression, std::stack<Token>& operator_stack) {
    state.current_token = static_cast<Token>(*state.itr);

    const auto checker_result = Error::error_bool(state.current_token, state.previous_token);
    if (checker_result) {
        return checker_result;
    }

    if (is_bool_operand(state.current_token)) {
        prefix_expression.push_back(state.current_token);
    } else if (state.current_token == Token::RIGHT_PAREN) {
        operator_stack.push(state.current_token);
    } else if (isnot(state.current_token) || is_bool_operator(state.current_token)) {
        while (!operator_stack.empty() && operator_stack.top() != Token::RIGHT_PAREN &&
               get_bool_precedence(operator_stack.top()) > get_bool_precedence(state.current_token)) {
            prefix_expression.push_back(operator_stack.top());
            operator_stack.pop();
        }

        operator_stack.push(state.current_token);
    } else if (state.current_token == Token::LEFT_PAREN) {
        while (!operator_stack.empty() && operator_stack.top() != Token::RIGHT_PAREN) {
            prefix_expression.push_back(operator_stack.top());
            operator_stack.pop();
        }

        // Remove the matching closing parenthesis.
        if (!operator_stack.empty()) {
            operator_stack.pop();
        } else {
            return std::optional<std::string>("Missing closing parentheses");
        }
    } else {
        return Error::invalid_character_error_bool(*state.itr);
    }

    state.previous_token = state.current_token;
    return std::nullopt;
}

}  // namespace

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

}  // namespace BoolParse
