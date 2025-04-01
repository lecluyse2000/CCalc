#include "startup.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <iostream>
#include <unordered_map>

#include "include/types.hpp"
#include "include/util.hpp"

using namespace Types;

namespace Startup {

namespace {

#ifdef _WIN32
    inline constexpr std::string_view ini_path = "AppData\\Local\\ccalc\\settings.ini";
#else
    inline constexpr std::string_view ini_path = ".config/ccalc/settings.ini";
#endif

[[nodiscard]] std::filesystem::path get_home_path() {
    #ifdef _WIN32
        const char* const home_path = getenv("USERPROFILE");
    #else
        const char* const home_path = getenv("HOME");
    #endif

    if (!home_path) return "";
    return std::filesystem::path(home_path);
}

bool create_ini(const std::filesystem::path& full_path) {
    try {
        std::filesystem::create_directories(full_path.parent_path());
        std::ofstream file(full_path, std::ios::trunc);
        if (file.is_open()) [[likely]] {
            file << "[Settings]\n";
            for (std::size_t i = 0; i < Util::num_settings; ++i) {
                if (i == Util::num_settings - 1) file << "# angle=0 (radians) or angle=1 (degrees)\n";
                file << Util::setting_fields[i] << Util::default_setting_values[i] << '\n'; 
            }
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

[[nodiscard]] inline bool create_ini_return_false(const std::filesystem::path& full_path) {
    if (!create_ini(full_path)) [[unlikely]] {
        std::cerr << "Unable to create settings.ini\n";
    }
    return false;
}

[[nodiscard]] bool create_retval(const std::string& line, const std::filesystem::path& full_path,
                                 std::unordered_map<Setting, long>& map) {
    const auto equal_pos = line.find('=');
    if (equal_pos == line.size() - 1 || equal_pos == std::string::npos) {
        return create_ini_return_false(full_path);
    }

    const std::string_view key = std::string_view(line).substr(0, equal_pos);
    const std::string_view value_string = std::string_view(line).substr(equal_pos + 1);
    if (key == "angle" && value_string != "0" && value_string != "1") {
        return create_ini_return_false(full_path);
    }
    if(!std::ranges::all_of(value_string, ::isdigit)) [[unlikely]] {
        return create_ini_return_false(full_path);
    }
    const long value_long = std::stol(value_string.data());
    if (value_long < 0) [[unlikely]] {
        return create_ini_return_false(full_path);
    }
    const auto emplace_result = map.try_emplace(string_to_settings_enum(key), value_long);
    if (!emplace_result.second) {
        return create_ini_return_false(full_path);
    }

    return true;
}

[[nodiscard]] bool final_verification(const std::filesystem::path& full_path,
                                      const std::unordered_map<Setting, long>& map) {
    if (map.size() != Util::setting_keys.size()) {
        return create_ini_return_false(full_path);
    }
    if (map.contains(Setting::INVALID)) {
        return create_ini_return_false(full_path);
    }
    return true;
}

}

[[nodiscard]] std::unordered_map<Setting, long> source_ini() noexcept {
    std::unordered_map<Setting, long> retval;
    const std::filesystem::path parent_path = get_home_path();
    if (parent_path.empty()) return Util::create_default_settings_map();
    const std::filesystem::path full_path = get_home_path() / ini_path;

    const bool settings_exists = std::filesystem::exists(full_path);
    if (!settings_exists) {
        if (!create_ini(full_path)) [[unlikely]] {
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
        if (line[0] == '#') continue;
        if(!create_retval(line, full_path, retval)) return Util::create_default_settings_map();
    }
    if (!final_verification(full_path, retval)) return Util::create_default_settings_map();

    return retval;
}

}
