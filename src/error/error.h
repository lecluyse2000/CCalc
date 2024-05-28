// Author: Caden LeCluyse

#ifndef ERROR_H
#define ERROR_H

#include <optional>
#include <string_view>

namespace Error {

std::optional<std::string> initial_checks(const std::string_view infix_expression);
std::optional<std::string> error_checker(const char current_token, const char previous_token);
std::string invalid_character_error(const char token);

}  // namespace Error

#endif
