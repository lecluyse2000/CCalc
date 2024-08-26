// Author: Caden LeCluyse

#ifndef PARSER_H
#define PARSER_H

#include <optional>
#include <stack>
#include <string>
#include <string_view>

class Parser {
   public:
    [[nodiscard]] std::pair<std::string, bool> create_prefix_expression(const std::string_view infix_expression);

   private:
    void empty_stack() noexcept;
    void reset_state() noexcept;
    [[nodiscard]] std::optional<std::string> parse(const std::string_view infix_expression,
                                                   std::string& prefix_expression);
    [[nodiscard]] std::optional<std::string> clear_stack(std::string& prefix_expression);
    char m_current_token;
    char m_previous_token;
    std::stack<char> m_operator_stack;
};

#endif
