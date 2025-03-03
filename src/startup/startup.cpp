#include "startup.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <unordered_map>

#include "include/util.hpp"

namespace Startup {

namespace {

inline constexpr std::string_view ini_path = ".config/ccalc/settings.ini";

[[nodiscard]] std::filesystem::path get_home_path() {
    const char* const home_path = getenv("HOME");
    if (!home_path) return "";
    return std::filesystem::path(home_path);
}

bool create_ini(const auto& full_path) {
    try {
        std::filesystem::create_directories(full_path.parent_path());
        std::ofstream file(full_path, std::ios::trunc);
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

[[nodiscard]] bool create_retval(const std::string& line, const auto& full_path, std::unordered_map<std::string, long>& map) {
    const auto equal_pos = line.find('=');
    if (equal_pos == line.size() - 1) {
        create_ini(full_path);
        return false;
    } else if (equal_pos == std::string::npos) {
        create_ini(full_path); 
        return false;
    }

    const std::string key = line.substr(0, equal_pos);
    const std::string_view value_string = std::string_view(line).substr(equal_pos + 1);
    if(!std::ranges::all_of(value_string, ::isdigit)) {
        create_ini(full_path); 
        return false;
    }
    const long value_long = std::stol(value_string.data());
    if (value_long < 0) {
        create_ini(full_path); 
        return false;
    }
    map.emplace(std::move(key), value_long);
    return true;
}

[[nodiscard]] bool final_verification(const auto& full_path, const std::unordered_map<std::string, long> map) {
    if (map.size() != 2) {
        create_ini(full_path);
        return false;
    }
    for (const auto& key : Util::settings_keys) {
        if (!map.contains(key)) {
            create_ini(full_path);
            return false;
        }
    }
    return true;
}

}

std::unordered_map<std::string, long> source_ini() noexcept {
    std::unordered_map<std::string, long> retval;
    const std::filesystem::path parent_path = get_home_path();
    if (parent_path == "") return Util::create_default_settings_map();
    const std::filesystem::path full_path = get_home_path() / ini_path;

    const bool settings_exists = std::filesystem::exists(full_path);
    if (!settings_exists) {
        const bool success = create_ini(full_path);
        if (!success) {
            std::cerr << "Unable to create settings.ini\n";
        }
        return Util::create_default_settings_map();
    }

    std::ifstream input_file(full_path);
    if (!input_file) [[unlikely]] return retval;
    std::string line;
    while (std::getline(input_file, line)) {
        if (line == "[Settings]") continue;
        if(!create_retval(line, full_path, retval)) return Util::create_default_settings_map();
    }
    if (!final_verification(full_path, retval)) return Util::create_default_settings_map();

    return retval;
}

}
