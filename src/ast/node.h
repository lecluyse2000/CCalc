// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <memory>

class Node {
   public:
    explicit Node(const char token);
    void set_left(std::unique_ptr<Node>&& node);
    void set_right(std::unique_ptr<Node>&& node);
    [[nodiscard]] virtual bool evaluate() const = 0;
    virtual ~Node() = default;

   protected:
    const char m_key;
    std::unique_ptr<Node> m_left_child;
    std::unique_ptr<Node> m_right_child;
};

class BoolNode : public Node {
   public:
    explicit BoolNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

class OperationNode : public Node {
   public:
    explicit OperationNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

class UnaryNode : public Node {
   public:
    explicit UnaryNode(const char token) : Node(token) {}
    [[nodiscard]] bool evaluate() const override;
};

#endif
