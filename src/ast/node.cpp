// Author: Caden LeCluyse

#include "node.h"

#include <cctype>
#include <gmpxx.h>
#include <limits>
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

static inline mpz_class mpz_exponent(mpz_class& left_value, mpz_class& right_value) {
    if (right_value == 0) return 1;
    if (right_value == 1) return left_value;
    mpz_class retval = 1;
    while (right_value > 0) {
        if (right_value % 2 == 1) {
            retval *= left_value;
        }
        left_value *= left_value;
        right_value /= 2;
    }
    
    return retval;
}

[[nodiscard]] mpz_class OperationMNode::evaluate() const {
    mpz_class left_value = m_left_child->evaluate();
    mpz_class right_value = m_right_child->evaluate();

    switch(key) {
        case '+':
            return left_value + right_value;
        case '-':
            return left_value - right_value;
        case '*':
            return left_value * right_value;
        default:
            return mpz_exponent(left_value, right_value);
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
                throw std::runtime_error("Divide by zero error");
            }
            mpfr_div(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        default:
            mpfr_pow(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
    }
}

[[nodiscard]] mpz_class FactorialNode::evaluate() const { return factorial(m_left_child->evaluate()); }

mpfr_t& FactorialNode::evaluate_float() {
    const mpfr_t& prev_val = m_left_child->evaluate_float();
    if (!mpfr_integer_p(prev_val)) {
        throw std::runtime_error("Factorial called on non integer value");
    } else if (mpfr_sgn(prev_val) < 0) {
        throw std::runtime_error("Factorial called on negative integer value");
    }
    const unsigned long int prev_val_int = mpfr_get_ui(prev_val, MPFR_RNDN);
    if (prev_val_int == std::numeric_limits<unsigned long int>::max() ||
        prev_val_int == std::numeric_limits<unsigned long int>::min()) {
        throw std::runtime_error("Value is too big for factorial");
    }
    mpfr_fac_ui(node_result, prev_val_int, MPFR_RNDN);
    return node_result;
}

[[nodiscard]] mpz_class UnaryMNode::evaluate() const { return m_left_child->evaluate() * -1; }

mpfr_t& UnaryMNode::evaluate_float() {
    const mpfr_t& prev_val = m_left_child->evaluate_float();
    mpfr_mul_si(node_result, prev_val, -1L, MPFR_RNDN);
    return node_result;
}
