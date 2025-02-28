// Author: Caden LeCluyse

#include "file.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gmpxx.h>
#include <iostream>
#include <mpfr.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
//for _NSGetExecutablePath and PATH_MAX
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#endif

//for PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#endif

#include "ast/ast.h"
#include "parser/parser.h"
#include "util.hpp"


namespace File {

inline constexpr std::string_view ini_name = "settings.ini";


[[nodiscard]] static std::filesystem::path get_executable_path() {
    
char buffer[PATH_MAX];
#ifdef __linux__
    const ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    
    if (len == -1) {
        std::cerr << "Failed to get executable path\n";
        return "";
    }
    
    buffer[len] = '\0';
#elif defined(__APPLE__)
    std::uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) != 0) {
        std::cerr << "Failed to get executable path\n";
        return "";
    }
#else
    #error "Unsupported platform"
#endif

    const std::string_view path(buffer);
    const std::size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) {
        return std::filesystem::path(path.substr(0, last_slash));
    }
    
    return std::filesystem::path(path);
}

[[nodiscard]] static bool create_ini() {
    try {
        std::ofstream file(get_executable_path() / ini_name);
        if (file.is_open()) {
            file << "[Settings]\n";
            file << "precision=" << Util::default_precision << "\n";
            file << "display_digits=" << Util::default_digits << "\n";
            file.close();
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> source_ini() noexcept {
    std::vector<std::string> retval;
    retval.reserve(2);
    const auto ini_path = get_executable_path() / ini_name;
    const bool settings_exists = std::filesystem::exists(ini_path);
    if (!settings_exists) {
        const bool success = create_ini();
        if (!success) {
            std::cerr << "Unable to create settings.ini\n";
        }
        return Util::create_default_settings_vec();
    }
    std::ifstream input_file(ini_path);
    std::string line;
    while (std::getline(input_file, line)) {
        retval.emplace_back(std::move(line));
    }
    return retval;
}

void output_history(const std::vector<std::pair<std::string, std::string> >& history, 
                    std::ofstream& output_file) noexcept {
    std::ranges::for_each(history, [&output_file](const auto& expression_result) {
        const auto [expression, result] = expression_result;
        output_file << "Expression: " << expression << "\nResult: " << result << "\n";
    });
}

namespace {

std::vector<std::string> get_expressions() noexcept {
    std::vector<std::string> expressions;
    std::ifstream input_file("expressions.txt");
    std::string current_expression;

    if (input_file.is_open()) {
        while (std::getline(input_file, current_expression)) {
            expressions.emplace_back(std::move(current_expression));
        }
    } else {
        std::cout << "Couldn't find expressions.txt!\n";
    }

    return expressions;
}

void math_float_procedure(FILE*& output_file, const std::string_view result) {
    try {
        const auto tree = std::make_unique<MathAST>(result, true);
        const mpfr_t& final_value = tree->evaluate_floating_point();
        if (mpfr_integer_p(final_value)) {
            mpfr_fprintf(output_file, "Result: %.0Rf\n", final_value);
        } else {
            mpfr_fprintf(output_file, "Result: %.12Rf\n", final_value);
        }
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_int_procedure(FILE*& output_file, const std::string_view result) {
    try {
        const auto tree = std::make_unique<MathAST>(result, false);
        const mpz_class final_value = tree->evaluate();
        gmp_fprintf(output_file, "Result: %Zd\n", final_value.get_mpz_t());
    } catch (const std::bad_alloc& err) {
        fprintf(output_file, "Error: The number grew too big!\n"); 
    } catch (const std::exception& err) {
        fprintf(output_file, "Error: %s\n", err.what()); 
    }
}

void math_procedure(FILE*& output_file, const std::string_view result, const bool is_floating_point) {
    if (is_floating_point) {
        math_float_procedure(output_file, result);
    } else {
        math_int_procedure(output_file, result);
    }
}

void bool_procedure(FILE*& output_file, const std::string_view result) {
    const auto syntax_tree = std::make_unique<BoolAST>(result);
    if (syntax_tree->evaluate()) {
        fprintf(output_file, "Result: True!\n");
    } else {
        fprintf(output_file, "Result: False!\n");
    }
}

void main_loop(FILE*& output_file, std::string& expression) {
    const std::string orig_expression = expression;
    expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());
    const auto [result, success, is_math, is_floating_point] = Parse::create_prefix_expression(expression);

    fprintf(output_file, "Expression: %s\n", orig_expression.c_str());
    if (!success) {
        fprintf(output_file, "Error: %s", result.c_str());
        return;
    }
    if(is_math) {
        math_procedure(output_file, result, is_floating_point);
    } else {
        bool_procedure(output_file, result);
    }
}

}  // namespace

void initiate_file_mode() {
    std::vector<std::string> expressions = get_expressions();
    if (expressions.empty()) return;
    FILE* output_file;
    output_file = fopen("results.txt", "w");
    if(!output_file) [[unlikely]] {
        std::cerr << "Error opening file!\n";
        return;
    }
    for (auto& expression : expressions) {
        main_loop(output_file, expression);
    }
    fclose(output_file);
}

}  // namespace File
