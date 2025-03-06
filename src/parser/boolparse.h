#ifndef BOOLPARSE_H
#define BOOLPARSE_H

#include <optional>
#include <stack>
#include <string>
#include <string_view>

namespace BoolParse {
    struct BoolParseState {
        std::string_view::reverse_iterator itr;
        char current_token = '\0';
        char previous_token = '\0';
    };

    std::optional<std::string> parse_bool(const std::string_view infix_expression, std::string& prefix_expression,
                                          std::stack<char>& operator_stack);
}

#endif
