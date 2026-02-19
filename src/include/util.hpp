#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <iostream>
#include <limits>
#include <mpfr.h>
#include <optional>
#include <string>

#include "ui/ui.h"

namespace Util {

inline constexpr std::size_t buffer_size = 128;

inline void clear_input_stream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Remove the trailing zeros from a MPFR float in string form
inline void trim_trailing_zero_mpfr(std::string& buffer) {
    // If there is no decimal, return early
    const auto find_decimal = std::ranges::find_if(buffer, [](const char c) {
        return c == '.';
    });
    if (find_decimal == buffer.end()) return;

    const auto last_non_zero = std::ranges::find_if(buffer.rbegin(), buffer.rend(), [](const char c) {
        return c != '0' && c != '.';
    });

    if (last_non_zero != buffer.rend()) {
        buffer.erase(last_non_zero.base(), buffer.end());
    } else {
        buffer.erase(find_decimal, buffer.end());
    }
    if (buffer.size() == 1 && buffer[0] == '-') buffer[0] = '0'; // Handle negative 0 case
}

// Convert an MPFR to a string 
[[nodiscard]]
inline bool convert_mpfr_char_vec(std::string& out, const mpfr_t& val, const mpfr_prec_t display_precision) {
    // If the float is an integer, don't worry about the precision
    const int snprintf_result = mpfr_integer_p(val) ? mpfr_snprintf(out.data(), buffer_size, "%.0Rf", val)
                                                    : mpfr_snprintf(out.data(), buffer_size, "%.*Rf", display_precision, val);
    if (snprintf_result < 0) [[unlikely]] {
        UI::print_error("mpfr_snprintf failure");
        return false;
    } else if (static_cast<std::size_t>(snprintf_result) >= buffer_size) {
        UI::print_error("buffer too small for mpfr_snprintf, output may be truncated.\n");
    }
    trim_trailing_zero_mpfr(out);
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
            UI::print_error("Unable to receive input! Aborting...\n");
            return std::nullopt;
        }

        // Verify the input
        std::cout << "You entered " << filename << " is this correct? (Y/N): ";
        if (!std::cin.get(filename_flag)) [[unlikely]] {
            UI::print_error("Unable to receive input! Aborting...\n");
            return std::nullopt;
        }

        while (toupper(filename_flag) != 'Y' && toupper(filename_flag) != 'N') {
            clear_input_stream();
            std::cout << "Incorrect input. Try again (Y/N): ";
            if (!std::cin.get(filename_flag)) [[unlikely]] {
                UI::print_error("Unable to receive input! Aborting...\n");
                return std::nullopt;
            }
        }
        clear_input_stream();
    }

    return std::optional<std::string>(filename);
}

}

#endif
