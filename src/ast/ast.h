// Author: Caden LeCluyse

#ifndef AST_H
#define AST_H

#include <gmpxx.h>
#include <memory>
#include <mpfr.h>
#include <span>

#include "include/types.hpp"
#include "bnode.h"
#include "mnode.h"

class BoolAST {
   public:
    BoolAST(const std::span<const Types::Token> expression) noexcept;
    [[nodiscard]] bool evaluate() const;

   private:
    std::unique_ptr<BoolNodes::BoolNode> build_ast() noexcept;
    const std::span<const Types::Token> m_prefix_expression;
    std::size_t m_index;
    std::unique_ptr<BoolNodes::BoolNode> m_root;
};

class MathAST {
   public:
    MathAST(const std::span<const Types::Token> expression, const bool _floating_point);
    [[nodiscard]] mpz_class evaluate() const;
    [[nodiscard]] mpfr_t& evaluate_floating_point() const;

   private:
    std::unique_ptr<MathNodes::MathNode> build_ast();
    const std::span<const Types::Token> m_prefix_expression;
    std::size_t m_index;
    const bool m_floating_point;
    std::unique_ptr<MathNodes::MathNode> m_root;
};

#endif
