#ifndef MATHPARSE_H
#define MATHPARSE_H

#include <optional>
#include <stack>
#include <string>

#include "include/types.hpp"

namespace MathParse {

struct MathParseState {
    MathParseState(std::string& infix) : rend(infix.rend()), end(infix.end()) {}

    std::vector<Types::Token> num_buffer;
    const std::string::reverse_iterator rend;
    const std::string::iterator end;
    std::string::reverse_iterator* itr = nullptr; // Pointer to iterator so the loop can be directly manipulated
    Types::Token current_token = Types::Token::NULLCHAR;
    Types::Token previous_token = Types::Token::NULLCHAR;
    bool in_number = false;
    bool done_parsing = false;
};

inline void clear_num_buffer(MathParseState& state) noexcept {
    state.in_number = false;
    state.num_buffer.clear();
}

std::optional<std::string> parse_math(std::string& infix_expression, Types::ParseResult& result,
                                      std::stack<Types::Token>& operator_stack);
}

#endif
