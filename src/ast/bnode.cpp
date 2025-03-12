#include "bnode.h"

#include <memory>

#include "include/types.hpp"

BoolNode::BoolNode(const Types::Token token) noexcept : key(token){}

[[nodiscard]] bool ValueBNode::evaluate() const { return key == Types::Token::TRUE; }

[[nodiscard]] bool OperationBNode::evaluate() const {
    const bool left_value = m_left_child->evaluate();
    const bool right_value = m_right_child->evaluate();

    switch (key) {
        case Types::Token::AND:
            return left_value && right_value;
        case Types::Token::OR:
            return left_value || right_value;
        case Types::Token::NAND:
            return !(left_value && right_value);
        default:
            return (!left_value && right_value) || (left_value && !right_value);
    }

}

[[nodiscard]] bool UnaryBNode::evaluate() const { return !m_left_child->evaluate(); }
