// Author: Caden LeCluyse

#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace Types {

inline constexpr std::string_view euler = 
"2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274274663919320030599218174135966290435";

enum struct Token : char {
    NULLCHAR = '\0',
    ADD = '+',
    SUB = '-',
    MULT = '*',
    DIV = '/',
    POW_XOR = '^',
    UNARY = '~',
    FAC = '!',
    ONE = '1',
    TWO = '2',
    THREE = '3',
    FOUR = '4',
    FIVE = '5',
    SIX = '6',
    SEVEN = '7',
    EIGHT = '8',
    NINE = '9',
    ZERO = '0',
    LEFT_PAREN = '(',
    RIGHT_PAREN = ')',
    DOT = '.',
    COMMA = ',',
    MODULO = '%',
    EQUAL = '=',
    SIN = 'S',
    COS = 'C',
    TAN = 'G', //tanGent, T is already used for true
    LOG = 'L',
    LN = 'N', 
    EULER = 'E',
    PI = 'P', 
    PI_2 = 'I',
    ANS = 'A',
    AND = '&',
    OR = '|',
    NAND = '@',
    NOR = '$',
    TRUE = 'T',
    FALSE = 'F'
};

inline constexpr bool is_valid_math_token(const char c) {
    switch (c) {
        case '\0':
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '~':
        case '!':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case '(':
        case ')':
        case '.':
        case ',':
        case '%':
        case '=':
        case 'S':
        case 'C':
        case 'G':
        case 'L':
        case 'N':
        case 'E':
        case 'P':
        case 'I':
        case 'A':
            return true;
        default:
            return false;
    }
}

inline constexpr bool is_valid_bool_token(const char c) {
    switch (c) {
        case '!':
        case '&':
        case '|':
        case '^':
        case '@':
        case '$':
        case 'T':
        case 'F':
            return true;
        default:
            return false;
    }
}

struct ParseResult {
    std::vector<Types::Token> result;
    std::string error_msg;
    bool success = false;
    bool is_math = false;
    bool is_floating_point = false;
};

enum struct Setting {
    PRECISION,
    DISPLAY_PREC,
    MAX_HISTORY,
    ANGLE,
    INVALID
};

[[nodiscard]] inline constexpr Setting string_to_settings_enum(const std::string_view string) {
    if (string == "precision") return Setting::PRECISION;
    if (string == "display_digits") return Setting::DISPLAY_PREC;
    if (string == "max_history") return Setting::MAX_HISTORY;
    if (string == "angle") return Setting::ANGLE;
    return Setting::INVALID;
}

[[nodiscard]] inline constexpr bool is_math_var(const Token token) noexcept {
    switch (token) {
        case Token::EULER:
        case Token::PI:
        case Token::PI_2: 
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool is_trig(const char token) noexcept {
    switch (token) {
        case 'S':
        case 'C':
        case 'T': 
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool is_trig(const Token token) noexcept {
    switch (token) {
        case Token::SIN:
        case Token::COS:
        case Token::TAN: 
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool is_math_operand(const Token token) noexcept {
    return std::isdigit(static_cast<char>(token)) || token == Token::DOT ||
           is_math_var(token);
}

[[nodiscard]] inline constexpr bool is_bool_operand(const Token token) noexcept {
    return token == Token::TRUE || token == Token::FALSE;
}

[[nodiscard]] inline constexpr bool isoperand(const Token token) noexcept {
    return is_math_operand(token) || is_bool_operand(token);
}

[[nodiscard]] inline constexpr bool is_math_operator(const Token token) noexcept {
    switch (token) {
        case Token::ADD:
        case Token::SUB:
        case Token::MULT:
        case Token::DIV:
        case Token::UNARY:
        case Token::POW_XOR:
        case Token::FAC:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool is_bool_operator(const Token token) noexcept {
    switch (token) {
        case Token::AND:
        case Token::OR:
        case Token::NAND:
        case Token::NOR:
        case Token::POW_XOR:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool isoperator(const Token token) noexcept {
    return is_math_operator(token) || is_bool_operator(token);
}

[[nodiscard]] inline constexpr bool isnot(const Token token) noexcept { return token == Token::FAC; }

[[nodiscard]] inline constexpr int get_precedence(const Token op) {
    switch (op) {
        case Token::ADD:
        case Token::SUB:
            return 1;
        case Token::MULT:
        case Token::DIV:
            return 2;
        case Token::UNARY:
            return 3;
        case Token::POW_XOR:
            return 4;
        case Token::FAC:
            return 5;
        default:
            return 0;
    }
}

}  // namespace Types

#endif
