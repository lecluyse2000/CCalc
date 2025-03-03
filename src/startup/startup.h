#ifndef STARTUP_H
#define STARTUP_H

#include <gmpxx.h>
#include <mpfr.h>
#include <string>
#include <unordered_map>

namespace Startup {

std::unordered_map<std::string, long> source_ini() noexcept;

}

#endif
