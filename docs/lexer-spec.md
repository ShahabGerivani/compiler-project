# B-Minor lexical specification

This lexer follows Appendix B of *Essentials of Compilation* (Douglas Thain)
and the Phase 1 project requirements. It returns each source token with its
original lexeme and a 1-based file, line, and column location. Whitespace and
comments are skipped. `tokenize()` appends one EOF token.

## Regular expressions

The expressions below use ASCII character classes. `LETTER` is `[A-Za-z]`,
`DIGIT` is `[0-9]`, and `ESC` is a backslash followed by one character. Raw
line breaks are not allowed inside quoted literals.

| Token | Regular expression or set |
|---|---|
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` (up to 256 characters) |
| Integer literal | `[0-9]+` |
| Character literal | `'([^'\\\r\n]|\\.)'` (one ASCII character or escape) |
| String literal | `"([^"\\\r\n]|\\.)*"` (up to 256 contents) |
| Whitespace | `[ \t\r\n]+` (ignored) |
| Line comment | `//[^\r\n]*` (ignored) |
| Block comment | `/*` through the first `*/` (ignored; not nested) |
| Operators | `++ -- == != <= >= && || + - * / % ^ ! < > =` |
| Separators | `( ) [ ] { } , : ;` |

The B-Minor keywords are `array boolean char else false for function if
integer print return string true void while`. A complete identifier is
scanned before keyword lookup, so `returnValue` is an identifier.

The book defines `\n` and `\0` specially in character and string constants;
a backslash before any other character represents that character. The lexer
preserves the original quoted lexeme and leaves escape decoding to a later
stage. Character constants must contain exactly one ASCII character after
escape processing. The book specifies signed 64-bit integer values, but this
lexer retains the digit sequence without numeric conversion so later stages
can check its range without risking overflow during scanning.

## Selection and recovery

Two-character operators are checked before their one-character prefixes,
implementing maximal munch. Identifiers are scanned whole and then looked up
in the keyword table, giving keywords priority without splitting keyword
prefixes from longer identifiers. Comments and whitespace produce no tokens.
An unknown character produces one `INVALID` token and scanning continues.
Unterminated strings or block comments, malformed character constants, and
overlong identifiers or literals also produce `INVALID` tokens and a
corresponding `LexicalError` entry.

## Public API

Include `bminor/lexer.hpp`, construct `bminor::Lexer` from source text and an
optional file name, then call `nextToken()` repeatedly or `tokenize()` once.
The `errors()` accessor exposes recoverable lexical diagnostics. The CMake
target `bminor_lexer` builds the lexer as a C++17 library.
