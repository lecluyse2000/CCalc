#ifndef MATHPARSE_H
#define MATHPARSE_H

#include <optional>
#include <stack>
#include <string>

#include "include/types.hpp"

namespace MathParse {
    struct MathParseState {
        std::string num_buffer;
        std::string::reverse_iterator itr;
        char current_token = '\0';
        char previous_token = '\0';
        bool in_number = false;
    };

    inline void clear_num_buffer(MathParseState& state) noexcept {
        state.in_number = false;
        state.num_buffer.clear();
    }

    std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                          std::stack<char>& operator_stack);
}

#endif
