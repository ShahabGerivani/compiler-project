#include "bminor/lexer.hpp"

#include <array>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace bminor {
namespace {

constexpr std::size_t kMaxIdentifierLength = 256;
constexpr std::size_t kMaxLiteralContents = 256;

bool isDigit(char c) noexcept { return c >= '0' && c <= '9'; }
bool isLetter(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool isIdentifierStart(char c) noexcept { return isLetter(c) || c == '_'; }
bool isIdentifierContinue(char c) noexcept {
    return isIdentifierStart(c) || isDigit(c);
}

const std::unordered_map<std::string_view, TokenType>& keywords() {
    static const std::unordered_map<std::string_view, TokenType> table = {
        {"array", TokenType::KwArray},       {"boolean", TokenType::KwBoolean},
        {"char", TokenType::KwChar},         {"else", TokenType::KwElse},
        {"false", TokenType::KwFalse},       {"for", TokenType::KwFor},
        {"function", TokenType::KwFunction}, {"if", TokenType::KwIf},
        {"integer", TokenType::KwInteger},   {"print", TokenType::KwPrint},
        {"return", TokenType::KwReturn},     {"string", TokenType::KwString},
        {"true", TokenType::KwTrue},         {"void", TokenType::KwVoid},
        {"while", TokenType::KwWhile},
    };
    return table;
}

struct Operator {
    std::string_view spelling;
    TokenType type;
};

// Keep multi-character operators first (maximal munch).
constexpr std::array<Operator, 18> kOperators = {{
    {"++", TokenType::Increment}, {"--", TokenType::Decrement},
    {"==", TokenType::EqualEqual}, {"!=", TokenType::BangEqual},
    {"<=", TokenType::LessEqual}, {">=", TokenType::GreaterEqual},
    {"&&", TokenType::AndAnd}, {"||", TokenType::OrOr},
    {"+", TokenType::Plus}, {"-", TokenType::Minus},
    {"*", TokenType::Star}, {"/", TokenType::Slash},
    {"%", TokenType::Percent}, {"^", TokenType::Power},
    {"!", TokenType::Bang}, {"<", TokenType::Less},
    {">", TokenType::Greater}, {"=", TokenType::Assign},
}};

}  // namespace

std::string_view tokenTypeName(TokenType type) noexcept {
    switch (type) {
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Invalid: return "INVALID";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::IntegerLiteral: return "INTEGER_LITERAL";
        case TokenType::CharacterLiteral: return "CHARACTER_LITERAL";
        case TokenType::StringLiteral: return "STRING_LITERAL";
        case TokenType::KwArray: return "KW_ARRAY";
        case TokenType::KwBoolean: return "KW_BOOLEAN";
        case TokenType::KwChar: return "KW_CHAR";
        case TokenType::KwElse: return "KW_ELSE";
        case TokenType::KwFalse: return "KW_FALSE";
        case TokenType::KwFor: return "KW_FOR";
        case TokenType::KwFunction: return "KW_FUNCTION";
        case TokenType::KwIf: return "KW_IF";
        case TokenType::KwInteger: return "KW_INTEGER";
        case TokenType::KwPrint: return "KW_PRINT";
        case TokenType::KwReturn: return "KW_RETURN";
        case TokenType::KwString: return "KW_STRING";
        case TokenType::KwTrue: return "KW_TRUE";
        case TokenType::KwVoid: return "KW_VOID";
        case TokenType::KwWhile: return "KW_WHILE";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Power: return "POWER";
        case TokenType::Bang: return "BANG";
        case TokenType::Increment: return "INCREMENT";
        case TokenType::Decrement: return "DECREMENT";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::AndAnd: return "AND_AND";
        case TokenType::OrOr: return "OR_OR";
        case TokenType::Assign: return "ASSIGN";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBracket: return "LEFT_BRACKET";
        case TokenType::RightBracket: return "RIGHT_BRACKET";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::Comma: return "COMMA";
        case TokenType::Colon: return "COLON";
        case TokenType::Semicolon: return "SEMICOLON";
    }
    return "UNKNOWN_TOKEN";
}

Lexer::Lexer(std::string source, std::string fileName)
    : source_(std::move(source)), fileName_(std::move(fileName)) {}

bool Lexer::atEnd() const noexcept { return position_ >= source_.size(); }

char Lexer::peek(std::size_t lookahead) const noexcept {
    const auto index = position_ + lookahead;
    return index < source_.size() ? source_[index] : '\0';
}

bool Lexer::match(char expected) noexcept {
    if (atEnd() || source_[position_] != expected) return false;
    advance();
    return true;
}

char Lexer::advance() noexcept {
    if (atEnd()) return '\0';
    const char c = source_[position_++];
    if (c == '\r') {
        if (position_ < source_.size() && source_[position_] == '\n') {
            ++position_;
        }
        ++line_;
        column_ = 1;
        return c;
    }
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

SourceLocation Lexer::currentLocation() const {
    return {fileName_, line_, column_, position_};
}

Token Lexer::makeToken(TokenType type, const SourceLocation& start) const {
    return {type, source_.substr(start.offset, position_ - start.offset), start};
}

void Lexer::skipTrivia() {
    for (;;) {
        while (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r') {
            advance();
        }

        if (peek() == '/' && peek(1) == '/') {
            const auto start = position_;
            while (!atEnd() && peek() != '\n' && peek() != '\r') advance();
            comments_.push_back({start, position_ - start});
            continue;
        }

        if (peek() == '/' && peek(1) == '*') {
            const auto start = currentLocation();
            advance();
            advance();
            while (!atEnd() && !(peek() == '*' && peek(1) == '/')) advance();
            if (atEnd()) {
                errors_.push_back({start, "unterminated block comment",
                                   source_.substr(start.offset)});
                // nextToken() needs to return this error token; leave its start
                // queued by rewinding to the comment opener.
                position_ = start.offset;
                line_ = start.line;
                column_ = start.column;
                return;
            }
            advance();
            advance();
            comments_.push_back({start.offset, position_ - start.offset});
            continue;
        }
        return;
    }
}

Token Lexer::scanQuotedLiteral(char quote, TokenType type, const SourceLocation& start) {
    advance();  // opening quote
    std::size_t contents = 0;
    bool tooLong = false;
    bool nonAsciiCharacter = false;
    while (!atEnd()) {
        const char c = peek();
        if (c == quote) {
            advance();
            auto token = makeToken(type, start);
            if (tooLong) {
                errors_.push_back({start, "literal exceeds 256 characters", token.lexeme});
                token.type = TokenType::Invalid;
            } else if (type == TokenType::CharacterLiteral &&
                       (contents != 1 || nonAsciiCharacter)) {
                errors_.push_back({start,
                                   nonAsciiCharacter
                                       ? "character literal must contain an ASCII character"
                                       : "character literal must contain exactly one character",
                                   token.lexeme});
                token.type = TokenType::Invalid;
            }
            return token;
        }
        if (c == '\n' || c == '\r') {
            auto token = makeToken(TokenType::Invalid, start);
            errors_.push_back({start, "newline in unterminated quoted literal", token.lexeme});
            return token;
        }
        if (c == '\\') {
            advance();
            if (atEnd()) break;
            // B-Minor defines \n and \0 specially; any other escaped
            // character denotes that character, so consume it as one unit.
            const char escaped = advance();
            nonAsciiCharacter = nonAsciiCharacter ||
                                (type == TokenType::CharacterLiteral &&
                                 static_cast<unsigned char>(escaped) > 0x7f);
            ++contents;
        } else {
            const char value = advance();
            nonAsciiCharacter = nonAsciiCharacter ||
                                (type == TokenType::CharacterLiteral &&
                                 static_cast<unsigned char>(value) > 0x7f);
            ++contents;
        }
        tooLong = tooLong || contents > kMaxLiteralContents;
    }

    auto token = makeToken(TokenType::Invalid, start);
    errors_.push_back({start, "unterminated quoted literal", token.lexeme});
    return token;
}

Token Lexer::nextToken() {
    skipTrivia();

    // An unterminated block comment is recorded by skipTrivia and rewound.
    if (position_ < source_.size() && source_.compare(position_, 2, "/*") == 0) {
        const auto start = currentLocation();
        while (!atEnd()) advance();
        auto token = makeToken(TokenType::Invalid, start);
        return token;
    }

    if (atEnd()) return {TokenType::EndOfFile, "", currentLocation()};

    const auto start = currentLocation();
    const char c = peek();

    if (isIdentifierStart(c)) {
        advance();
        while (isIdentifierContinue(peek())) advance();
        auto token = makeToken(TokenType::Identifier, start);
        if (token.lexeme.size() > kMaxIdentifierLength) {
            token.type = TokenType::Invalid;
            errors_.push_back({start, "identifier exceeds 256 characters", token.lexeme});
        } else if (const auto it = keywords().find(token.lexeme); it != keywords().end()) {
            token.type = it->second;
        }
        return token;
    }

    if (isDigit(c)) {
        advance();
        while (isDigit(peek())) advance();
        return makeToken(TokenType::IntegerLiteral, start);
    }

    if (c == '"') return scanQuotedLiteral('"', TokenType::StringLiteral, start);
    if (c == '\'') return scanQuotedLiteral('\'', TokenType::CharacterLiteral, start);

    constexpr std::array<std::pair<char, TokenType>, 9> punctuation = {{
        {'(', TokenType::LeftParen}, {')', TokenType::RightParen},
        {'[', TokenType::LeftBracket}, {']', TokenType::RightBracket},
        {'{', TokenType::LeftBrace}, {'}', TokenType::RightBrace},
        {',', TokenType::Comma}, {':', TokenType::Colon}, {';', TokenType::Semicolon},
    }};
    for (const auto& [spelling, type] : punctuation) {
        if (c == spelling) {
            advance();
            return makeToken(type, start);
        }
    }

    for (const auto& op : kOperators) {
        if (source_.compare(position_, op.spelling.size(), op.spelling) == 0) {
            for (std::size_t i = 0; i < op.spelling.size(); ++i) advance();
            return makeToken(op.type, start);
        }
    }

    advance();
    auto token = makeToken(TokenType::Invalid, start);
    errors_.push_back({start, "unrecognized character", token.lexeme});
    return token;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> result;
    for (;;) {
        auto token = nextToken();
        result.push_back(token);
        if (token.type == TokenType::EndOfFile) break;
    }
    return result;
}

}  // namespace bminor
