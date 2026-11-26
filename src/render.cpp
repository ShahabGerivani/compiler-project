#include "bminor/render.hpp"

#include <stdexcept>

namespace bminor {
namespace {

struct Style {
    std::string_view ansi;
    std::string_view cssClass;
};

Style style(HighlightKind kind) {
    switch (kind) {
        case HighlightKind::Keyword:     return {"\x1b[1;34m", "kw"};
        case HighlightKind::Type:        return {"\x1b[36m", "type"};
        case HighlightKind::Identifier:  return {"", "id"};
        case HighlightKind::Function:    return {"\x1b[1;33m", "fn"};
        case HighlightKind::Number:      return {"\x1b[35m", "num"};
        case HighlightKind::String:      return {"\x1b[32m", "str"};
        case HighlightKind::Character:   return {"\x1b[92m", "chr"};
        case HighlightKind::Boolean:     return {"\x1b[95m", "bool"};
        case HighlightKind::Operator:    return {"\x1b[96m", "op"};
        case HighlightKind::Punctuation: return {"\x1b[37m", "punct"};
        case HighlightKind::Comment:     return {"\x1b[90m", "comment"};
        case HighlightKind::Invalid:     return {"\x1b[4;31m", "invalid"};
    }
    throw std::invalid_argument("unknown highlight kind");
}

template <typename Plain, typename Styled>
void renderSpans(std::string_view source, const std::vector<HighlightSpan>& spans,
                 Plain plain, Styled styled) {
    std::size_t cursor = 0;
    for (const auto& span : spans) {
        if (span.offset < cursor || span.offset > source.size() ||
            span.length > source.size() - span.offset) {
            throw std::invalid_argument("invalid highlight span");
        }
        plain(source.substr(cursor, span.offset - cursor));
        styled(source.substr(span.offset, span.length), style(span.kind));
        cursor = span.offset + span.length;
    }
    plain(source.substr(cursor));
}

void appendHtml(std::string& output, std::string_view text) {
    for (char character : text) {
        switch (character) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\'': output += "&#39;"; break;
            case '\r': output += "&#13;"; break;
            default: output += character; break;
        }
    }
}

}  // namespace

std::string renderAnsi(std::string_view source, const std::vector<HighlightSpan>& spans) {
    std::string output;
    output.reserve(source.size());
    renderSpans(source, spans,
        [&](std::string_view text) { output += text; },
        [&](std::string_view text, Style tokenStyle) {
            output += tokenStyle.ansi;
            output += text;
            if (!tokenStyle.ansi.empty()) output += "\x1b[0m";
        });
    return output;
}

std::string renderHtml(std::string_view source, const std::vector<HighlightSpan>& spans) {
    std::string output = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>B-Minor source</title>
<style>
body { background: #181b22; color: #e6e9ef; }
pre { padding: 1rem; overflow-x: auto; }
.kw { color: #90b5ff; font-weight: bold; }
.type { color: #8ed7cb; }
.fn { color: #ffda83; font-weight: bold; }
.num { color: #d8a8ff; }
.str { color: #aad88c; }
.chr { color: #90dbaf; }
.bool { color: #ffaddc; }
.op { color: #8ce6f2; }
.punct { color: #bbc5d4; }
.comment { color: #a4abba; }
.invalid { color: #ff9999; text-decoration: underline; }
</style>
</head>
<body><pre><code>)HTML";
    renderSpans(source, spans,
        [&](std::string_view text) { appendHtml(output, text); },
        [&](std::string_view text, Style tokenStyle) {
            output += "<span class=\"";
            output += tokenStyle.cssClass;
            output += "\">";
            appendHtml(output, text);
            output += "</span>";
        });
    output += "</code></pre></body>\n</html>\n";
    return output;
}

}  // namespace bminor
