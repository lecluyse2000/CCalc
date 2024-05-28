// Author: Caden LeCluyse

#include "ast.h"

#include <cctype>
#include <string_view>

#include "node.h"

std::unique_ptr<Node> AST::build_ast() {
    const char current_token = m_prefix_expression[m_index];

    if (toupper(current_token) == 'T' || toupper(current_token) == 'F') {
        m_index++;
        return std::make_unique<BoolNode>(current_token);
    }

    m_index++;
    std::unique_ptr<Node> node;
    if (current_token == '!') {
        node = std::make_unique<UnaryNode>(current_token);
        node->get_left() = build_ast();
    } else {
        node = std::make_unique<OperationNode>(current_token);
        node->get_left() = build_ast();
        node->get_right() = build_ast();
    }

    return node;
}

AST::AST(const std::string_view expression) : m_prefix_expression(expression), m_index(0) { m_root = build_ast(); }

[[nodiscard]] bool AST::evaluate() const { return m_root->evaluate(); }
