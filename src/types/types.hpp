// Author: Caden LeCluyse

#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>

namespace Types {

[[nodiscard]] constexpr bool isoperand(const char token) noexcept {
    return (toupper(token) == 'T' || toupper(token) == 'F');
}

[[nodiscard]] constexpr bool isoperator(const char token) noexcept {
    return (token == '&' || token == '|' || token == '@' || token == '$');
}

[[nodiscard]] constexpr bool isnot(const char token) noexcept { return token == '!'; }

}  // namespace Types

#endif
