// Author: Caden LeCluyse

#ifndef ERROR_HPP
#define ERROR_HPP

#include <cctype>
#include <optional>
#include <string>
#include <string_view>

#include "types.hpp"

namespace Error {

// Declare helper functions in an anonymous namespace
namespace {

[[nodiscard]] 
constexpr std::optional<std::string> check_leading(const std::string_view infix_expression, const bool math) {
    if(Types::isoperator(infix_expression[0]) && infix_expression[0] != '-' && infix_expression[0] != '~') {
        return std::optional<std::string>("Expression begins with an operator!\n");
    }
    if (infix_expression[0] == ')') {
        return std::optional<std::string>("Expression begins with closed parentheses!\n");
    } else if (infix_expression[0] == '!' && math) {
        return std::optional<std::string>("Expression begins with factorial!\n");
    }
    for (const auto i : infix_expression) {
        if (!math && std::isdigit(i)) {
            return std::optional<std::string>("Boolean expression contains a number!\n");
        } else if (math && Types::is_bool_operand(i)) {
            return std::optional<std::string>("Arithmetic expression contains a bool!\n");
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_trailing(const std::string_view infix_expression, const bool math) {
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        if (Types::isnot(*itr) && !math) {
            return std::optional<std::string>("Expression ends with NOT!\n");
        } else if (Types::isoperator(*itr) && (!math && *itr != '!')) {
            return std::optional<std::string>("Expression ends with an operator!\n");
        } else if (*itr == '(') {
            return std::optional<std::string>("Expression ends with open parentheses!\n");
        } else {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_parentheses(const char current_token, const char previous_token) {
    if (current_token == '(' && previous_token == ')') {
        return std::optional<std::string>("Empty parentheses detected!\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operands(const char current_token, const char previous_token) {
    if (Types::isoperator(current_token) && Types::isoperator(previous_token)) {
        return std::optional<std::string>("Two consecutive operators detected: " + std::string(1, current_token) + " and " +
                std::string(1, previous_token) + "\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operators(const char current_token, const char previous_token) {
    if (Types::isoperand(current_token) && Types::isoperand(previous_token)) {
        return std::optional<std::string>("Two consecutive operands detected: " + std::string(1, current_token) + " and " +
                std::string(1, previous_token) + "\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_not_after_value(const char current_token, const char previous_token) {
    if (Types::isnot(current_token) && (Types::isoperator(previous_token) || previous_token == ')')) {
        return std::optional<std::string>("NOT applied after value!\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operator_math(const char current_token, const char previous_token) {
    if ((current_token == ')' && Types::isoperand(previous_token)) ||
        ((Types::isoperand(current_token) && current_token != '~') && previous_token == '(') || 
        (current_token == ')' && previous_token == '(')) {
        return std::optional<std::string>("Missing operator!\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operator_bool(const char current_token, const char previous_token) {
    if ((current_token == ')' && (Types::isnot(previous_token) || Types::isoperand(previous_token))) ||
        (Types::isoperand(current_token) && (previous_token == '(' || Types::isnot(previous_token))) ||
        (current_token == ')' && previous_token == '(')) {
        return std::optional<std::string>("Missing operator!\n");
    }

    return std::nullopt;
}

// This function is pretty bad but I don't feel like refactoring
[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operand_math(const char current_token, const char previous_token) {
    if ((current_token == '+' && previous_token =='*') || (current_token == '*' && previous_token == '*') ||
         (current_token == '-' && previous_token == '/') || (Types::is_math_operator(current_token) && current_token != '!' && previous_token == ')') ||
         (current_token == '*' && previous_token == '/') || (current_token == '/' && previous_token == '*') ||
         (current_token == '(' && (Types::is_math_operator(previous_token) && previous_token != '+' && previous_token != '-' && previous_token != '~')) ||
         (current_token == '/' && previous_token == '/') || (current_token == '^' && previous_token == '^')){
        return std::optional<std::string>("Missing operand!\n");
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_operand_bool(const char current_token, const char previous_token) {
    if ((current_token == '(' && Types::is_bool_operator(previous_token)) ||
        (Types::is_bool_operator(current_token) && previous_token == ')')) {
        return std::optional<std::string>("Missing an operand!\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_for_factorial_error(const char current_token, const char previous_token) {
    if (current_token == '!' && std::isdigit(previous_token)) {
        return std::optional<std::string>("Digit following factorial!\n");
    } else if(!std::isdigit(current_token) && current_token != ')' && previous_token == '!') {
        return std::optional<std::string>("Factorial follows a non-number value!\n");
    } else if(current_token == '!' && previous_token == '!') {
        return std::optional<std::string>("Consecutive factorials detected!\n");
    }

    return std::nullopt;
}

}  // namespace

[[nodiscard]] inline
constexpr std::optional<std::string> initial_checks(const std::string_view infix_expression, const bool math) {
    const auto leading = check_leading(infix_expression, math);
    if (leading) return std::optional<std::string>(*leading);
    
    const auto trailing = check_trailing(infix_expression, math);
    if (trailing) return std::optional<std::string>(*trailing);

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> error_math(const char current_token, const char previous_token) {
    std::initializer_list<std::optional<std::string> (*)(const char, const char)> error_checks{
        check_missing_parentheses, check_missing_operator_math, check_missing_operand_math, 
        check_for_factorial_error};

    for (auto check : error_checks) {
        const auto result = check(current_token, previous_token);
        if (result) return result;
    }
    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> error_bool(const char current_token, const char previous_token) {
    std::initializer_list<std::optional<std::string> (*)(const char, const char)> error_checks{
        check_missing_parentheses, check_consecutive_operands, check_consecutive_operators,
        check_not_after_value,     check_missing_operator_bool,     check_missing_operand_bool};

    for (auto check : error_checks) {
        const auto result = check(current_token, previous_token);
        if (result) return result;
    }
    return std::nullopt;
}

[[nodiscard]] inline constexpr std::string invalid_character_error_math(const char token) {
    if (isalnum(token)) {
        return "Expected a number, received: " + std::string(1, token) + "\n";
    } else if (token == ']' || token == '[') {
        return "Invalid use of brackets detected! Just use parentheses please.\n";
    }
    return "Expected +, -, *, /, ^, received: " + std::string(1, token) + "\n";
}

[[nodiscard]] inline constexpr std::string invalid_character_error_bool(const char token) {
    if (isalnum(token)) {
        return "Expected T or F, received: " + std::string(1, token) + "\n";
    } else if (token == ']' || token == '[') {
        return "Invalid use of brackets detected! Just use parentheses please.\n";
    }
    return "Expected &, |, !, @, $, received: " + std::string(1, token) + "\n";
}

}  // namespace Error


#endif
