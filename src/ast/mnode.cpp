// Author: Caden LeCluyse

#include "mnode.h"

#include <cctype>
#include <gmpxx.h>
#include <limits>
#include <memory>
#include <mpfr.h>
#include <stdexcept>

#include "include/types.hpp"

using namespace Types;

namespace MathNodes {

static void radians_to_degrees(mpfr_t& result, const mpfr_t& radians) {
    // Create temporary variable
    mpfr_t pi;
    mpfr_init2(pi, mpfr_get_prec(result));
    
    mpfr_const_pi(pi, MPFR_RNDN);
    
    // Calculate 180/π
    mpfr_t factor;
    mpfr_init2(factor, mpfr_get_prec(result));
    mpfr_set_ui(factor, 180, MPFR_RNDN);
    mpfr_div(factor, factor, pi, MPFR_RNDN);
    
    mpfr_mul(result, radians, factor, MPFR_RNDN);
    
    mpfr_clear(pi);
    mpfr_clear(factor);
}

static void degrees_to_radians(mpfr_t& result, const mpfr_t& degrees) {
    // Create temporary variable for π/180
    mpfr_t pi_div_180;
    mpfr_init2(pi_div_180, mpfr_get_prec(result));
    
    mpfr_const_pi(pi_div_180, MPFR_RNDN);
    mpfr_div_ui(pi_div_180, pi_div_180, 180, MPFR_RNDN);
    mpfr_mul(result, degrees, pi_div_180, MPFR_RNDN);
    
    mpfr_clear(pi_div_180);
}

ValueMNode::ValueMNode(const std::string_view _value_mpz, const std::string_view _value_mpf) :
    value_mpz(_value_mpz.data()) {
    mpfr_init2(value_mpfr, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    const int successful = mpfr_set_str(value_mpfr, _value_mpf.data(), 10, MPFR_RNDN);
    if (successful != 0) [[unlikely]] {
        mpfr_clear(value_mpfr);
        throw std::invalid_argument("Invalid floating point: " + std::string(_value_mpf));
    }
}

ValueMNode::ValueMNode(const Token token) : value_mpz(0) {
    mpfr_init2(value_mpfr, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    if (token == Token::PI) mpfr_const_pi(value_mpfr, MPFR_RNDN);
    if (token == Token::EULER) {
        const int successful = mpfr_set_str(value_mpfr, euler.data(), 10, MPFR_RNDN);
        if (successful != 0) [[unlikely]] {
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
        case Token::ADD:
            return left_value + right_value;
        case Token::SUB:
            return left_value - right_value;
        case Token::MULT:
            return left_value * right_value;
        default:
            return mpz_exponent(left_value, right_value);
    }
}

mpfr_t& OperationMNode::evaluate_float() {
    const mpfr_t& left_value = m_left_child->evaluate_float();
    const mpfr_t& right_value = m_right_child->evaluate_float();

    switch(key) {
        case Token::ADD:
            mpfr_add(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Token::SUB:
            mpfr_sub(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Token::MULT:
            mpfr_mul(node_result, left_value, right_value, MPFR_RNDN);
            return node_result;
        case Token::DIV:
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

mpfr_t& TrigMNode::evaluate_float() {
    const mpfr_t& left_value = m_left_child->evaluate_float();
    const bool use_degrees = Startup::settings.at(Setting::ANGLE) == 1;
    
    switch(key) {
        case Token::SIN:
            if (use_degrees) {
                mpfr_sinu(node_result, left_value, 360, MPFR_RNDN);
            } else {
                mpfr_sin(node_result, left_value, MPFR_RNDN);
            }
            return node_result;
        case Token::COS:
            if (use_degrees) {
                mpfr_cosu(node_result, left_value, 360, MPFR_RNDN);
            } else {
                mpfr_cos(node_result, left_value, MPFR_RNDN);
            }
            return node_result;
        case Token::TAN:
            if (use_degrees) {
                mpfr_tanu(node_result, left_value, 360, MPFR_RNDN);
            } else {
                mpfr_tan(node_result, left_value, MPFR_RNDN);
            }
            return node_result;
        default:
            throw std::runtime_error("Invalid trig node");
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

}
