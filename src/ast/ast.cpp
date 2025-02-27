// Author: Caden LeCluyse

#include "ast.h"

#include <cctype>
#include <gmpxx.h>
#include <mpfr.h>
#include <string_view>

#include "types.hpp"
#include "node.h"

std::unique_ptr<BoolNode> BoolAST::build_ast() noexcept {
    const char current_token = m_prefix_expression[m_index++];

    if (Types::is_bool_operand(current_token)) {
        return std::make_unique<ValueBNode>(current_token);
    }

    std::unique_ptr<BoolNode> node;
    if (Types::isnot(current_token)) {
        node = std::make_unique<UnaryBNode>(current_token);
        node->m_left_child = build_ast();
    } else {
        node = std::make_unique<OperationBNode>(current_token);
        node->m_left_child = build_ast();
        node->m_right_child = build_ast();
    }

    return node;
}

BoolAST::BoolAST(const std::string_view expression) noexcept :  m_prefix_expression(expression),  m_index(0), m_root(build_ast()){}

[[nodiscard]] bool BoolAST::evaluate() const { return m_root->evaluate(); }

std::unique_ptr<MathNode> MathAST::build_ast() noexcept {
    char current_token = m_prefix_expression[m_index++];
    if (current_token == ',') {
        current_token = m_prefix_expression[m_index++];
    }

    if (Types::isoperand(current_token)) {
        std::string current_num;
        current_num += current_token;
        while (m_index < m_prefix_expression.length()) {
            const char next_token = m_prefix_expression[m_index++];
            if (next_token == ',') break;
            current_num += next_token; 
        }
        if (m_floating_point) {
            return std::make_unique<ValueMNode>("0", current_num);
        }
        return std::make_unique<ValueMNode>(current_num, "0");
    }

    std::unique_ptr<MathNode> node;
    if (current_token == '!') {
        node = std::make_unique<FactorialNode>();
        node->m_left_child = build_ast();
    } else if (current_token == '~') {
        node = std::make_unique<UnaryMNode>();
        node->m_left_child = build_ast();
    } else {
        node = std::make_unique<OperationMNode>(current_token);
        node->m_left_child = build_ast();
        node->m_right_child = build_ast();
    }

    return node;
}

MathAST::MathAST(const std::string_view expression, const bool _floating_point) noexcept :
    m_prefix_expression(expression), m_index(0), m_floating_point(_floating_point),
    m_root(build_ast()) {
}

[[nodiscard]] mpz_class MathAST::evaluate() const { return m_root->evaluate(); }
[[nodiscard]] mpfr_t& MathAST::evaluate_floating_point() const { return m_root->evaluate_float(); }
