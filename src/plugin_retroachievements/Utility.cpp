#include "Utility.hpp"

#include <Codec/Json.hpp>
#include <Platform/Logging.hpp>
#include <Platform/PlatformDefines.hpp>
#include <Platform/Time.hpp>


nbl::Result nbl::ra::parseTitle(StringView title, std::function_ref<void(StringView)> tagConsumer, StringView* gameTitle, StringView* subsetName) {

    enum State : i32 {
        S_TAGS,
        S_TAG_NAME,
        S_TITLE,
        S_TITLE_SPACE,
        S_SUBSET,
        S_SUBSET_NAME
    };

    usize begin = 0;
    usize length = 0;

    static StringView _subsetText = "Subset - ";

    i32 state = S_TAGS;
    char c;
    for(usize i = 0 ; i < title.length ; i++) {

        c = title[i];

        switch(state) {
        case S_TAGS:
            if(c == '~') {
                state = S_TAG_NAME;
                begin = i + 1;
                length = 0;
            } else if(c != ' ') {
                state = S_TITLE;
                begin = i;
                length = 1;
            }
            break;
        case S_TAG_NAME:
            if(c == '~') {
                state = S_TAGS;
                tagConsumer(title.slice(begin, length));
            } else {
                length++;
            }
            break;
        case S_TITLE: 
            if(c == ' ') {
                state = S_TITLE_SPACE;
            } else {
                length++;
            }
            break;
        case S_TITLE_SPACE:
            if(c == '[') {
                state = S_SUBSET;
                *gameTitle = title.slice(begin, length);
                length = 0;
            } else {
                state = S_TITLE;
                length += 2;
            }
            break;
        case S_SUBSET:
            if(length == _subsetText.length) {
                state = S_SUBSET_NAME;
                begin = i;
                length = 0;
            } 
            else if(c != _subsetText[length++]) {
                state = S_TITLE;
                length += gameTitle->length + 2;
            }
            break;
        case S_SUBSET_NAME:
            length++;
        }
    }

    if(state == S_SUBSET_NAME) {
        if(c == ']') {
            *subsetName = title.slice(begin, length);
        } else {
            state = S_TITLE;
            length += gameTitle->length;
        } 
    }

    if(state == S_TITLE) {
        *gameTitle = title.slice(begin, length);
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::ra::parseDate(StringView date, u64* outTime) { 

    JsonNumber current{};
    u32 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;

    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(0, 4).lstrip('0', 1)), &current));
    year = current.asInt();
    
    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(5, 2).lstrip('0', 1)), &current));
    month = current.asUint();

    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(8, 2).lstrip('0', 1)), &current));
    day = current.asUint();

    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(11, 2).lstrip('0', 1)), &current));
    hour = current.asUint();

    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(14, 2).lstrip('0', 1)), &current));
    minute = current.asUint();

    NBL_CHECK(nbl::parseJsonNumber(StringReader(date.slice(17, 2).lstrip('0', 1)), &current));
    second = current.asUint();

    return utcFromComponents(year, month, day, hour, minute, second, outTime);
}
