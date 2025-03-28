// Author: Caden LeCluyse

#ifndef ERROR_HPP
#define ERROR_HPP

#include <array>
#include <cctype>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "include/types.hpp"

using namespace Types;

namespace Error {

// Declare helper functions in an anonymous namespace
namespace {

[[nodiscard]] inline
constexpr std::optional<std::string> check_leading(const std::string_view infix_expression, const bool math) {
    if (infix_expression.size() == 1) {
        return std::optional<std::string>("Expression is only one character long");
    }
    if (math && is_math_operator(static_cast<Token>(infix_expression[0])) && infix_expression[0] != '-') {
        return std::optional<std::string>("Math expression begins with an operator");
    }
    if (!math && is_bool_operator(static_cast<Token>(infix_expression[0]))) {
        return std::optional<std::string>("Boolean expression begins with an operator");
    }
    if (infix_expression[0] == ')') {
        return std::optional<std::string>("Expression begins with closed parentheses");
    } else if (infix_expression[0] == '!' && math) {
        return std::optional<std::string>("Expression begins with factorial");
    }
    for (const auto i : infix_expression) {
        if (!math && std::isdigit(i)) {
            return std::optional<std::string>("Boolean expression contains a number");
        } else if (math && is_bool_operand(static_cast<Token>(i))) {
            return std::optional<std::string>("Arithmetic expression contains a bool");
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_trailing(const std::string_view infix_expression, const bool math) {
    const Token rbegin = static_cast<Token>(*infix_expression.rbegin());
    if (isnot(rbegin) && !math) {
        return std::optional<std::string>("Expression ends with NOT");
    } else if (math && is_math_operator(rbegin) && rbegin != Token::FAC) {
        return std::optional<std::string>("Math expression ends with an operator");
    } else if (!math && is_bool_operator(rbegin)) {
        return std::optional<std::string>("Boolean expression ends with an operator");
    } else if (rbegin == Token::LEFT_PAREN) {
        return std::optional<std::string>("Expression ends with open parentheses");
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_parentheses(const Token current_token, const Token previous_token) {
    if (current_token == Token::LEFT_PAREN && previous_token == Token::RIGHT_PAREN) {
        return std::optional<std::string>("Empty parentheses detected");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operators(const Token current_token, const Token previous_token) {
    if (is_bool_operator(current_token) && is_bool_operator(previous_token)) {
        return std::optional<std::string>("Two consecutive operators detected: " + std::string{static_cast<char>(current_token)} + " and " +
                std::string{static_cast<char>(previous_token)} + "");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operands(const Token current_token, const Token previous_token) {
    if (isoperand(current_token) && isoperand(previous_token)) {
        return std::optional<std::string>("Two consecutive operands detected: " + std::string{static_cast<char>(current_token)} + " and " +
                std::string{static_cast<char>(previous_token)} + "");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_not_after_value(const Token current_token, const Token previous_token) {
    if (isnot(current_token) && (isoperator(previous_token) || previous_token == Token::RIGHT_PAREN)) {
        return std::optional<std::string>("NOT applied after value");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operator_math(const Token current_token, const Token previous_token) {
    if ((current_token == Token::RIGHT_PAREN && is_math_operand(previous_token)) ||
        ((is_math_operand(current_token) && current_token != Token::UNARY) && previous_token == Token::LEFT_PAREN) || 
        (current_token == Token::RIGHT_PAREN && previous_token == Token::LEFT_PAREN)) {
        return std::optional<std::string>("Missing operator between " + std::string{static_cast<char>(current_token)}
                                  + " and " + std::string{static_cast<char>(previous_token)} + "");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operator_bool(const Token current_token, const Token previous_token) {
    if ((current_token == Token::RIGHT_PAREN && (isnot(previous_token) || is_bool_operand(previous_token))) ||
        (is_bool_operand(current_token) && (previous_token == Token::LEFT_PAREN || isnot(previous_token))) ||
        (current_token == Token::RIGHT_PAREN && previous_token == Token::LEFT_PAREN)) {
        return std::optional<std::string>("Missing operator between " + std::string{static_cast<char>(current_token)}
                                  + " and " + std::string{static_cast<char>(previous_token)} + "");

    }

    return std::nullopt;
}

// I was going to use a map, but that wouldn't be constexpr compatible
static inline constexpr std::initializer_list<Token> invalid_tokens_no_add_sub = {
    Token::MULT, 
    Token::DIV, 
    Token::POW, 
    Token::FAC, 
    Token::RIGHT_PAREN
};

static inline constexpr std::initializer_list<Token> invalid_tokens = {
    Token::ADD, 
    Token::SUB, 
    Token::MULT, 
    Token::DIV, 
    Token::POW, 
    Token::FAC, 
    Token::RIGHT_PAREN
};
static inline constexpr
std::array<std::pair<Token, std::initializer_list<Token> >, 7 > invalid_math_operator_sequences {
    std::make_pair(Token::ADD, invalid_tokens_no_add_sub),
    std::make_pair(Token::SUB, invalid_tokens_no_add_sub),
    std::make_pair(Token::MULT, invalid_tokens),
    std::make_pair(Token::DIV, invalid_tokens),
    std::make_pair(Token::POW, invalid_tokens_no_add_sub), 
    std::make_pair(Token::LEFT_PAREN, invalid_tokens_no_add_sub)
};

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operand_math(const Token current_token, const Token previous_token) {
    for (const auto& [token, prev_tokens] : invalid_math_operator_sequences) {
        if (current_token == token) {
            for (const auto c : prev_tokens) {
                if (previous_token == c) {
                    return std::optional<std::string>("Missing operand between " + std::string{static_cast<char>(current_token)}
                                                      + " and " + std::string{static_cast<char>(previous_token)} + "");
                }
            }
        }
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operand_bool(const Token current_token, const Token previous_token) {
    if ((current_token == Token::LEFT_PAREN && is_bool_operator(previous_token)) ||
        (is_bool_operator(current_token) && previous_token == Token::RIGHT_PAREN)) {
        return std::optional<std::string>("Missing operand between " + std::string{static_cast<char>(current_token)}
                                          + " and " + std::string{static_cast<char>(previous_token)} + "");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_for_factorial_error(const Token current_token, const Token previous_token) {
    if (current_token == Token::FAC && std::isdigit(static_cast<char>(previous_token))) {
        return std::optional<std::string>("Digit following factorial");
    } else if(!std::isdigit(static_cast<char>(current_token)) && current_token != Token::RIGHT_PAREN &&
               current_token != Token::FAC && previous_token == Token::FAC) {
        return std::optional<std::string>("Factorial follows a non-number value");
    } else if(current_token == Token::FAC && previous_token == Token::FAC) {
        return std::optional<std::string>("Consecutive factorials detected");
    }

    return std::nullopt;
}

}  // namespace

[[nodiscard]] inline
constexpr std::optional<std::string> initial_checks(const std::string_view infix_expression, const bool math) {
    const auto leading = check_leading(infix_expression, math);
    if (leading) return leading;
    
    const auto trailing = check_trailing(infix_expression, math);
    if (trailing) return trailing;

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> variable_error(const Token previous_token) {
    if (previous_token == Token::DOT) {
        return std::optional<std::string>("Decimal point detected after variable");
    } else if (std::isdigit(static_cast<char>(previous_token))) {
        return std::optional<std::string>("Digit detected after variable");
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> error_math(const Token current_token, const Token previous_token) {
    std::initializer_list<std::optional<std::string> (*)(const Token, const Token)> error_checks{
        check_missing_parentheses, check_missing_operator_math, check_missing_operand_math, 
        check_for_factorial_error};

    for (auto check : error_checks) {
        const auto result = check(current_token, previous_token);
        if (result) return result;
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> error_bool(const Token current_token, const Token previous_token) {
    std::initializer_list<std::optional<std::string> (*)(const Token, const Token)> error_checks{
        check_missing_parentheses, check_consecutive_operands, check_consecutive_operators,
        check_not_after_value,     check_missing_operator_bool,     check_missing_operand_bool};

    for (auto check : error_checks) {
        const auto result = check(current_token, previous_token);
        if (result) return result;
    }
    return std::nullopt;
}

[[nodiscard]] inline constexpr std::string invalid_character_error_math(const char token) {
    if (token == ']' || token == '[') {
        return "Invalid use of brackets detected! Just use parentheses please.";
    }
    return "Expected +, -, *, /, ^, received: " + std::string{token} + "";
}

[[nodiscard]] inline constexpr std::string invalid_character_error_bool(const char token) {
    if (isalnum(token)) {
        return "Expected T or F, received: " + std::string{token} + "";
    } else if (token == ']' || token == '[') {
        return "Invalid use of brackets detected! Just use parentheses please.";
    }
    return "Expected &, |, !, @, $, received: " + std::string{token} + "";
}

}  // namespace Error


#endif
