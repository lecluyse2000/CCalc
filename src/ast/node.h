// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <memory>

struct Node {
    explicit Node(const char token) noexcept;
    virtual ~Node() = default;
    const char m_key;
    std::unique_ptr<Node> m_left_child;
    std::unique_ptr<Node> m_right_child;
    [[nodiscard]] virtual bool evaluate() const = 0;
};

struct BoolNode : public Node {
    explicit BoolNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct OperationNode : public Node {
    explicit OperationNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct UnaryNode : public Node {
    explicit UnaryNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

#endif
