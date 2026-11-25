# B-Minor syntax

Source: the Phase 1 handout, sections 3–5; Douglas Thain's compiler book,
Appendix B. The grammar below uses the notation from Appendix 1: brackets
mean optional, braces mean repetition, and quoted punctuation is literal.

```ebnf
program       = { globalDecl }, EOF ;
globalDecl    = IDENT, ":", ( variableTail | functionType, ( ";" | "=", block ) ) ;
localDecl     = IDENT, ":", ( variableTail | functionType, ";" ) ;
variableTail  = atomic, [ "=", expr ], ";"
              | arrayType, [ "=", ( expr | initializer ) ], ";" ;
atomic        = "integer" | "boolean" | "char" | "string" ;
returnType    = atomic | "void" ;
arrayType     = "array", "[", INTEGER, "]", ( atomic | arrayType ) ;
paramType     = atomic | functionType | "array", "[", [ INTEGER ], "]", paramType ;
functionType  = "function", returnType, "(", [ parameter, { ",", parameter } ], ")" ;
parameter     = IDENT, ":", paramType ;
initializer   = "{", initItem, { ",", initItem }, "}" ;
initItem      = expr | initializer ;
block         = "{", { stmt }, "}" ;
stmt          = localDecl | block | ";"
              | "if", "(", expr, ")", stmt, [ "else", stmt ]
              | "for", "(", [ expr ], ";", [ expr ], ";", [ expr ], ")", stmt
              | "return", [ expr ], ";"
              | "print", exprList, ";"
              | expr, ";" ;
exprList      = expr, { ",", expr } ;
expr          = logicalOr, [ "=", expr ] ;
logicalOr     = logicalAnd, { "||", logicalAnd } ;
logicalAnd    = equality, { "&&", equality } ;
equality      = relation, { ( "==" | "!=" ), relation } ;
relation      = add, { ( "<" | "<=" | ">" | ">=" ), add } ;
add           = multiply, { ( "+" | "-" ), multiply } ;
multiply      = power, { ( "*" | "/" | "%" ), power } ;
power         = unary, [ "^", power ] ;
unary         = ( "-" | "!" ), unary | postfix ;
postfix       = primary, { "(", [ exprList ], ")" | "[", expr, "]" },
                { "++" | "--" } ;
primary       = IDENT | INTEGER | CHAR | STRING | "true" | "false" | "(", expr, ")" ;
```

The recursive descent parser uses precedence climbing for the expression
levels from assignment through exponentiation. The grammar has no left
recursion: left associative operators repeat, and right associative assignment
and exponentiation consume an operator before recursing. As in C, relational
operators bind more tightly than equality, and logical AND binds more tightly
than OR. Unary operators bind more tightly than exponentiation per Appendix B.

Most choices are distinguished by the first token. At statement start,
`IDENT ':'` selects a declaration; other identifier starts select expressions.
This shared FIRST token uses two tokens of lookahead. For optional expressions
in returns, loop clauses and calls, their FOLLOW delimiters (`;` or `)`) are
absent from their FIRST set. The dangling `else` is resolved by consuming it
in the innermost active `if`. The book reserves `while` but does not define a
while statement. Main's signature, operand types, assignment targets, array
sizes and constant global initializers are checked in later analysis.

## AST and recovery

Every node stores the start of its source construct and an initially empty
type annotation. An optional token preserves the spelling and position of
names, types, literals, keywords and operators. Children are in source order:

| Node | Children |
| --- | --- |
| Program / Block / InitializerList / PrintStmt | List of declarations, statements, initializers or arguments |
| VariableDecl / FunctionDecl | Declared type, then optional initializer or function body |
| FunctionType / Parameter | Return type followed by parameters; a parameter contains its type |
| ArrayType | Size (null for unsized parameters), then element type |
| IfStmt / ForStmt | Condition and branches; for has three fixed expression slots (nullable) followed by its body |
| ReturnStmt | Optional expression |
| ExprStmt / Group / Unary / Postfix | One expression |
| Binary / Assignment / Subscript | Two expressions |
| Call | Callee followed by argument expressions |

At a syntax error, the parser records the unexpected token's location and
unwinds to the nearest block or top-level declaration list. It skips to a
semicolon, closing brace, or a recognizable new declaration or statement
start. The caller receives the partially recovered AST plus the list of
syntax errors. Read lexer diagnostics separately from `Lexer::errors()`.
