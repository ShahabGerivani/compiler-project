#pragma once

#include "bminor/token.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace bminor {

enum class NodeKind {
    Program, VariableDecl, FunctionDecl, Parameter, AtomicType, ArrayType,
    FunctionType, InitializerList, Block, EmptyStmt, ExprStmt, IfStmt,
    ForStmt, ReturnStmt, PrintStmt, Identifier, Literal, Group, Unary,
    Binary, Assignment, Postfix, Call, Subscript
};

struct AstNode {
    NodeKind kind;
    SourceLocation location;
    std::optional<Token> token;
    std::optional<std::string> typeAnnotation;
    std::vector<std::unique_ptr<AstNode>> children;
};

using Ast = std::unique_ptr<AstNode>;

}  // namespace bminor
