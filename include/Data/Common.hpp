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

#pragma once

#include <Data/Database.hpp>
#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Math.hpp>

namespace nbl {

/**
 * \brief Enumeration for sorting options when looking up installations
 */
enum class SortBy {
    TITLE,
    LAST_PLAYED,
    ADDED,
    PLAY_COUNT,
    PLAY_TIME,
    RELEASE_DATE,
    RATING,
    ACHIEVEMENT_PROGRESS,
    _END
};

StringView getName(SortBy sortBy);
Result byName(StringView name, SortBy* outSortBy);


/**
 * \brief Enumeration for sort order when looking things up in the database
 */
enum class SortOrder {
    ASCENDING,
    DESCENDING
};

StringView getName(SortOrder sortOrder);
Result byName(StringView name, SortOrder* outSortOrder);


enum class LogoPosition : u32 {
    LEFT,
    CENTER,
    RIGHT,
    HIDDEN,
    INHERIT,
    _END
};

inline LogoPosition byId(u32 id) {
    if(id >= static_cast<u32>(LogoPosition::_END)) {
        return LogoPosition::LEFT;
    }
    return static_cast<LogoPosition>(id);
}


union InlineIcon {
    i64 encoded;
    i64 imageId;
    struct {
        char data[4];
        u8 length;
    } utf;

    static InlineIcon fromDataValue(DataValue value) {
        if(value.type == DataType::INTEGER) {
            return { .encoded = value.intValue };
        } else if(value.type == DataType::TEXT) {
            usize length = min(value.textValue.length, static_cast<usize>(4));
            InlineIcon out{};
            memcpy(out.utf.data, value.textValue.data, length);
            out.utf.length = length;
            return out;
        } else {
            return { .encoded = 0 };
        }
    }
};


struct SearchCriteria {
    ConstSpan<ConstSpan<i64>> tags;
    ConstSpan<i64> excludedTags;
    u64 minReleaseDate;
    u64 maxReleaseDate;
    f64 minRating;
    f64 maxRating;
    u64 minPlayTime;
    u64 maxPlayTime;
    SortBy sortBy;
    SortOrder sortOrder;
    u64 offset;
    u64 limit;
    StringView titleFilter;
    i64 gameId;
    i64 deviceId;
    u64 minAchievements;
    u64 maxAchievements;
    u64 minUnlockedAchievements;
    u64 maxUnlockedAchievements;
};

}
