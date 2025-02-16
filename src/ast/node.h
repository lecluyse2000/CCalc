// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <string>
#include <memory>
#include <optional>

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
    MathNode() = default;
    virtual ~MathNode() = default;
    [[nodiscard]] virtual long long evaluate() const = 0;
    [[nodiscard]] virtual std::optional<long double> evaluate_float() const = 0;
    std::unique_ptr<MathNode> m_left_child;
    std::unique_ptr<MathNode> m_right_child;
};

struct ValueMNode : public MathNode {
    explicit ValueMNode(const std::string& _value_long, const std::string& _value_double) :
        value_long(std::stoll(_value_long)), value_double(std::stold(_value_double)) {}
    [[nodiscard]] long long evaluate() const override;
    [[nodiscard]] std::optional<long double> evaluate_float() const override;
    const long long value_long;
    const long double value_double;
};

struct OperationMNode : public MathNode {
    explicit OperationMNode(const char token) : key(token) {}
    [[nodiscard]] long long evaluate() const override;
    [[nodiscard]] std::optional<long double> evaluate_float() const override;
    const char key;
};

struct UnaryMNode : public MathNode {
    explicit UnaryMNode(){}
    [[nodiscard]] long long evaluate() const override;
    [[nodiscard]] std::optional<long double> evaluate_float() const override;
};

#endif
