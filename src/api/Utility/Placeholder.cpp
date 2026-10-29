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
 * \file Utility/Placeholder.cpp
 * \author jwall64
 * \brief Implementation of Utility/Placeholder.hpp
 */

#include <Utility/Placeholder.hpp>
#include <Platform/Logging.hpp>

bool nbl::nextPlaceholder(StringView str, usize* placeholderStart, usize* placeholderEnd, StringView* placeholder) {

    usize depth = 0;
    usize start = 0;
    for(usize i = 0 ; i < str.length() ; i++) {
        switch(depth) {
            case 0:
                if(str[i] == '$') {
                    depth = 1;
                }
                break;
            case 1:
                if(str[i] == '{') {
                    depth = 2;
                    start = i - 1;
                } else {
                    depth = 0;
                }
                break;
            case 2:
                if(str[i] == '}') {
                    *placeholderEnd = i;
                    *placeholderStart = start;
                    *placeholder = StringView(str.data() + start + 2, i - start - 2);
                    return true;
                }
                break;
        }
    }

    return false;
}

nbl::Result nbl::formatPlaceholders(StringView format, Writer writer, Span<Span<Placeholder>> placeholders, bool strict) {

    if(!strict && placeholders.length() == 0) {
        return writer.writeString(format);
    }

    usize placeholderStart = 0;
    usize placeholderEnd = 0;
    StringView placeholderName = {};

    usize index = 0;
    usize prevPlaceholder = 0;

    while(nextPlaceholder(format.slice(index), &placeholderStart, &placeholderEnd, &placeholderName)) {
        
        StringView placeholderValue{};
        bool found = false;

        for(usize set = 0 ; set < placeholders.length() ; set++) {
            for(usize i = 0 ; i < placeholders[set].length() ; i++) {
                Placeholder& p = placeholders[set][i];
                if(p.key == placeholderName) {
                    placeholderValue = p.value;
                    found = true;
                    prevPlaceholder = set + 1;
                    break;
                }
            }
        }

        if(strict && !found) {
            NBL_DEBUG("Placeholder ", placeholderName, " not found");
            return nbl::Result::NOT_FOUND;
        }


        placeholderStart += index;
        placeholderEnd += index;

        if(placeholderStart - index > 0) {
            NBL_CHECK(writer.writeString(format.slice(index, placeholderStart - index)));
        }

        index = placeholderEnd + 1;

        if(!found) {
            NBL_DEBUG("Placeholder ", placeholderName, " not found");
            NBL_CHECK(writer.writeString("${"));
            NBL_CHECK(writer.writeString(placeholderName));
            NBL_CHECK(writer.writeChar('}'));
        } else {
            NBL_CHECK(writer.writeString(placeholderValue));
        }
    }

    if(index < format.length()) {
        NBL_CHECK(writer.writeString(format.slice(index)));
    }
    
    return nbl::Result::SUCCESS;
}
