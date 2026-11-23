#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace bminor {

enum class TokenType {
    EndOfFile,
    Invalid,

    Identifier,
    IntegerLiteral,
    CharacterLiteral,
    StringLiteral,

    KwArray,
    KwBoolean,
    KwChar,
    KwElse,
    KwFalse,
    KwFor,
    KwFunction,
    KwIf,
    KwInteger,
    KwPrint,
    KwReturn,
    KwString,
    KwTrue,
    KwVoid,
    KwWhile,

    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %
    Power,         // ^
    Bang,          // !
    Increment,     // ++
    Decrement,     // --
    EqualEqual,    // ==
    BangEqual,     // !=
    Less,          // <
    LessEqual,     // <=
    Greater,       // >
    GreaterEqual,  // >=
    AndAnd,        // &&
    OrOr,          // ||
    Assign,        // =

    LeftParen,     // (
    RightParen,    // )
    LeftBracket,   // [
    RightBracket,  // ]
    LeftBrace,     // {
    RightBrace,    // }
    Comma,         // ,
    Colon,         // :
    Semicolon,     // ;
};

struct SourceLocation {
    std::string file;
    std::size_t line = 1;
    std::size_t column = 1;
    std::size_t offset = 0;
};

struct Token {
    TokenType type = TokenType::Invalid;
    std::string lexeme;
    SourceLocation location;
};

struct LexicalError {
    SourceLocation location;
    std::string message;
    std::string lexeme;
};

std::string_view tokenTypeName(TokenType type) noexcept;

}  // namespace bminor
