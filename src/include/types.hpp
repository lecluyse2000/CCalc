// Author: Caden LeCluyse

#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>
#include <string_view>

namespace Types {

enum struct Setting {
    PRECISION,
    DISPLAY_PREC,
    INVALID
};

[[nodiscard]] inline constexpr Setting string_to_settings_enum(const std::string_view string) {
    if (string == "precision") return Setting::PRECISION;
    if (string == "display_digits") return Setting::DISPLAY_PREC;
    return Setting::INVALID;
}

[[nodiscard]] inline constexpr bool is_math_operand(const char token) noexcept {
    return std::isdigit(token) || token == '.';
}

[[nodiscard]] inline constexpr bool is_bool_operand(const char token) noexcept {
    return toupper(token) == 'T' || toupper(token) == 'F';
}

[[nodiscard]] inline constexpr bool isoperand(const char token) noexcept {
    return is_math_operand(token) || is_bool_operand(token);
}

[[nodiscard]] inline constexpr bool is_math_operator(const char token) noexcept {
    switch (token) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '~':
        case '!':
            return true;
    }
    return false;
}

[[nodiscard]] inline constexpr bool is_bool_operator(const char token) noexcept {
    return token == '&' || token == '|' || token == '@' || token == '$';
}

[[nodiscard]] inline constexpr bool isoperator(const char token) noexcept {
    return is_math_operator(token) || is_bool_operator(token);
}

[[nodiscard]] inline constexpr bool isnot(const char token) noexcept { return token == '!'; }

[[nodiscard]] inline constexpr int get_precedence(const char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '~':
            return 3;
        case '^':
            return 4;
        case '!':
            return 5;
        default:
            return 0;
    }
}

}  // namespace Types

#endif
