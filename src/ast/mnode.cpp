// Author: Caden LeCluyse

#include "mnode.h"

#include <cctype>
#include <gmpxx.h>
#include <limits>
#include <memory>
#include <mpfr.h>
#include <stdexcept>

#include "include/types.hpp"

ValueMNode::ValueMNode(const std::string_view _value_mpz, const std::string_view _value_mpf) :
    value_mpz(_value_mpz.data()) {
    mpfr_init2(value_mpfr, static_cast<mpfr_prec_t>(Startup::settings.at(Types::Setting::PRECISION)));
    const int successful = mpfr_set_str(value_mpfr, _value_mpf.data(), 10, MPFR_RNDN);
    if (successful != 0) {
        mpfr_clear(value_mpfr);
        throw std::invalid_argument("Invalid floating point: " + std::string(_value_mpf));
    }
}

ValueMNode::ValueMNode(const Types::Token token) : value_mpz(0) {
    mpfr_init2(value_mpfr, static_cast<mpfr_prec_t>(Startup::settings.at(Types::Setting::PRECISION)));
    if (token == Types::Token::PI) mpfr_const_pi(value_mpfr, MPFR_RNDN);
    if (token == Types::Token::EULER) {
        const int successful = mpfr_set_str(value_mpfr, Types::euler.data(), 10, MPFR_RNDN);
        if (successful != 0) {
            mpfr_clear(value_mpfr);
            throw std::invalid_argument("Invalid floating point, token type: " + std::string{static_cast<char>(token)} );
        }
    }
}
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
        case Types::Token::ADD:
            return left_value + right_value;
        case Types::Token::SUB:
            return left_value - right_value;
        case Types::Token::MULT:
            return left_value * right_value;
        default:
            return mpz_exponent(left_value, right_value);
    }
}

mpfr_t& OperationMNode::evaluate_float() {
    const mpfr_t& left_value = m_left_child->evaluate_float();
    const mpfr_t& right_value = m_right_child->evaluate_float();

    switch(key) {
        case Types::Token::ADD:
            mpfr_add(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Types::Token::SUB:
            mpfr_sub(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Types::Token::MULT:
            mpfr_mul(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Types::Token::DIV:
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
    mpfr_mul_si(node_result, m_left_child->evaluate_float(), -1L, MPFR_RNDN);
    return node_result;
}
