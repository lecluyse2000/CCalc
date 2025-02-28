#ifndef UTIL_HPP
#define UTIL_HPP

#include <cmath>
#include <mpfr.h>
#include <iostream>
#include <limits>
#include <stack>
#include <string>
#include <vector>

namespace Util {

inline constexpr mpfr_prec_t default_precision = 256;
static const mp_prec_t default_digits = static_cast<mp_prec_t>(mpfr_get_str_ndigits(10, default_precision));

inline int bitsToDecimalDigits(const int bits) {
    return static_cast<int>(std::ceil(bits * std::log10(2.0)));
}

inline std::vector<std::string> create_default_settings_vec() {
    std::vector<std::string> retval;
    retval.emplace_back("precision=" + std::to_string(default_precision));
    retval.emplace_back("display_digits=" + std::to_string(default_digits));
    return retval;
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
