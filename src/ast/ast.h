// Author: Caden LeCluyse

#ifndef AST_H
#define AST_H

#include <gmpxx.h>
#include <memory>
#include <mpfr.h>
#include <string_view>

#include "node.h"

class BoolAST {
   public:
    BoolAST(const std::string_view expression) noexcept;
    [[nodiscard]] bool evaluate() const;

   private:
    std::unique_ptr<BoolNode> build_ast() noexcept;
    const std::string_view m_prefix_expression;
    std::size_t m_index;
    std::unique_ptr<BoolNode> m_root;
};

class MathAST {
   public:
    MathAST(const std::string_view expression, const bool _floating_point) noexcept;
    [[nodiscard]] mpz_class evaluate() const;
    [[nodiscard]] mpfr_t& evaluate_floating_point() const;

   private:
    std::unique_ptr<MathNode> build_ast() noexcept;
    const std::string_view m_prefix_expression;
    std::size_t m_index;
    const bool m_floating_point;
    std::unique_ptr<MathNode> m_root;
};

#endif
