#ifndef MATHPARSE_H
#define MATHPARSE_H

#include <optional>
#include <stack>
#include <string>

#include "include/types.hpp"

namespace MathParse {

    struct MathParseState {
        std::vector<Types::Token> num_buffer;
        std::string::reverse_iterator itr;
        std::string::reverse_iterator rend;
        std::string::iterator end;
        Types::Token current_token = Types::Token::NULLCHAR;
        Types::Token previous_token = Types::Token::NULLCHAR;
        bool in_number = false;
    };

    inline void clear_num_buffer(MathParseState& state) noexcept {
        state.in_number = false;
        state.num_buffer.clear();
    }

    std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                          std::stack<Types::Token>& operator_stack);
}

#endif
