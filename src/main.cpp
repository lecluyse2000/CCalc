// Author: Caden LeCluyse

#include <string>

#include "file/file.h"
#include "ui/ui.h"

int main(const int argc, const char* const argv[]) {
    if (argc > 2) {
        UI::print_excessive_arguments(argc - 1);
        return 1;
    } else if (argc == 1) {
        UI::print_insufficient_arguments();
        return 1;
    }

    const std::string expression = argv[1];
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
    } else if (expression[0] == '-') {
        UI::print_invalid_flag(expression);
        return 1;
    }

    UI::evaluate_expression(expression);
    return 0;
}
