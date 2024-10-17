// Author: Caden LeCluyse

#ifndef ERROR_HPP
#define ERROR_HPP

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>

#include "../types/types.hpp"

namespace Error {

// Declare helper functions in an anonymous namespace
namespace {

[[nodiscard]] inline constexpr std::optional<std::string> check_leading(const std::string_view infix_expression) {
    for (const auto i : infix_expression) {
        if (isspace(i)) {
            continue;
        } else if (Types::isoperator(i)) {
            return ("Expression begins with an operator!\n");
        } else if (i == ')') {
            return ("Expression begins with closed parentheses!\n");
        } else {
            break;
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline constexpr std::optional<std::string> check_trailing(const std::string_view infix_expression) {
    // I originally was just checking for the last value, but then I realized white space messed it all up
    for (auto itr = infix_expression.rbegin(); itr != infix_expression.rend(); ++itr) {
        if (isspace(*itr)) {
            continue;
        } else if (Types::isnot(*itr)) {
            return ("Expression ends with NOT!\n");
        } else if (Types::isoperator(*itr)) {
            return ("Expression ends with an operator!\n");
        } else if (*itr == '(') {
            return ("Expression ends with open parentheses!\n");
        } else {
            break;
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_missing_parentheses(const char current_token, const char previous_token) {
    if (current_token == '(' && previous_token == ')') {
        return ("Empty parentheses detected!\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operands(const char current_token, const char previous_token) {
    if (Types::isoperator(current_token) && Types::isoperator(previous_token)) {
        return ("Two consecutive operators detected: " + std::string(1, current_token) + " and " +
                std::string(1, previous_token) + "\n");
    }

    return std::nullopt;
}

[[nodiscard]] inline
constexpr std::optional<std::string> check_consecutive_operators(const char current_token, const char previous_token) {
    if (Types::isoperand(current_token) && Types::isoperand(previous_token)) {
        return ("Two consecutive operands detected: " + std::string(1, current_token) + " and " +
                std::string(1, previous_token) + "\n");
    }

    return std::nullopt;
}

[[nodiscard]]
inline constexpr std::optional<std::string> check_not_after_value(const char current_token, const char previous_token) {
    if (Types::isnot(current_token) && (Types::isoperator(previous_token) || previous_token == ')')) {
        return ("NOT applied after value!\n");
    }

    return std::nullopt;
}

[[nodiscard]]
inline constexpr std::optional<std::string> check_missing_operator(const char current_token, const char previous_token) {
    if ((current_token == ')' && (Types::isnot(previous_token) || Types::isoperand(previous_token))) ||
        (Types::isoperand(current_token) && (previous_token == '(' || Types::isnot(previous_token))) ||
        (current_token == ')' && previous_token == '(')) {
        return ("Missing operator!\n");
    }

    return std::nullopt;
}

[[nodiscard]]
inline constexpr std::optional<std::string> check_missing_operand(const char current_token, const char previous_token) {
    if ((current_token == '(' && Types::isoperator(previous_token)) ||
        (Types::isoperator(current_token) && previous_token == ')')) {
        return ("Missing an operand!\n");
    }

    return std::nullopt;
}

}  // namespace

[[nodiscard]] inline constexpr std::optional<std::string> initial_checks(const std::string_view infix_expression) {
    if (std::ranges::all_of(infix_expression, isspace)) {
        return ("Expression contains only spaces!\n");
    }
    const auto leading = check_leading(infix_expression);
    if (leading) {
        return leading;
    }
    const auto trailing = check_trailing(infix_expression);
    if (trailing) {
        return trailing;
    }

    return std::nullopt;
}

[[nodiscard]] inline constexpr std::optional<std::string> error_checker(const char current_token, const char previous_token) {
    std::initializer_list<std::optional<std::string> (*)(const char, const char)> error_checks{
        check_missing_parentheses, check_consecutive_operands, check_consecutive_operators,
        check_not_after_value,     check_missing_operator,     check_missing_operand};

    for (auto check : error_checks) {
        const auto result = check(current_token, previous_token);
        if (result) {
            return result;
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline constexpr std::string invalid_character_error(const char token) {
    if (isalnum(token)) {
        return ("Expected T or F, received: " + std::string(1, token) + "\n");
    } else if (token == ']' || token == '[') {
        return ("Invalid use of brackets detected! Just use parentheses please.\n");
    }
    return ("Expected &, |, !, @, $, received: " + std::string(1, token) + "\n");
}

}  // namespace Error


#endif
