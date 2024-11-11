// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <optional>
#include <stack>
#include <string>
#include <string_view>

namespace Parse {
    inline void empty_stack(std::stack<char>& operator_stack) noexcept {
        while (!operator_stack.empty()) {
            operator_stack.pop();
        }
    }

    [[nodiscard]] std::optional<std::string_view> parse(const std::string_view infix_expression,
                                                   std::string& prefix_expression);
    [[nodiscard]] std::optional<std::string_view> clear_stack(std::string& prefix_expression);
    [[nodiscard]] std::pair<std::string, const bool> create_prefix_expression(const std::string_view infix_expression);
};

#endif
