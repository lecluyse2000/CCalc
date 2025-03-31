// Author: Caden LeCluyse

#include "ast.h"

#include <cctype>
#include <gmpxx.h>
#include <mpfr.h>
#include <span>

#include "include/types.hpp"
#include "bnode.h"
#include "mnode.h"

using namespace Types;

std::unique_ptr<BoolNodes::BoolNode> BoolAST::build_ast() noexcept {
    const Token current_token = m_prefix_expression[m_index++];

    if (is_bool_operand(current_token)) {
        return std::make_unique<BoolNodes::ValueBNode>(current_token);
    }

    std::unique_ptr<BoolNodes::BoolNode> node;
    if (isnot(current_token)) {
        node = std::make_unique<BoolNodes::UnaryBNode>(current_token);
        node->m_left_child = build_ast();
    } else {
        node = std::make_unique<BoolNodes::OperationBNode>(current_token);
        node->m_left_child = build_ast();
        node->m_right_child = build_ast();
    }

    return node;
}

BoolAST::BoolAST(const std::span<const Token> expression) noexcept :  m_prefix_expression(expression),  m_index(0), m_root(build_ast()){}

[[nodiscard]] bool BoolAST::evaluate() const { return m_root->evaluate(); }

std::unique_ptr<MathNodes::MathNode> MathAST::build_ast() {
    Token current_token = m_prefix_expression[m_index++];
    if (current_token == Token::COMMA) {
        current_token = m_prefix_expression[m_index++];
    }

    if (is_math_operand(current_token)) {
        if (is_math_var(current_token) && current_token != Token::ANS) {
            return std::make_unique<MathNodes::ValueMNode>(current_token); 
        }
        std::string current_num;
        current_num += static_cast<char>(current_token);
        while (m_index < m_prefix_expression.size()) {
            const Token next_token = m_prefix_expression[m_index++];
            if (next_token == Token::COMMA) break;
            current_num += static_cast<char>(next_token); 
        }
        if (m_floating_point) {
            return std::make_unique<MathNodes::ValueMNode>("0", current_num);
        }
        return std::make_unique<MathNodes::ValueMNode>(current_num, "0");
    }

    std::unique_ptr<MathNodes::MathNode> node;
    if (is_trig(current_token)) {
        node = std::make_unique<MathNodes::TrigMNode>(current_token);
        node->m_left_child = build_ast();
    } else if (current_token == Token::FAC) {
        node = std::make_unique<MathNodes::FactorialNode>();
        node->m_left_child = build_ast();
    } else if (current_token == Token::UNARY) {
        node = std::make_unique<MathNodes::UnaryMNode>();
        node->m_left_child = build_ast();
    } else {
        node = std::make_unique<MathNodes::OperationMNode>(current_token);
        node->m_left_child = build_ast();
        node->m_right_child = build_ast();
    }

    return node;
}

MathAST::MathAST(const std::span<const Token> expression, const bool _floating_point) :
    m_prefix_expression(expression), m_index(0), m_floating_point(_floating_point),
    m_root(build_ast()) {
}

[[nodiscard]] mpz_class MathAST::evaluate() const { return m_root->evaluate(); }
[[nodiscard]] mpfr_t& MathAST::evaluate_floating_point() const { return m_root->evaluate_float(); }
