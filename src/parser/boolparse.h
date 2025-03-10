#ifndef BOOLPARSE_H
#define BOOLPARSE_H

#include <optional>
#include <stack>
#include <string_view>

#include "include/types.hpp"

namespace BoolParse {
    struct BoolParseState {
        std::string_view::reverse_iterator itr;
        char current_token = '\0';
        char previous_token = '\0';
    };

    std::optional<std::string> parse_bool(const std::string_view infix_expression, std::vector<Types::Token>& prefix_expression,
                                          std::stack<Types::Token>& operator_stack);
}

#endif
