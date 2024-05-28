// Author: Caden LeCluyse

#ifndef AST_H
#define AST_H

#include <memory>
#include <string_view>

#include "node.h"

class AST {
   public:
    AST(const std::string_view expression);
    [[nodiscard]] bool evaluate() const;

   private:
    std::unique_ptr<Node> build_ast();
    const std::string_view m_prefix_expression;
    std::size_t m_index;
    std::unique_ptr<Node> m_root;
};

#endif
