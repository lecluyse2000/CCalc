// Author: Caden LeCluyse

#include <cctype>
#include <mpfr.h>
#include <string>
#include <utility>
#include <vector>

#include "file/file.h"
#include "ui/ui.h"

std::pair<mpfr_prec_t, mp_prec_t> get_settings() {
    std::vector<std::string> settings = File::source_ini();
}

int main(const int argc, const char* const argv[]) {
    if (argc > 2) {
        UI::print_excessive_arguments(argc - 1);
        return 1;
    } else if (argc == 1) {
        UI::print_insufficient_arguments();
        return 1;
    }

    std::string expression = argv[1];
    if (expression == "-c" || expression == "--continuous") {
        return UI::program_loop();
    } else if (expression == "-v" || expression == "--version") {
        UI::print_version();
        return 0;
    } else if (expression == "-f" || expression == "--file") {
        File::initiate_file_mode();
        return 0;
    } else if (expression == "-h" || expression == "--help") {
        UI::print_help();
        return 0;
    } else if (expression[0] == '-' && (expression.size() == 1 ||
                                       (expression.size() >= 2 && (!std::isdigit(expression[1]) && expression[1] != '(')))) {
        UI::print_invalid_flag(expression);
        return 1;
    }

    UI::evaluate_expression(expression);
    return 0;
}
