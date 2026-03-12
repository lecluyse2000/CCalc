// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

#include "include/types.hpp"

namespace Parse {
    [[nodiscard]] Types::ParseResult create_prefix_expression(std::string& infix_expression,
                                                              const std::unordered_map<char, std::string>& var_map);
}

#endif
