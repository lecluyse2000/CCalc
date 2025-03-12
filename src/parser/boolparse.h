#ifndef BOOLPARSE_H
#define BOOLPARSE_H

#include <optional>
#include <stack>
#include <string_view>

#include "include/types.hpp"

namespace BoolParse {

struct BoolParseState {
    std::string_view::reverse_iterator itr;
    Types::Token current_token = Types::Token::NULLCHAR;
    Types::Token previous_token = Types::Token::NULLCHAR;
};

std::optional<std::string> parse_bool(const std::string_view infix_expression, std::vector<Types::Token>& prefix_expression,
                                      std::stack<Types::Token>& operator_stack);
}

#endif
