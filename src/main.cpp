#include "bminor/render.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

namespace {

void usage(std::ostream& out) {
    out << "Usage: bminor [--ansi | --html] [-o OUTPUT] [--] INPUT\n";
}

void diagnostic(const bminor::SourceLocation& location, const char* kind,
                const std::string& message) {
    std::cerr << location.file << ':' << location.line << ':' << location.column
              << ": " << kind << " error: " << message << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string input;
    std::string output;
    bool html = false;
    bool formatSet = false;
    bool options = true;
    for (int index = 1; index < argc; ++index) {
        const std::string_view arg = argv[index];
        if (options && arg == "--help") {
            usage(std::cout);
            return 0;
        }
        if (options && arg == "--") {
            options = false;
        } else if (options && (arg == "--ansi" || arg == "--html") && !formatSet) {
            html = arg == "--html";
            formatSet = true;
        } else if (options && arg == "-o" && output.empty() && index + 1 < argc &&
                   argv[index + 1][0] != '\0' && argv[index + 1][0] != '-') {
            output = argv[++index];
        } else if (!arg.empty() && (!options || arg.front() != '-') && input.empty()) {
            input = arg;
        } else {
            std::cerr << "Invalid argument: " << arg << '\n';
            usage(std::cerr);
            return 2;
        }
    }
    if (input.empty()) {
        usage(std::cerr);
        return 2;
    }

    std::ifstream file(input, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open input: " << input << '\n';
        return 2;
    }
    std::string source;
    char buffer[8192];
    while (file.read(buffer, sizeof buffer) || file.gcount() != 0) {
        source.append(buffer, static_cast<std::size_t>(file.gcount()));
    }
    if (file.bad() || !file.eof()) {
        std::cerr << "Cannot read input: " << input << '\n';
        return 2;
    }

    const auto result = bminor::highlight(source, input);
    for (const auto& error : result.lexicalErrors) diagnostic(error.location, "lexical", error.message);
    for (const auto& error : result.syntaxErrors) diagnostic(error.location, "syntax", error.message);
    const auto rendered = html ? bminor::renderHtml(source, result.spans)
                               : bminor::renderAnsi(source, result.spans);
    if (!output.empty()) {
        std::error_code error;
        if (std::filesystem::equivalent(input, output, error)) {
            std::cerr << "Output must differ from input: " << input << '\n';
            return 2;
        }
        std::ofstream destination(output, std::ios::binary);
        destination << rendered;
        destination.close();
        if (!destination) {
            std::cerr << "Cannot write output: " << output << '\n';
            return 2;
        }
    } else {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
        const auto console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        const bool restore = !html && GetConsoleMode(console, &mode) &&
                             SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
        std::cout << rendered << std::flush;
#ifdef _WIN32
        if (restore) SetConsoleMode(console, mode);
#endif
        if (!std::cout) {
            std::cerr << "Cannot write to standard output\n";
            return 2;
        }
    }
    return result.lexicalErrors.empty() && result.syntaxErrors.empty() ? 0 : 1;
}
