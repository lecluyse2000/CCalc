#include "startup.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <iostream>
#include <unordered_map>

#include "include/types.hpp"
#include "include/util.hpp"

namespace Startup {

[[maybe_unused]] inline constexpr std::string_view ini_path = ".config/ccalc/settings.ini";
[[maybe_unused]] inline constexpr std::string_view ini_path_windows = "AppData\\Local\\ccalc\\settings.ini";

namespace {

[[nodiscard]] std::filesystem::path get_home_path() {
    #ifdef _WIN32
        const char* const home_path = getenv("USERPROFILE");
    #else
        const char* const home_path = getenv("HOME");
    #endif

    if (!home_path) return "";
    return std::filesystem::path(home_path);
}

bool create_ini(const auto& full_path) {
    try {
        std::filesystem::create_directories(full_path.parent_path());
        std::ofstream file(full_path, std::ios::trunc);
        if (file.is_open()) {
            file << "[Settings]\n";
            file << "precision=" << Util::default_precision << '\n';
            file << "display_digits=" << Util::default_digits << '\n';
            file << "max_history=" << Util::default_history_max << '\n';
            file.close();
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

inline bool create_ini_return_false(const auto& full_path) {
    if (!create_ini(full_path)) {
        std::cerr << "Unable to create settings.ini\n";
    }
    return false;
}

[[nodiscard]] bool create_retval(const std::string& line, const auto& full_path, std::unordered_map<Types::Setting, long>& map) {
    const auto equal_pos = line.find('=');
    if (equal_pos == line.size() - 1 || equal_pos == std::string::npos) {
        create_ini_return_false(full_path);
    }

    const std::string_view key = std::string_view(line).substr(0, equal_pos);
    const std::string_view value_string = std::string_view(line).substr(equal_pos + 1);
    if(!std::ranges::all_of(value_string, ::isdigit)) {
        return create_ini_return_false(full_path);
    }
    const long value_long = std::stol(value_string.data());
    if (value_long < 0) {
        return create_ini_return_false(full_path);
    }
    const auto emplace_result = map.try_emplace(Types::string_to_settings_enum(key), value_long);
    if (!emplace_result.second) {
        return create_ini_return_false(full_path);
    }

    return true;
}

[[nodiscard]] bool final_verification(const auto& full_path, const std::unordered_map<Types::Setting, long>& map) {
    if (map.size() != Util::setting_keys.size()) {
        return create_ini_return_false(full_path);
    }
    if (map.contains(Types::Setting::INVALID)) {
        return create_ini_return_false(full_path);
    }

    return true;
}

}

std::unordered_map<Types::Setting, long> source_ini() noexcept {
    std::unordered_map<Types::Setting, long> retval;
    const std::filesystem::path parent_path = get_home_path();
    if (parent_path == "") return Util::create_default_settings_map();
    #ifdef _WIN32
        const std::filesystem::path full_path = get_home_path() / ini_path_windows;
    #else
        const std::filesystem::path full_path = get_home_path() / ini_path;
    #endif

    const bool settings_exists = std::filesystem::exists(full_path);
    if (!settings_exists) {
        if (!create_ini(full_path)) {
            std::cerr << "Unable to create settings.ini\n";
        }
        return Util::create_default_settings_map();
    }

    std::ifstream input_file(full_path);
    if (!input_file) [[unlikely]] {
        create_ini(full_path);
        return Util::create_default_settings_map();
    }
    std::string line;
    while (std::getline(input_file, line)) {
        if (line == "[Settings]") continue;
        if(!create_retval(line, full_path, retval)) return Util::create_default_settings_map();
    }
    if (!final_verification(full_path, retval)) return Util::create_default_settings_map();

    return retval;
}

}
