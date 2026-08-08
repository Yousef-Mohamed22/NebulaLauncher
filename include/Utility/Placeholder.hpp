#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Platform/StringBuilder.hpp>

namespace nbl {

struct Placeholder {
    StringView key;
    StringView value;
};

bool nextPlaceholder(StringView str, usize* placeholderStart, usize* placeholderEnd, StringView* placeholder);

Result formatPlaceholders(StringView format, StringBuilder* builder, Span<Placeholder> placeholders);

}
