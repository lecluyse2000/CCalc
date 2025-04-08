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

std::unique_ptr<BoolNodes::BoolNode> BoolAST::rec_build_ast(const std::span<const Types::Token>& prefix_expression, std::size_t& index) {
    const Token current_token = prefix_expression[index++];

    if (is_bool_operand(current_token)) {
        return std::make_unique<BoolNodes::ValueBNode>(current_token);
    }

    std::unique_ptr<BoolNodes::BoolNode> node;
    if (isnot(current_token)) {
        node = std::make_unique<BoolNodes::UnaryBNode>(current_token);
        node->m_left_child = rec_build_ast(prefix_expression, index);
    } else {
        node = std::make_unique<BoolNodes::OperationBNode>(current_token);
        node->m_left_child = rec_build_ast(prefix_expression, index);
        node->m_right_child = rec_build_ast(prefix_expression, index);
    }

    return node;
}

void BoolAST::build_ast(const std::span<const Types::Token> prefix_expression) noexcept {
    std::size_t index = 0;
    m_root = rec_build_ast(prefix_expression, index);
}

[[nodiscard]] bool BoolAST::evaluate() const { return m_root->evaluate(); }

std::unique_ptr<MathNodes::MathNode> MathAST::rec_build_ast(const std::span<const Types::Token>& prefix_expression, const bool floating_point,
                                                            std::size_t& index) {
    Token current_token = prefix_expression[index++];
    if (current_token == Token::COMMA) {
        current_token = prefix_expression[index++];
    }

    if (is_math_operand(current_token)) {
        if (is_math_var(current_token) && current_token != Token::ANS) {
            return std::make_unique<MathNodes::ValueMNode>(current_token); 
        }
        std::string current_num;
        current_num += static_cast<char>(current_token);
        while (index < prefix_expression.size()) {
            const Token next_token = prefix_expression[index++];
            if (next_token == Token::COMMA) break;
            current_num += static_cast<char>(next_token); 
        }
        if (floating_point) {
            return std::make_unique<MathNodes::ValueMNode>("0", current_num);
        }
        return std::make_unique<MathNodes::ValueMNode>(current_num, "0");
    }

    std::unique_ptr<MathNodes::MathNode> node;
    if (is_trig(current_token)) {
        node = std::make_unique<MathNodes::TrigMNode>(current_token);
        node->m_left_child = rec_build_ast(prefix_expression, floating_point, index);
    } else if (current_token == Token::FAC) {
        node = std::make_unique<MathNodes::FactorialNode>();
        node->m_left_child = rec_build_ast(prefix_expression, floating_point, index);
    } else if (current_token == Token::UNARY) {
        node = std::make_unique<MathNodes::UnaryMNode>();
        node->m_left_child = rec_build_ast(prefix_expression, floating_point, index);
    } else {
        node = std::make_unique<MathNodes::OperationMNode>(current_token);
        node->m_left_child = rec_build_ast(prefix_expression, floating_point, index);
        node->m_right_child = rec_build_ast(prefix_expression, floating_point, index);
    }

    return node;
}

void MathAST::build_ast(const std::span<const Types::Token> prefix_expression, const bool floating_point) {
    std::size_t index = 0;
    m_root = rec_build_ast(prefix_expression, floating_point, index);
}

[[nodiscard]] mpz_class MathAST::evaluate() const { return m_root->evaluate(); }
[[nodiscard]] mpfr_t& MathAST::evaluate_floating_point() const { return m_root->evaluate_float(); }
