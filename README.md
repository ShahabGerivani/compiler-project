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
