#pragma once

#include "bminor/token.hpp"

#include <string>
#include <vector>

namespace bminor {

// A single-pass scanner for the lexical rules in the B-Minor language.
// Whitespace and comments are skipped; invalid input is returned as Invalid
// tokens and also recorded in errors(), allowing callers to continue scanning.
class Lexer {
public:
    explicit Lexer(std::string source, std::string fileName = "<input>");

    Token nextToken();
    std::vector<Token> tokenize();

    const std::vector<LexicalError>& errors() const noexcept { return errors_; }

private:
    bool atEnd() const noexcept;
    char peek(std::size_t lookahead = 0) const noexcept;
    bool match(char expected) noexcept;
    char advance() noexcept;
    void skipTrivia();
    Token makeToken(TokenType type, const SourceLocation& start) const;
    SourceLocation currentLocation() const;
    Token scanQuotedLiteral(char quote, TokenType type, const SourceLocation& start);

    std::string source_;
    std::string fileName_;
    std::size_t position_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
    std::vector<LexicalError> errors_;
};

}  // namespace bminor
