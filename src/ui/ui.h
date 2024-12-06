// Author: Caden LeCluyse

#ifndef UI_H
#define UI_H

#include <string_view>

namespace UI {

void print_excessive_arguments(const int arguments);
void print_insufficient_arguments();
[[nodiscard]] int program_loop(); 
void print_version();
void print_help();
void print_invalid_flag(const std::string_view expression);
void evaluate_expression(const std::string_view expression);

}

#endif
