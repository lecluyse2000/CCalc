#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <mpfr.h>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "include/types.hpp"

namespace Util {

inline constexpr std::size_t buffer_size = 256;

inline constexpr std::size_t num_settings = 4;
inline constexpr std::array<Types::Setting, num_settings> setting_keys = {
    Types::Setting::PRECISION,
    Types::Setting::DISPLAY_PREC,
    Types::Setting::MAX_HISTORY,
    Types::Setting::ANGLE
};
inline constexpr long default_precision = 320;
inline constexpr long default_digits = 15;
inline constexpr long default_history_max = 50;
inline constexpr long default_angle = 0; // 0 is radians, 1 is degrees
inline constexpr std::array<std::string_view, num_settings> setting_fields = {
    "precision=",
    "display_digits=",
    "max_history=",
    "angle="
};
inline constexpr std::array<long, num_settings> default_setting_values = {
    default_precision,
    default_digits,
    default_history_max,
    default_angle
};

inline std::unordered_map<Types::Setting, long> create_default_settings_map() {
    std::unordered_map<Types::Setting, long> retval;
    for (std::size_t i = 0; i < num_settings; ++i) {
        retval.emplace(setting_keys[i], default_setting_values[i]);
    }
    return retval;
}

inline void clear_input_stream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

inline void empty_stack(std::stack<Types::Token>& operator_stack) noexcept {
    while (!operator_stack.empty()) {
        operator_stack.pop();
    }
}

// Remove the trailing zeros from a MPFR float in char vector form
inline void trim_trailing_zero_mpfr(std::vector<char>& buffer) {
    // If there is no decimal, return early
    const auto find_decimal = std::ranges::find_if(buffer, [](const char c) {
        return c == '.';
    });
    if (find_decimal == buffer.end()) return;
    const auto last_non_zero = std::find_if(buffer.rbegin(), buffer.rend(), [](const char c) {
        return c != '0' && c != '.';
    });
    if (last_non_zero != buffer.rend()) {
        buffer.erase(last_non_zero.base(), buffer.end());
    } 
    if (buffer.size() == 1 && buffer[0] == '-') buffer[0] = '0'; // Handle negative 0 case
}

// Convert an MPFR to a char vector
[[nodiscard]]
inline bool convert_mpfr_char_vec(std::vector<char>& buffer, const mpfr_t& val, const mpfr_prec_t display_precision) {
    // If the float is an integer, don't worry about the precision
    const int snprintf_result = mpfr_integer_p(val) ? mpfr_snprintf(buffer.data(), buffer_size, "%.0Rf", val)
                                                    : mpfr_snprintf(buffer.data(), buffer_size, "%.*Rf", display_precision, val);
    if (snprintf_result < 0) [[unlikely]] {
        std::cerr << "Error in mpfr_snprintf!\n";
        return false;
    } else if (static_cast<std::size_t>(snprintf_result) >= buffer_size) {
        std::cerr << "Warning: Buffer too small for mpfr_snprintf, output may be truncated.\n";
    }
    buffer.resize(static_cast<std::size_t>(snprintf_result));
    trim_trailing_zero_mpfr(buffer);
    return true; 
}

// This function grabs the filename the user wants to print to
// write indicates if we are reading or writing
[[nodiscard]] inline std::optional<std::string> get_filename(const bool write) {
    std::string filename;
    char filename_flag = 'f';

    while (toupper(filename_flag) != 'Y') {
        if (write) {
            std::cout << "What is the name of the file you would like to save to? ";
        } else {
            std::cout << "What is the name of the file you would like to open? ";
        }
        if (!std::getline(std::cin, filename)) [[unlikely]] {
            std::cerr << "Unable to receive input! Aborting...\n\n";
            return std::nullopt;
        }

        // Verify the input
        std::cout << "You entered " << filename << " is this correct? (Y/N): ";
        if (!std::cin.get(filename_flag)) [[unlikely]] {
            std::cerr << "Unable to receive input! Aborting...\n\n";
            return std::nullopt;
        }

        while (toupper(filename_flag) != 'Y' && toupper(filename_flag) != 'N') {
            clear_input_stream();
            std::cout << "Incorrect input. Try again (Y/N): ";
            if (!std::cin.get(filename_flag)) [[unlikely]] {
                std::cerr << "Unable to receive input! Aborting...\n\n";
                return std::nullopt;
            }
        }
        clear_input_stream();
    }

    return std::optional<std::string>(filename);
}

}

#endif
