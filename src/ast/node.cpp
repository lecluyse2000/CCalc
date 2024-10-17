// Author: Caden LeCluyse

#include "node.h"

#include <cctype>
#include <memory>

Node::Node(const char token) : m_key(token) {}

[[nodiscard]] bool BoolNode::evaluate() const { return toupper(m_key) == 'T'; }

[[nodiscard]] bool OperationNode::evaluate() const {
    const bool left_value = m_left_child->evaluate();
    const bool right_value = m_right_child->evaluate();

    switch (m_key) {
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

[[nodiscard]] bool UnaryNode::evaluate() const { return !m_left_child->evaluate(); }
