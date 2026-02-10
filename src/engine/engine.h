#ifndef ENGINE_H
#define ENGINE_H

#include <string>

namespace Engine {

enum struct InputResult {
    CONTINUE_TO_EVALUATE,
    CONTINUE,
    QUIT_FAILURE,
    QUIT_SUCCESS
};

[[nodiscard]] int start_engine(const int argc, const char* const argv[]);
void evaluate_expression(std::string& expression);

}

#endif
