#ifndef UTIL_HPP
#define UTIL_HPP

#include <array>
#include <cmath>
#include <mpfr.h>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <unordered_map>

namespace Util {

inline constexpr std::array<std::string, 2> settings_keys = {"precision", "display_digits"};
inline constexpr long default_precision = 320;
inline constexpr long default_digits = 20;

inline std::unordered_map<std::string, long> create_default_settings_map() {
    return { {settings_keys[0], default_precision},
             {settings_keys[1], default_digits} };
}

inline void clear_input_stream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

inline void empty_stack(std::stack<char>& operator_stack) noexcept {
    while (!operator_stack.empty()) {
        operator_stack.pop();
    }
}

}

#endif
