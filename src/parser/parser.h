// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "include/types.hpp"

namespace Parse {
    [[nodiscard]] Types::ParseResult create_prefix_expression(std::string& infix_expression);
};

#endif
