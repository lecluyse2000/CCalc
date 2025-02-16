// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include <string>
#include <string_view>

namespace Parse {
    struct ParseResult {
        std::string result;
        bool success;
        bool is_math;
        bool is_floating_point;
        
        ParseResult(std::string_view _result, const bool _err, const bool _math, const bool _fp) : 
        result(_result), success(_err), is_math(_math), is_floating_point(_fp) {}

        ParseResult(std::string&& _result, const bool _err, const bool _math, const bool _fp) : 
        result(std::move(_result)), success(_err), is_math(_math), is_floating_point(_fp) {}
    };

    inline void empty_stack(std::stack<char>& operator_stack) noexcept {
        while (!operator_stack.empty()) {
            operator_stack.pop();
        }
    }
    
    inline void clear_num_buffer(std::string& buf, bool& in_num) noexcept {
        in_num = false;
        buf.clear();
    }

    [[nodiscard]] ParseResult create_prefix_expression(std::string& infix_expression);
};

#endif
