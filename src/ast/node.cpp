// Author: Caden LeCluyse

#include "node.h"

#include <cctype>
#include <memory>

Node::Node(const char token) : m_key(token) {}

void Node::set_left(std::unique_ptr<Node>&& node) { m_left_child = std::move(node); }

void Node::set_right(std::unique_ptr<Node>&& node) { m_right_child = std::move(node); }

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
    }

    // If we are still in the function here it means that the operation is xor
    return (!left_value && right_value) || (left_value && !right_value);
}

[[nodiscard]] bool UnaryNode::evaluate() const { return !m_left_child->evaluate(); }
