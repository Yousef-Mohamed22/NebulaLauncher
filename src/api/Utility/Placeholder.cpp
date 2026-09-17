#include <Utility/Placeholder.hpp>

bool nbl::nextPlaceholder(StringView str, usize* placeholderStart, usize* placeholderEnd, StringView* placeholder) {

    usize depth = 0;
    usize start = 0;
    for(usize i = 0 ; i < str.length ; i++) {
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
                    *placeholder = StringView(str.data + start + 2, i - start - 2);
                    return true;
                }
                break;
        }
    }

    return false;
}

nbl::Result nbl::formatPlaceholders(StringView format, Writer writer, Span<Span<Placeholder>> placeholders, bool strict) {

    if(!strict && placeholders.length == 0) {
        return writer.writeString(format);
    }

    usize placeholderStart = 0;
    usize placeholderEnd = 0;
    StringView placeholderName = {};

    usize index = 0;
    usize prevPlaceholder = 0;

    while(nextPlaceholder(format.tail(index), &placeholderStart, &placeholderEnd, &placeholderName)) {
        
        StringView placeholderValue{};
        bool found = false;

        for(usize set = 0 ; set < placeholders.length ; set++) {
            for(usize i = 0 ; i < placeholders[set].length ; i++) {
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
            nbl::log(nbl::LogLevel::DEBUG, "Placeholder ", placeholderName, " not found");
            return nbl::Result::NOT_FOUND;
        }


        placeholderStart += index;
        placeholderEnd += index;

        if(placeholderStart - index > 0) {
            NBL_CHECK(writer.writeString(format.slice(index, placeholderStart - index)));
        }

        index = placeholderEnd + 1;

        if(!found) {
            nbl::log(nbl::LogLevel::DEBUG, "Placeholder ", placeholderName, " not found");
            NBL_CHECK(writer.writeString("${"));
            NBL_CHECK(writer.writeString(placeholderName));
            NBL_CHECK(writer.writeChar('}'));
        } else {
            NBL_CHECK(writer.writeString(placeholderValue));
        }
    }

    if(index < format.length) {
        NBL_CHECK(writer.writeString(format.tail(index)));
    }
    
    return nbl::Result::SUCCESS;
}
