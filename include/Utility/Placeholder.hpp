/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Utility/Placeholder.hpp
 * \author jwall64
 * \brief Functions for finding and applying ${placeholders} in strings
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Core/Array.hpp>
#include <Utility/StringBuilder.hpp>

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
