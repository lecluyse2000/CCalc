// Author: Caden LeCluyse

#ifndef ERROR_H
#define ERROR_H

#include <string_view>

namespace Error {

void initial_checks(const std::string_view infix_expression);
void error_checker(const char current_token, const char previous_token);
[[noreturn]] void throw_invalid_character_error(const char token);

}  // namespace Error

#endif
