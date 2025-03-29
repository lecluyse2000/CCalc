#ifndef BNODE_H
#define BNODE_H

#include <memory>

#include "include/types.hpp"

namespace BoolNodes {

struct BoolNode {
    explicit BoolNode(const Types::Token token) noexcept;
    virtual ~BoolNode() = default;
    [[nodiscard]] virtual bool evaluate() const = 0;
    std::unique_ptr<BoolNode> m_left_child;
    std::unique_ptr<BoolNode> m_right_child;
    const Types::Token key;
};

struct ValueBNode : public BoolNode {
    explicit ValueBNode(const Types::Token token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct OperationBNode : public BoolNode {
    explicit OperationBNode(const Types::Token token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct UnaryBNode : public BoolNode {
    explicit UnaryBNode(const Types::Token token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

}

#endif
