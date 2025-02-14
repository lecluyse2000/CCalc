// Author: Caden LeCluyse

#ifndef AST_H
#define AST_H

#include <memory>
#include <string_view>

#include "node.h"

class BoolAST {
   public:
    BoolAST(const std::string_view expression) noexcept;
    [[nodiscard]] virtual bool evaluate() const = 0;

   private:
    std::unique_ptr<BoolNode> build_ast() noexcept;
    std::unique_ptr<BoolNode> m_root;
    const std::string_view m_prefix_expression;
    std::size_t m_index;
};

class MathAST {
   public:
    MathAST(const std::string_view expression) noexcept;
    [[nodiscard]] virtual bool evaluate() const = 0;

   private:
    std::unique_ptr<MathNode> build_ast() noexcept;
    std::unique_ptr<MathNode> m_root;
    const std::string_view m_prefix_expression;
    std::size_t m_index;
};

#endif
