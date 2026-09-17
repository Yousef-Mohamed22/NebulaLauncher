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

nbl::Result nbl::formatPlaceholders(StringView format, StringBuilder *builder, Span<Placeholder> placeholders, bool strict) {

    usize placeholderStart = 0;
    usize placeholderEnd = 0;
    StringView placeholderName = {};

    usize index = 0;
    usize prevPlaceholder = 0;

    while(nextPlaceholder(format.tail(index), &placeholderStart, &placeholderEnd, &placeholderName)) {
        
        StringView placeholderValue{};
        bool found = false;

        for(usize i = 0 ; i < placeholders.length ; i++) {
            usize realI = (i + prevPlaceholder) % placeholders.length;
            Placeholder& p = placeholders[realI];

            if(p.key == placeholderName) {
                placeholderValue = p.value;
                found = true;
                prevPlaceholder = i + 1;
                break;
            }
        }

        if(strict && !found) {
            nbl::log(nbl::LogLevel::DEBUG, "Placeholder ", placeholderName, " not found");
            return nbl::Result::NOT_FOUND;
        }


        placeholderStart += index;
        placeholderEnd += index;

        if(placeholderStart - index > 0) {
            NBL_CHECK(builder->appendChecked(format.slice(index, placeholderStart - index)));
        }

        index = placeholderEnd + 1;

        if(!found) {
            nbl::log(nbl::LogLevel::DEBUG, "Placeholder ", placeholderName, " not found");
            NBL_CHECK(builder->append("${").append(placeholderName).appendChecked('}'));
        } else {
            NBL_CHECK(builder->appendChecked(placeholderValue));
        }
    }

    if(index < format.length) {
        NBL_CHECK(builder->appendChecked(format.tail(index)));
    }
    
    return nbl::Result::SUCCESS;
}
