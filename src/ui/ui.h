// Author: Caden LeCluyse

#ifndef UI_H
#define UI_H

#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace UI {

inline void clear_input_stream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print_excessive_arguments(const int arguments);
void print_insufficient_arguments();
[[nodiscard]] int program_loop(); 
void print_version();
void print_help();
void print_invalid_flag(const std::string_view expression);
void evaluate_expression(std::string& expression);

}

#endif
