// Author: Caden LeCluyse
#include "parser/parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>
#include <string>
#include <string_view>

#include "boolparse.h"
#include "include/error.hpp"
#include "include/types.hpp"
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
constexpr bool contains_bool_op(const std::string_view& infix_expression) noexcept {
    for (auto itr = infix_expression.begin(); itr != infix_expression.end(); ++itr) {
        if (is_bool_operator(static_cast<Token>(*itr))) {
            if (*itr == '^' && !pow_search(infix_expression, itr)) {
                return true;
            } else if (*itr == '^') return false;
            return true;
        }
    }

    return false;
}

[[nodiscard]]
constexpr std::optional<bool> is_math_equation(const std::string_view infix_expression,
                                               const std::unordered_map<char, std::string>& var_map) noexcept {
    if (contains_bool_op(infix_expression)) return false;

    // From now on, we are operating on the assumption that there are no boolean operators
    for (auto itr = infix_expression.begin(); itr != infix_expression.end(); ++itr) {
        const char c = *itr;
        const char next_token = (itr + 1 != infix_expression.end()) ? *(itr + 1) : '\0';

        // Check if "TAN" is present in the expression, since TAN has a T and would mess things up
        if (c == 'T') {
            const std::string_view get_tan = infix_expression.substr(static_cast<std::size_t>(std::distance(infix_expression.begin(), itr)), 3);
            if (get_tan == "TAN" || var_map.contains('T')) return true;
            return false;
        } else if (c == 'F' && var_map.contains('F')) { 
            return true;
        } else if (c == 'F') return false;

        if (c == '!' && is_bool_operand(static_cast<Token>(next_token))) {
            return false;
            // I think this is a more robust check than just looking for a digit
            // This needs to be changed at some point
        } else if ((std::isdigit(c) || var_map.contains(c))  && (next_token == '!' || next_token == ')' ||
                                                                 is_math_var(static_cast<Token>(next_token)) ||
                                                                 var_map.contains(next_token) || next_token == 'A')) {
            return true;
        }
        if ((is_math_var(static_cast<Token>(c)) || var_map.contains(c)) &&
            (is_math_var(static_cast<Token>(next_token)) || var_map.contains(next_token))) {
            return true;
        } else if (is_math_operator(static_cast<Token>(c))) {
            return true;
        } 
    }
    return std::nullopt;
} 

constexpr bool check_ans(std::string& infix, const std::unordered_map<char, std::string>& var_map, std::size_t& index) {
    if (index + 2 >= infix.size()) return false;
    if (infix[index + 1] == 'N' && infix[index + 2] == 'S') {
        infix.replace(index, 3, "(" + var_map.at('\0') + ")");
        while (++index < infix.size() && infix[index] != ')');
        return true;
    }

    return false;
}

constexpr void expand_vars(std::string& infix, const std::unordered_map<char, std::string>& var_map) {
    for (std::size_t i = 0; i < infix.size(); ++i) {
        if (!var_map.contains(infix[i]) && infix[i] != 'A') continue;
        if (infix[i] == 'P' && i + 1 < infix.size() && infix[i + 1] == 'I') {
            i++;
            continue;
        }
        if (infix[i] == 'A' && i > 0 && infix[i - 1] == 'T' &&
            i + 1 < infix.size() && infix[i + 1] == 'N') {
            continue;
        }
        if (infix[i] == 'A' && check_ans(infix, var_map, i)) continue;

        infix.replace(i, 1, "(" + var_map.at(infix[i]) + ")");
        while (++i < infix.size() && infix[i] != ')');
    }
}

[[nodiscard]] std::optional<std::string_view>
constexpr clear_stack(std::vector<Token>& prefix_expression, std::stack<Token>& operator_stack, const bool is_math) {
    while (!operator_stack.empty()) {
        if (operator_stack.top() == Token::RIGHT_PAREN) {
            while (!operator_stack.empty()) operator_stack.pop();
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
ParseResult create_prefix_expression(std::string& infix_expression, const std::unordered_map<char, std::string>& var_map) {
    std::stack<Token> operator_stack;
    ParseResult parse_result; 

    const auto is_math = is_math_equation(infix_expression, var_map);
    if (!is_math) {
        parse_result.error_msg = "No valid operators detected";
        return parse_result;
    }
    if (*is_math) expand_vars(infix_expression, var_map);

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
    parse_result.is_math = *is_math;
    if (stack_result) {
        parse_result.error_msg = *stack_result;
        return parse_result; 
    }

    std::ranges::reverse(parse_result.result);
    parse_result.success = true;
    return parse_result;
}

}
