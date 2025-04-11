// Author: Caden LeCluyse
#include "parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>

#include "boolparse.h"
#include "include/error.hpp"
#include "include/types.hpp"
#include "include/util.hpp"
#include "mathparse.h"

using namespace Types;

namespace Parse {

namespace {

[[nodiscard]]
constexpr bool pow_search(const std::string_view& infix_expression, auto current_itr) noexcept {
    for (; current_itr != infix_expression.end(); ++current_itr) {
        if (*current_itr == '(') continue;
        if (*current_itr == 'F') return false;
        else if (*current_itr == 'T') {
            const std::string_view get_tan = infix_expression.substr(static_cast<std::size_t>(std::distance(infix_expression.begin(),
                                                                                              current_itr)), 3);
            if (get_tan == "TAN") return true;
            return false;
        }
    }
    
    return true;
}

[[nodiscard]]
constexpr std::optional<bool> is_math_equation(const std::string_view infix_expression) noexcept {
    for (auto itr = infix_expression.begin(); itr != infix_expression.end(); ++itr) {
        const char c = *itr;
        const char next_token = (itr + 1 != infix_expression.end()) ? *(itr + 1) : '\0';

        // Special check since ^ is exponent for math and xor for boolean
        if (c == '^') return pow_search(infix_expression, itr); 

        // Check if "TAN" is present in the expression, since TAN has a T and would mess things up
        if (c == 'T') {
            const std::string_view get_tan = infix_expression.substr(static_cast<std::size_t>(std::distance(infix_expression.begin(), itr)), 3);
            if (get_tan == "TAN") return true;
            return false;
        }
        if (c == '!' && is_bool_operand(static_cast<Token>(next_token))) {
            return false;
            // I think this is a more robust check than just looking for a digit
        } else if (std::isdigit(c) && (next_token == '!' || next_token == ')' ||
                                       is_math_var(static_cast<Token>(next_token)))) {
            return true;
        }
        if (is_math_var(static_cast<Token>(c)) && is_math_var(static_cast<Token>(next_token))) {
            return true;
        }
        if (is_bool_operator(static_cast<Token>(c))) {
            return false;
        } else if (is_math_operator(static_cast<Token>(c))) {
            return true;
        } else {
            continue;
        }
    }
    return std::nullopt;
} 


[[nodiscard]] std::optional<std::string_view>
clear_stack(std::vector<Token>& prefix_expression, std::stack<Token>& operator_stack, const bool is_math) {
    while (!operator_stack.empty()) {
        if (operator_stack.top() == Token::RIGHT_PAREN) {
            Util::empty_stack(operator_stack);
            return std::optional<std::string_view>("Missing open parentheses");
        }
        prefix_expression.push_back(operator_stack.top());
        if(is_math) prefix_expression.push_back(Token::COMMA);
        operator_stack.pop();
    }

    return std::nullopt;
}

}

// Takes in a standard expression string in infix form, and converts it to prefix
// This is a variation of the Shunting yard algorithm, invented by Dijkstra in 1961
[[nodiscard]]
ParseResult create_prefix_expression(std::string& infix_expression) {
    std::stack<Token> operator_stack;
    ParseResult parse_result; 

    const auto is_math = is_math_equation(infix_expression);
    if (!is_math) {
        parse_result.error_msg = "No valid operators detected";
        return parse_result;
    }

    const auto initial_checks = Error::initial_checks(infix_expression, *is_math);
    if (initial_checks) {
        parse_result.error_msg = *initial_checks;
        return parse_result;
    }
    const auto error_parsing = *is_math ? MathParse::parse_math(infix_expression, parse_result, operator_stack)
                                        : BoolParse::parse_bool(infix_expression, parse_result.result, operator_stack);
    if (error_parsing) {
        parse_result.error_msg = *error_parsing;
        return parse_result;
    }
    const auto stack_result = clear_stack(parse_result.result, operator_stack, *is_math);
    if (stack_result) {
        parse_result.error_msg = *stack_result;
        parse_result.is_math = *is_math;
        return parse_result; 
    }

    std::ranges::reverse(parse_result.result);
    parse_result.success = true;
    parse_result.is_math = *is_math;
    return parse_result;
}

}
