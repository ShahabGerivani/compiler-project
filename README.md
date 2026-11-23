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
