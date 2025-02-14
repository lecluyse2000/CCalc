// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <memory>

struct BoolNode {
    explicit BoolNode(const char token) noexcept;
    virtual ~BoolNode() = default;
    [[nodiscard]] virtual bool evaluate() const = 0;
    std::unique_ptr<BoolNode> m_left_child;
    std::unique_ptr<BoolNode> m_right_child;
    const char key;
};

struct ValueBNode : public BoolNode {
    explicit ValueBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct OperationBNode : public BoolNode {
    explicit OperationBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct UnaryBNode : public BoolNode {
    explicit UnaryBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct MathNode {
    explicit MathNode(const char token) noexcept;
    virtual ~MathNode() = default;
    std::unique_ptr<MathNode> m_left_child;
    std::unique_ptr<MathNode> m_right_child;
    [[nodiscard]] bool evaluate() const;
    const char key;
};

#endif
