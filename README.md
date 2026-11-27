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
Both renderers copy the gaps between spans directly from the original text.
Function declarations and named call targets are classified using the AST.
Other identifiers, including those in regions the parser could not recover,
use the default identifier category. Valid comments are tracked by the lexer
without adding them to the parser's token stream.

Include `bminor/render.hpp` and pass the same source text to the renderers:

```cpp
const std::string source = "main: function integer () = { return 0; }";
const auto highlighted = bminor::highlight(source, "example.bminor");
const auto terminal = bminor::renderAnsi(source, highlighted.spans);
const auto html = bminor::renderHtml(source, highlighted.spans);
```

Print the ANSI string in an ANSI-capable terminal or save the HTML string as
a standalone UTF-8 file. The HTML includes its stylesheet and escapes source
text before placing it in the code block. Both functions reject spans that
overlap, are out of order, or fall outside the source.

## Command line

On Windows with MinGW, build and run from the project root:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
cmake --build build-mingw
.\build-mingw\bminor.exe --ansi example.bminor
.\build-mingw\bminor.exe --html -o example.html example.bminor
```

ANSI is the default format. Without `-o`, the result goes to stdout.
Diagnostics go to stderr; lexical or syntax errors still produce highlighted
output and return exit code 1. Argument and file errors return 2; success
returns 0. Use `--help` to see the command syntax.
