#pragma once

#include "bminor/highlighter.hpp"

#include <string_view>

namespace bminor {

std::string renderAnsi(std::string_view source, const std::vector<HighlightSpan>& spans);
std::string renderHtml(std::string_view source, const std::vector<HighlightSpan>& spans);

}  // namespace bminor
