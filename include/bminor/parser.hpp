#pragma once

#include "bminor/ast.hpp"

namespace bminor {

struct SyntaxError {
    SourceLocation location;
    std::string message;
};

struct ParseResult {
    Ast program;
    std::vector<SyntaxError> errors;
};

ParseResult parse(std::vector<Token> tokens);

}  // namespace bminor
