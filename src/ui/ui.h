// Author: Caden LeCluyse

#ifndef UI_H
#define UI_H

#include <gmpxx.h>
#include <mpfr.h>
#include <readline/history.h>
#include <string>
#include <unordered_map>

namespace UI {

void print_excessive_arguments(const int arguments);
void print_insufficient_arguments();
void print_help_continuous();
void print_result(const std::string_view result);
void print_error(const std::string_view error);
std::string print_mpfr(const mpfr_t& final_value, const mpfr_prec_t display_precision);
void print_history(const std::unordered_map<HIST_ENTRY*, std::string>& history);
void print_version();
void print_help();
void print_invalid_flag(const std::string_view expression);

}

#endif
