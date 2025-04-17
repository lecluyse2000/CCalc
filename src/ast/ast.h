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
    BoolAST() noexcept = default;
    void build_ast(const std::span<const Types::Token> expression) noexcept;
    [[nodiscard]] bool evaluate() const;

   private:
    std::unique_ptr<BoolNodes::BoolNode> rec_build_ast(const std::span<const Types::Token>& prefix_expression, std::size_t& index);
    std::unique_ptr<BoolNodes::BoolNode> m_root;
};

class MathAST {
   public:
    MathAST() = default;
    void build_ast(const std::span<const Types::Token> prefix_expression, const bool floating_point);
    [[nodiscard]] mpz_class evaluate() const;
    [[nodiscard]] mpfr_t& evaluate_floating_point() const;

   private:
    std::unique_ptr<MathNodes::MathNode> build_value_node(const std::span<const Types::Token>& prefix_expression, const bool floating_point,
                                                          std::size_t& index, const Token current_token) const;
    std::unique_ptr<MathNodes::MathNode> rec_build_ast(const std::span<const Types::Token>& prefix_expression, const bool floating_point,
                                                       std::size_t& index);
    std::unique_ptr<MathNodes::MathNode> m_root;
};

#endif
