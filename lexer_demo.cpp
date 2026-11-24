#include "bminor/lexer.hpp"
#include <iostream>

int main() {
    bminor::Lexer lexer(
        "if ifCount <= 10 // comment\n"
        "@ return \"hello\\n\";",
        "sample.bminor"
    );

    for (const auto& token : lexer.tokenize()) {
        std::cout << token.location.line << ':' << token.location.column
                  << ' ' << bminor::tokenTypeName(token.type)
                  << " [" << token.lexeme << "]\n";
    }

    for (const auto& error : lexer.errors()) {
        std::cout << error.location.file << ':' << error.location.line
                  << ':' << error.location.column << ": " << error.message << '\n';
    }
}
