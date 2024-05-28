// Author: Caden LeCluyse

#include "parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <utility>

#include "error/error.hpp"
#include "types/types.hpp"

void Parser::empty_stack() noexcept {
    while (!m_operator_stack.empty()) {
        m_operator_stack.pop();
    }
}

[[nodiscard]]
std::optional<std::string> Parser::parse(const std::string_view infix_expression, std::string& prefix_expression) {
    // Traverse the string in reverse
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        // Ignore white space
        if (isspace(*itr)) {
            continue;
        }

        // Grab the current token
        // If the token is an operand, simply add it to the string
        // if the token is an operator or a closing parentheses, add it to the stack
        // If the token is an open parentheses, pop from the stack and add to the string until a closing parentheses is
        // found
        m_current_token = *itr;

        // Check for various errors
        const auto checker_result = Error::error_checker(m_current_token, m_previous_token);
        if (checker_result) {
            return checker_result;
        }

        if (Types::isoperand(m_current_token)) {
            prefix_expression.push_back(m_current_token);
        } else if (Types::isnot(m_current_token) || Types::isoperator(m_current_token) || m_current_token == ')') {
            m_operator_stack.push(m_current_token);
        } else if (m_current_token == '(') {
            while (!m_operator_stack.empty() && m_operator_stack.top() != ')') {
                prefix_expression.push_back(m_operator_stack.top());
                m_operator_stack.pop();
            }

            // Pop the closing parentheses off the stack
            if (!m_operator_stack.empty()) {
                m_operator_stack.pop();
            } else {
                return ("Missing closing parentheses!\n\n");
            }
        } else {
            return Error::invalid_character_error(m_current_token);
        }
        m_previous_token = m_current_token;
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<std::string> Parser::clear_stack(std::string& prefix_expression) {
    while (!m_operator_stack.empty()) {
        if (m_operator_stack.top() == ')') {
            empty_stack();
            return ("Missing open parentheses!\n\n");
        }
        prefix_expression.push_back(m_operator_stack.top());
        m_operator_stack.pop();
    }

    return std::nullopt;
}

[[nodiscard]] std::pair<std::string, bool> Parser::create_prefix_expression(const std::string_view infix_expression) {
    empty_stack();
    m_current_token = '\0';
    m_previous_token = '\0';
    std::string prefix_expression;

    const auto initial_checks = Error::initial_checks(infix_expression);
    if (initial_checks) {
        return std::make_pair(*initial_checks, false);
    }
    const auto parse_result = parse(infix_expression, prefix_expression);
    if (parse_result) {
        return std::make_pair(*parse_result, false);
    }
    const auto stack_result = clear_stack(prefix_expression);
    if (stack_result) {
        return std::make_pair(*stack_result, false);
    }
    std::ranges::reverse(prefix_expression);

    return std::make_pair(prefix_expression, true);
}
