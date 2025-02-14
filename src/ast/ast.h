// Author: Caden LeCluyse

#ifndef AST_H
#define AST_H

#include <memory>
#include <string_view>

#include "node.h"

class AST {
   public:
    AST(const std::string_view expression) noexcept;
    [[nodiscard]] virtual bool evaluate() const = 0;

   private:
    std::unique_ptr<Node> build_ast() noexcept;
    const std::string_view m_prefix_expression;
    std::size_t m_index;
    std::unique_ptr<Node> m_root;
};

class MathAST: public AST {
    MathAST(const std::string_view expression) : AST(expression){}
    [[nodiscard]] bool evaluate() const override; 
};

class BoolAST: public AST {
    BoolAST(const std::string_view expression) : AST(expression){}
    [[nodiscard]] bool evaluate() const override; 
};

#endif
