#include "bminor/parser.hpp"

#include <initializer_list>
#include <utility>

namespace bminor {
namespace {

using Kind = NodeKind;
using Type = TokenType;

Ast node(Kind kind, const Token& start) {
    return std::make_unique<AstNode>(
        AstNode{kind, start.location, std::nullopt, std::nullopt, {}});
}

Ast withToken(Kind kind, const Token& token) {
    auto result = node(kind, token);
    result->token = token;
    return result;
}

int precedence(Type type) {
    switch (type) {
        case Type::Assign: return 1;
        case Type::OrOr: return 2;
        case Type::AndAnd: return 3;
        case Type::EqualEqual: case Type::BangEqual: return 4;
        case Type::Less: case Type::LessEqual:
        case Type::Greater: case Type::GreaterEqual: return 5;
        case Type::Plus: case Type::Minus: return 6;
        case Type::Star: case Type::Slash: case Type::Percent: return 7;
        case Type::Power: return 8;
        default: return 0;
    }
}

struct Recover {};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
        if (tokens_.empty() || tokens_.back().type != Type::EndOfFile) {
            auto location = tokens_.empty() ? SourceLocation{} : tokens_.back().location;
            if (!tokens_.empty()) {
                location.offset += tokens_.back().lexeme.size();
                location.column += tokens_.back().lexeme.size();
            }
            tokens_.push_back({Type::EndOfFile, "", std::move(location)});
        }
    }

    ParseResult run() {
        auto program = node(Kind::Program, current());
        sequence(*program, true);
        return {std::move(program), std::move(errors_)};
    }

private:
    const Token& current() const { return tokens_[position_]; }
    bool at(Type type) const { return current().type == type; }
    const Token& take() { return tokens_[position_++]; }
    bool accept(Type type) {
        if (!at(type)) return false;
        take();
        return true;
    }
    bool oneOf(std::initializer_list<Type> types) const {
        for (auto type : types) if (at(type)) return true;
        return false;
    }
    [[noreturn]] void fail(std::string expected) {
        errors_.push_back({current().location, "expected " + expected +
            ", found " + std::string(tokenTypeName(current().type))});
        throw Recover{};
    }
    const Token& expect(Type type, const char* expected) {
        if (!at(type)) fail(expected);
        return take();
    }
    bool startsDecl() const {
        return at(Type::Identifier) && tokens_[position_ + 1].type == Type::Colon;
    }

    void sequence(AstNode& parent, bool top) {
        while (!at(Type::EndOfFile) && (top || !at(Type::RightBrace))) {
            const auto start = position_;
            try {
                parent.children.push_back(top ? declaration(true) : statement());
            } catch (const Recover&) {
                synchronize(start, top);
            }
        }
    }

    void synchronize(std::size_t start, bool top) {
        std::size_t braces = 0;
        while (!at(Type::EndOfFile)) {
            if (braces == 0) {
                if (!top && at(Type::RightBrace)) return;
                if (position_ > start &&
                    (startsDecl() || (!top && oneOf({Type::KwIf, Type::KwFor,
                                                   Type::KwReturn, Type::KwPrint})))) return;
                if (accept(Type::Semicolon)) return;
            }
            if (at(Type::LeftBrace)) ++braces;
            else if (at(Type::RightBrace)) {
                if (braces == 0) {
                    take();
                    return;
                }
                --braces;
            }
            take();
        }
    }

    Ast atomicType(bool allowVoid) {
        if (oneOf({Type::KwInteger, Type::KwBoolean, Type::KwChar, Type::KwString}) ||
            (allowVoid && at(Type::KwVoid))) return withToken(Kind::AtomicType, take());
        fail(allowVoid ? "a return type" : "an atomic type");
    }

    Ast valueType(bool parameter) {
        if (at(Type::KwArray)) {
            auto result = withToken(Kind::ArrayType, take());
            expect(Type::LeftBracket, "'['");
            if (!parameter || !at(Type::RightBracket)) {
                result->children.push_back(withToken(Kind::Literal,
                    expect(Type::IntegerLiteral, "an array size")));
            } else {
                result->children.push_back(nullptr);
            }
            expect(Type::RightBracket, "']'");
            result->children.push_back(valueType(parameter));
            return result;
        }
        if (parameter && at(Type::KwFunction)) return functionType();
        return atomicType(false);
    }

    Ast functionType() {
        auto result = withToken(Kind::FunctionType, expect(Type::KwFunction, "'function'"));
        result->children.push_back(atomicType(true));
        expect(Type::LeftParen, "'('");
        if (!at(Type::RightParen)) {
            do {
                auto parameter = withToken(Kind::Parameter,
                    expect(Type::Identifier, "a parameter name"));
                expect(Type::Colon, "':'");
                parameter->children.push_back(valueType(true));
                result->children.push_back(std::move(parameter));
            } while (accept(Type::Comma));
        }
        expect(Type::RightParen, "')'");
        return result;
    }

    Ast declaration(bool top) {
        const auto& name = expect(Type::Identifier, "a declaration name");
        expect(Type::Colon, "':'");
        if (at(Type::KwFunction)) {
            auto result = withToken(Kind::FunctionDecl, name);
            result->children.push_back(functionType());
            if (at(Type::Assign)) {
                if (!top) fail("';' (nested function definitions are not allowed)");
                take();
                result->children.push_back(block());
            } else {
                expect(Type::Semicolon, "';'");
            }
            return result;
        }
        auto result = withToken(Kind::VariableDecl, name);
        auto type = valueType(false);
        const bool array = type->kind == Kind::ArrayType;
        result->children.push_back(std::move(type));
        if (accept(Type::Assign)) {
            result->children.push_back(array && at(Type::LeftBrace) ? initializerList() : expression());
        }
        expect(Type::Semicolon, "';'");
        return result;
    }

    Ast initializerList() {
        auto result = node(Kind::InitializerList, expect(Type::LeftBrace, "'{'"));
        do {
            result->children.push_back(at(Type::LeftBrace) ? initializerList() : expression());
        } while (accept(Type::Comma));
        expect(Type::RightBrace, "'}'");
        return result;
    }

    Ast block() {
        auto result = node(Kind::Block, expect(Type::LeftBrace, "'{'"));
        sequence(*result, false);
        expect(Type::RightBrace, "'}'");
        return result;
    }

    Ast statement() {
        if (startsDecl()) return declaration(false);
        if (at(Type::LeftBrace)) return block();
        if (at(Type::Semicolon)) return node(Kind::EmptyStmt, take());
        if (at(Type::KwIf)) {
            auto result = withToken(Kind::IfStmt, take());
            expect(Type::LeftParen, "'('");
            result->children.push_back(expression());
            expect(Type::RightParen, "')'");
            result->children.push_back(statement());
            if (accept(Type::KwElse)) result->children.push_back(statement());
            return result;
        }
        if (at(Type::KwFor)) {
            auto result = withToken(Kind::ForStmt, take());
            expect(Type::LeftParen, "'('");
            for (auto stop : {Type::Semicolon, Type::Semicolon, Type::RightParen}) {
                result->children.push_back(at(stop) ? nullptr : expression());
                expect(stop, stop == Type::Semicolon ? "';'" : "')'");
            }
            result->children.push_back(statement());
            return result;
        }
        if (at(Type::KwReturn)) {
            auto result = withToken(Kind::ReturnStmt, take());
            if (!at(Type::Semicolon)) result->children.push_back(expression());
            expect(Type::Semicolon, "';'");
            return result;
        }
        if (at(Type::KwPrint)) {
            auto result = withToken(Kind::PrintStmt, take());
            do { result->children.push_back(expression()); } while (accept(Type::Comma));
            expect(Type::Semicolon, "';'");
            return result;
        }
        auto result = node(Kind::ExprStmt, current());
        result->children.push_back(expression());
        expect(Type::Semicolon, "';'");
        return result;
    }

    Ast expression(int minimum = 1) {
        auto left = unary();
        while (precedence(current().type) >= minimum) {
            const auto& op = take();
            const int level = precedence(op.type);
            const bool rightAssociative = op.type == Type::Assign || op.type == Type::Power;
            auto result = withToken(op.type == Type::Assign ? Kind::Assignment : Kind::Binary, op);
            result->location = left->location;
            result->children.push_back(std::move(left));
            result->children.push_back(expression(level + (rightAssociative ? 0 : 1)));
            left = std::move(result);
        }
        return left;
    }

    Ast unary() {
        if (oneOf({Type::Minus, Type::Bang})) {
            auto result = withToken(Kind::Unary, take());
            result->children.push_back(unary());
            return result;
        }
        auto base = primary();
        while (oneOf({Type::LeftParen, Type::LeftBracket})) {
            const auto& op = take();
            const bool call = op.type == Type::LeftParen;
            auto result = withToken(call ? Kind::Call : Kind::Subscript, op);
            result->location = base->location;
            result->children.push_back(std::move(base));
            if (call) {
                if (!at(Type::RightParen)) {
                    do { result->children.push_back(expression()); } while (accept(Type::Comma));
                }
                expect(Type::RightParen, "')'");
            } else {
                result->children.push_back(expression());
                expect(Type::RightBracket, "']'");
            }
            base = std::move(result);
        }
        while (oneOf({Type::Increment, Type::Decrement})) {
            auto result = withToken(Kind::Postfix, take());
            result->location = base->location;
            result->children.push_back(std::move(base));
            base = std::move(result);
        }
        return base;
    }

    Ast primary() {
        if (at(Type::Identifier)) return withToken(Kind::Identifier, take());
        if (oneOf({Type::IntegerLiteral, Type::CharacterLiteral, Type::StringLiteral,
                   Type::KwTrue, Type::KwFalse})) return withToken(Kind::Literal, take());
        if (at(Type::LeftParen)) {
            auto result = node(Kind::Group, take());
            result->children.push_back(expression());
            expect(Type::RightParen, "')'");
            return result;
        }
        fail("an expression");
    }

    std::vector<Token> tokens_;
    std::size_t position_ = 0;
    std::vector<SyntaxError> errors_;
};

}  // namespace

ParseResult parse(std::vector<Token> tokens) {
    return Parser(std::move(tokens)).run();
}

}  // namespace bminor
