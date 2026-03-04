#include "startup/startup.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <readline/readline.h>
#include <string>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <unordered_map>

#include "engine/signal.h"
#include "file/file.h"
#include "include/types.hpp"
#include "ui/ui.h"

using namespace Types;

namespace Startup {

namespace {

[[nodiscard]] std::filesystem::path get_home_path() {
    const char* const home_path = getenv("HOME");

    if (!home_path) return "";
    return std::filesystem::path(home_path);
}

bool create_ini(const std::filesystem::path& full_path) {
    try {
        std::filesystem::create_directories(full_path.parent_path());
        std::ofstream file(full_path, std::ios::trunc);
        if (file.is_open()) [[likely]] {
            file << "[Settings]\n";
            for (std::size_t i = 0; i < num_settings; ++i) {
                // Comment for angle setting
                if (setting_fields[i] == "angle=") file << "# angle=0 (radians) or angle=1 (degrees)\n";
                file << setting_fields[i] << default_setting_values[i] << '\n'; 
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
        UI::print_error("Unable to create settings.ini");
    }
    return false;
}

// Create the setting map based on the parsed line from settings.ini
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
    if (!emplace_result.second) { // If try_emplace is not successful, create the default map
        return create_ini_return_false(full_path);
    }

    return true;
}

// Verify there is no invalid setting in the map
[[nodiscard]] bool final_verification(const std::filesystem::path& full_path,
                                      const std::unordered_map<Setting, long>& map) {
    if (map.size() != setting_keys.size()) {
        return create_ini_return_false(full_path);
    }
    if (map.contains(Setting::INVALID)) {
        return create_ini_return_false(full_path);
    }
    return true;
}

inline std::unordered_map<Types::Setting, long> create_default_settings_map() {
    std::unordered_map<Types::Setting, long> retval;
    for (std::size_t i = 0; i < num_settings; ++i) {
        retval.emplace(setting_keys[i], default_setting_values[i]);
    }
    return retval;
}

[[nodiscard]] std::filesystem::path get_history_location() {
    const std::string home = get_home_path();
    if (home.empty()) return std::string(Types::history_file_name);
    return home / std::filesystem::path(Types::history_file_name);
}

}

[[nodiscard]] std::unordered_map<Setting, long> source_ini() noexcept {
    static constexpr std::string_view ini_path = ".config/ccalc/settings.ini";
    std::unordered_map<Setting, long> retval;
    const std::filesystem::path parent_path = get_home_path();
    if (parent_path.empty()) return create_default_settings_map();
    const std::filesystem::path full_path = get_home_path() / ini_path;

    if (!std::filesystem::exists(full_path)) {
        if (!create_ini(full_path)) [[unlikely]] {
            UI::print_error("Unable to create settings.ini\n");
        }
        return create_default_settings_map();
    }

    std::ifstream input_file(full_path);
    if (!input_file) [[unlikely]] {
        create_ini(full_path);
        return create_default_settings_map();
    }
    std::string line;
    while (std::getline(input_file, line)) {
        if (line == "[Settings]") continue;
        if (line[0] == '#') continue; // Ignore comments
        if(!create_retval(line, full_path, retval)) return create_default_settings_map();
    }
    if (!final_verification(full_path, retval)) return create_default_settings_map();

    return retval;
}

const std::unordered_map<Types::Setting, long> settings = source_ini();
const std::string history_location = get_history_location();

void startup(std::vector<std::pair<std::string, std::string> >& history) {
    using_history();
    stifle_history(static_cast<int>(Startup::settings.at(Setting::MAX_HISTORY)));
    rl_event_hook = Signal::check_signals_hook;
    rl_catch_signals = 0;
    std::ifstream history_file;
    history_file.open(Startup::history_location);
    if(history_file.is_open()) File::read_history(history, history_file);
    Signal::register_handlers();
}

}
