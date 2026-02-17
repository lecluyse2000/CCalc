#ifndef STARTUP_H
#define STARTUP_H

#include <gmpxx.h>
#include <mpfr.h>
#include <unordered_map>

#include "include/types.hpp"

namespace Startup {

extern const std::unordered_map<Types::Setting, long> settings;
[[nodiscard]] std::unordered_map<Types::Setting, long> source_ini() noexcept;

}

#endif
