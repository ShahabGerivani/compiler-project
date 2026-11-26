#include "bminor/highlighter.hpp"

#include <algorithm>
#include <unordered_set>
#include <utility>

namespace bminor {
namespace {

using Kind = HighlightKind;
using Offsets = std::unordered_set<std::size_t>;

void collectFunctionNames(const AstNode& root, Offsets& offsets) {
    std::vector<const AstNode*> pending{&root};
    while (!pending.empty()) {
        const auto& node = *pending.back();
        pending.pop_back();
        if (node.kind == NodeKind::FunctionDecl && node.token) {
            offsets.insert(node.token->location.offset);
        } else if (node.kind == NodeKind::Call && !node.children.empty()) {
            const auto* callee = node.children.front().get();
            while (callee && callee->kind == NodeKind::Group && !callee->children.empty()) {
                callee = callee->children.front().get();
            }
            if (callee && callee->kind == NodeKind::Identifier && callee->token) {
                offsets.insert(callee->token->location.offset);
            }
        }
        for (const auto& child : node.children) {
            if (child) pending.push_back(child.get());
        }
    }
}

Kind classify(const Token& token, const Offsets& functions) {
    using Type = TokenType;
    switch (token.type) {
        case Type::Identifier:
            return functions.count(token.location.offset) ? Kind::Function : Kind::Identifier;
        case Type::IntegerLiteral: return Kind::Number;
        case Type::StringLiteral: return Kind::String;
        case Type::CharacterLiteral: return Kind::Character;
        case Type::KwTrue: case Type::KwFalse: return Kind::Boolean;
        case Type::KwArray: case Type::KwBoolean: case Type::KwChar:
        case Type::KwInteger: case Type::KwString: case Type::KwVoid:
            return Kind::Type;
        case Type::KwElse: case Type::KwFor: case Type::KwFunction:
        case Type::KwIf: case Type::KwPrint: case Type::KwReturn: case Type::KwWhile:
            return Kind::Keyword;
        case Type::LeftParen: case Type::RightParen: case Type::LeftBracket:
        case Type::RightBracket: case Type::LeftBrace: case Type::RightBrace:
        case Type::Comma: case Type::Colon: case Type::Semicolon:
            return Kind::Punctuation;
        case Type::Invalid: case Type::EndOfFile: return Kind::Invalid;
        default: return Kind::Operator;
    }
}

}  // namespace

HighlightResult highlight(std::string source, std::string fileName) {
    Lexer lexer(std::move(source), std::move(fileName));
    const auto tokens = lexer.tokenize();
    auto parsed = parse(tokens);
    Offsets functions;
    collectFunctionNames(*parsed.program, functions);

    HighlightResult result{{}, lexer.errors(), std::move(parsed.errors)};
    const auto& comments = lexer.comments();
    auto comment = comments.begin();
    for (const auto& token : tokens) {
        while (comment != comments.end() && comment->offset < token.location.offset) {
            result.spans.push_back({comment->offset, comment->length, Kind::Comment});
            ++comment;
        }
        if (token.type != TokenType::EndOfFile) {
            result.spans.push_back({token.location.offset, token.lexeme.size(),
                                    classify(token, functions)});
        }
    }
    return result;
}

}  // namespace bminor
