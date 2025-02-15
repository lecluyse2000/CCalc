// Author: Caden LeCluyse

#include "node.h"

#include <cctype>
#include <cmath>
#include <memory>

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

[[nodiscard]] long long ValueMNode::evaluate() const { return value_long; }

[[nodiscard]] long double ValueMNode::evaluate_float() const { return value_double; }

[[nodiscard]] long long OperationMNode::evaluate() const {
    const long long left_value = m_left_child->evaluate();
    const long long right_value = m_right_child->evaluate();

    switch(key) {
        case '+':
            return left_value + right_value;
        case '*':
            return left_value * right_value;
        default:
            if (!right_value) return 1;
            if (right_value == 1) return left_value;
            long long retval = left_value;
            for (int i = 0; i < right_value - 1; ++i) {
                retval *= left_value; 
            }
            return retval;
    }
}

[[nodiscard]] long double OperationMNode::evaluate_float() const {
    const long double left_value = m_left_child->evaluate_float();
    const long double right_value = m_right_child->evaluate_float();

    switch(key) {
        case '+':
            return left_value + right_value;
        case '*':
            return left_value * right_value;
        case '/':
            return left_value / right_value;
        default:
            return static_cast<long double>(std::pow(left_value, right_value));
    }
}

[[nodiscard]] long long UnaryMNode::evaluate() const { return m_left_child->evaluate() * -1; }

[[nodiscard]] long double UnaryMNode::evaluate_float() const { return m_left_child->evaluate_float() * -1.0; }
