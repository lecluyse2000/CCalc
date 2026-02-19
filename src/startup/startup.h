#ifndef STARTUP_H
#define STARTUP_H

#include <gmpxx.h>
#include <mpfr.h>
#include <unordered_map>

#include "include/types.hpp"

namespace Startup {

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
extern const std::unordered_map<Types::Setting, long> settings;
[[nodiscard]] std::unordered_map<Types::Setting, long> source_ini() noexcept;

}

#endif
