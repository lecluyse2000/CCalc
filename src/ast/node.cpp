// Author: Caden LeCluyse

#include "node.h"

#include <cctype>
#include <cmath>
#include <gmpxx.h>
#include <memory>
#include <mpfr.h>

BoolNode::BoolNode(const char token) noexcept : key(token){}

[[nodiscard]] bool ValueBNode::evaluate() const { return toupper(key) == 'T'; }

[[nodiscard]] bool OperationBNode::evaluate() const {
    const bool left_value = m_left_child->evaluate();
    const bool right_value = m_right_child->evaluate();

    switch (key) {
        case '&':
            return left_value && right_value;
        case '|':
            return left_value || right_value;
        case '@':
            return !(left_value && right_value);
        default:
            return (!left_value && right_value) || (left_value && !right_value);
    }

}

[[nodiscard]] bool UnaryBNode::evaluate() const { return !m_left_child->evaluate(); }

[[nodiscard]] mpz_class ValueMNode::evaluate() const { return value_mpz; }

[[nodiscard]] mpfr_t& ValueMNode::evaluate_float() { return value_mpfr; }

[[nodiscard]] mpz_class OperationMNode::evaluate() const {
    const mpz_class left_value = m_left_child->evaluate();
    const mpz_class right_value = m_right_child->evaluate();

    switch(key) {
        case '+':
            return left_value + right_value;
        case '-':
            return left_value - right_value;
        case '*':
            return left_value * right_value;
        default:
            return left_value ^ right_value;
    }
}

mpfr_t& OperationMNode::evaluate_float() {
    const mpfr_t& left_value = m_left_child->evaluate_float();
    const mpfr_t& right_value = m_right_child->evaluate_float();

    switch(key) {
        case '+':
            mpfr_add(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case '-':
            mpfr_sub(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case '*':
            mpfr_mul(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case '/':
            if (mpfr_zero_p(right_value)) {
                throw std::runtime_error("Divide by zero error!");
            }
            mpfr_div(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        default:
            mpfr_pow(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
    }
}

[[nodiscard]] mpz_class UnaryMNode::evaluate() const { return m_left_child->evaluate() * -1; }

mpfr_t& UnaryMNode::evaluate_float() {
    const mpfr_t& prev_val = m_left_child->evaluate_float();
    mpfr_mul_si(node_result, prev_val, -1L, MPFR_RNDN);
    return node_result;
}
