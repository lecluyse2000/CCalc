#ifndef UTIL_HPP
#define UTIL_HPP

#include <array>
#include <mpfr.h>
#include <iostream>
#include <limits>
#include <stack>
#include <unordered_map>

#include "include/types.hpp"

namespace Util {

inline constexpr std::array<Types::Setting, 2> setting_keys = {Types::Setting::PRECISION, Types::Setting::DISPLAY_PREC};
inline constexpr long default_precision = 320;
inline constexpr long default_digits = 20;

inline std::unordered_map<Types::Setting, long> create_default_settings_map() {
    return { {setting_keys[0], default_precision},
             {setting_keys[1], default_digits} };
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
