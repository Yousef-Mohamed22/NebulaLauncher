#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Core/Array.hpp>
#include <Platform/StringBuilder.hpp>

namespace nbl {

struct Placeholder {
    StringView key;
    StringView value;
};

bool nextPlaceholder(StringView str, usize* placeholderStart, usize* placeholderEnd, StringView* placeholder);

Result formatPlaceholders(StringView format, Writer writer, Span<Span<Placeholder>> placeholders, bool strict=true);

inline Result formatPlaceholders(StringView format, Writer writer, Span<Placeholder> placeholders, bool strict=true) {
    return formatPlaceholders(format, writer, nbl::Array { placeholders }, strict);
}

inline Result formatPlaceholders(StringView format, StringBuilder* builder, Span<Placeholder> placeholders, bool strict=true) {
    return formatPlaceholders(format, builder->writer(), placeholders, strict);
}

}
