#pragma once

#include "bminor/parser.hpp"
#include "bminor/lexer.hpp"

namespace bminor {

enum class HighlightKind {
    Keyword, Type, Identifier, Function, Number, String, Character, Boolean,
    Operator, Punctuation, Comment, Invalid
};

struct HighlightSpan {
    std::size_t offset;
    std::size_t length;
    HighlightKind kind;
};

struct HighlightResult {
    std::vector<HighlightSpan> spans;
    std::vector<LexicalError> lexicalErrors;
    std::vector<SyntaxError> syntaxErrors;
};

HighlightResult highlight(std::string source, std::string fileName = "<input>");

}  // namespace bminor
