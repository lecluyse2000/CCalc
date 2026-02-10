// Author: Caden LeCluyse

#ifndef UI_H
#define UI_H

#include <gmpxx.h>
#include <mpfr.h>
#include <span>
#include <string_view>

namespace UI {

void print_excessive_arguments(const int arguments);
void print_insufficient_arguments();
void print_help_continuous();
void print_result(const std::string_view);
std::string print_mpfr(const mpfr_t& final_value, const mpfr_prec_t display_precision);
void print_history(const std::span<const std::pair<std::string, std::string> > history);
void print_version();
void print_help();
void print_invalid_flag(const std::string_view expression);

}

#endif
