// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <string>
#include <string_view>
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
    virtual ~MathNode() = default;
    [[nodiscard]] virtual bool evaluate() const = 0;
    std::unique_ptr<MathNode> m_left_child;
    std::unique_ptr<MathNode> m_right_child;
};

struct ValueMNode : public MathNode {
    explicit ValueMNode(const char token) : value(token - '0') {}
    [[nodiscard]] bool evaluate() const override;
    const long long value;
};

struct ValueFNode : public MathNode {
    explicit ValueFNode(const std::string& _value) : value(std::stold(_value)) {}
    [[nodiscard]] bool evaluate() const override;
    const long double value;
};

struct OperationMNode : public MathNode {
    explicit OperationMNode(const char token) : key(token) {}
    [[nodiscard]] bool evaluate() const override;
    const char key;
};

struct UnaryMNode : public MathNode {
    explicit UnaryMNode(){}
    [[nodiscard]] bool evaluate() const override;
};

#endif
