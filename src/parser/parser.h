// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include <string>
#include <string_view>

namespace Parse {
    struct MathParseState {
        std::string num_buffer;
        std::string::reverse_iterator itr;
        char current_token = '\0';
        char previous_token = '\0';
        bool in_number = false;
    };

    struct BoolParseState {
        std::string_view::reverse_iterator itr;
        char current_token = '\0';
        char previous_token = '\0';
    };

    struct ParseResult {
        std::string result;
        bool success = false;
        bool is_math = false;
        bool is_floating_point = false;
    };

    inline void empty_stack(std::stack<char>& operator_stack) noexcept {
        while (!operator_stack.empty()) {
            operator_stack.pop();
        }
    }
    
    inline void clear_num_buffer(MathParseState& state) noexcept {
        state.in_number = false;
        state.num_buffer.clear();
    }

    [[nodiscard]] ParseResult create_prefix_expression(std::string& infix_expression);
};

#endif
