# B-Minor compiler project

This repository contains the C++17 front-end for the B-Minor compiler
project. The first implemented component is the lexical analyzer.

## Lexer

The lexer API is in [`include/bminor/lexer.hpp`](include/bminor/lexer.hpp).
It returns typed tokens with their exact source lexemes and source locations,
skips whitespace and both B-Minor comment forms, uses maximal munch for
operators, and records recoverable errors as `INVALID` tokens. The formal
token rules and design decisions are documented in
[`docs/lexer-spec.md`](docs/lexer-spec.md).

Build the lexer library with CMake:

```sh
cmake -S . -B build
cmake --build build
```

## Parser

Link against the CMake target `bminor_parser`, which also links the lexer:

```cpp
#include "bminor/lexer.hpp"
#include "bminor/parser.hpp"

bminor::Lexer lexer("main: function integer () = { return 0; }", "example.bminor");
auto result = bminor::parse(lexer.tokenize());
// Inspect lexer.errors(), result.errors, and result.program.
```

See [the parser specification](docs/parser-spec.md) for the EBNF,
operator precedence, AST layout, and error recovery.

## Syntax highlighting

Link against `bminor_highlighter` and include `bminor/highlighter.hpp`.
Call `bminor::highlight(source, fileName)` to get sorted, non-overlapping
highlight spans (byte offset and length) and lexer/parser diagnostics.
Renderers can copy the gaps between spans directly from the original text.
Function declarations and named call targets are classified using the AST.
Other identifiers, including those in regions the parser could not recover,
use the default identifier category. Valid comments are tracked by the lexer
without adding them to the parser's token stream.
