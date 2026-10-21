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
 * \file Data/FilterModel.cpp
 * \author jwall64
 * \brief Implementation of Data/FilterModel.hpp
 */


#include <Data/FilterModel.hpp>

#include <Core/Math.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Utility/StringBuilder.hpp>
#include <Codec/Json.hpp>
#include <Data/Database.hpp>

namespace {

nbl::Result _serializeCriteria(const nbl::SearchCriteria& criteria, nbl::Writer writer) { 

    return nbl::JsonWriter(writer)
        .writeObject([&](auto obj) { obj
            .writeArray("tags", [&](auto sets) {
                for(const nbl::ConstSpan<i64>& span : criteria.tags) {
                    sets.writeArray([&](auto tags) {
                        for(i64 tagId : span) {
                            tags.add(tagId);
                        }
                        return nbl::Result::SUCCESS;
                    });
                }
                return nbl::Result::SUCCESS;
            })
            .writeArray("excludedTags", [&](auto tags) {
                for(i64 tag : criteria.excludedTags) {
                    tags.add(tag);
                }
                return nbl::Result::SUCCESS;
            })
            .add("minReleaseDate", criteria.minReleaseDate)
            .add("maxReleaseDate", criteria.maxReleaseDate)
            .add("minRating", criteria.minRating)
            .add("maxRating", criteria.maxRating)
            .add("minPlayTime", criteria.minPlayTime)
            .add("maxPlayTime", criteria.maxPlayTime)
            .add("sortBy", nbl::getName(criteria.sortBy))
            .add("sortOrder", nbl::getName(criteria.sortOrder))
            .add("offset", criteria.offset)
            .add("limit", criteria.limit)
            .add("titleFilter", criteria.titleFilter)
            .add("gameId", criteria.gameId)
            .add("minAchievements", criteria.minAchievements)
            .add("maxAchievements", criteria.maxAchievements)
            .add("minUnlockedAchievements", criteria.minUnlockedAchievements)
            .add("maxUnlockedAchievements", criteria.maxUnlockedAchievements);
            return nbl::Result::SUCCESS;
        });
}

nbl::Result _deserializeCriteria(nbl::StringView criteriaJson, nbl::AllocationFrame* frame, nbl::SearchCriteria* outCriteria) {
    
    nbl::JsonElement criteriaValue = {};
    nbl::log(nbl::LogLevel::DEBUG, "Parsing: ", criteriaJson, " as filter criteria");

    NBL_CHECK(nbl::parseJson(criteriaJson, frame, &criteriaValue));

    if(criteriaValue.type != nbl::JsonType::OBJECT) {
        nbl::log(nbl::LogLevel::ERROR, "Expected criteria JSON to be an object, but got ", criteriaValue.type);
        return nbl::Result::INVALID_ARGUMENT;
    }

    nbl::JsonObject& criteriaObject = criteriaValue.object;

    auto getChecked = [](const nbl::JsonObject& criteriaObj, nbl::StringView key, nbl::JsonType type, auto* outValue) -> nbl::Result {
        const nbl::JsonElement* value = criteriaObj.get(key);
        if(value) {
            if(value->type != type) {
                nbl::log(nbl::LogLevel::ERROR, "Expected criteria ", key, " to be of type ", type, " but got ", value->type);
                return nbl::Result::INVALID_ARGUMENT;
            }
            *outValue = value;
        }
        return nbl::Result::SUCCESS;
    };

    const nbl::JsonElement* tagsValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "tags", nbl::JsonType::ARRAY, &tagsValue));
    if(tagsValue && tagsValue->array.count > 0) {

        nbl::Span<nbl::ConstSpan<i64>> outSets{};

        frame->allocate(&outSets.data, tagsValue->array.count);

        for(const nbl::JsonElement& set : tagsValue->array) {
            if(set.type != nbl::JsonType::ARRAY) return nbl::Result::INVALID_ARGUMENT;

            nbl::Span<i64> outSet{};

            if(set.array.count > 0) {
                frame->allocate(&outSet.data, set.array.count);

                for(const nbl::JsonElement& tag : set.array) {
                    if(tag.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
                    outSet[outSet.length++] = tag.number.intValue;
                }
            }

            outSets[outSets.length++] = outSet;
        }

        outCriteria->tags = outSets;
    }

    const nbl::JsonElement* excludedTagsValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "excludedTags", nbl::JsonType::ARRAY, &excludedTagsValue));
    if(excludedTagsValue && excludedTagsValue->array.count > 0) {
        nbl::Span<i64> outTags{};
        frame->allocate(&outTags.data, excludedTagsValue->array.count);

        usize i = 0;
        for(const nbl::JsonElement& tag : excludedTagsValue->array) {
            if(tag.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
            outTags[outTags.length++] = tag.number.intValue;
        }

        outCriteria->excludedTags = outTags;
    }

    outCriteria->minReleaseDate = criteriaObject.getNumber("minReleaseDate").asUint();
    outCriteria->maxReleaseDate = criteriaObject.getNumber("maxReleaseDate").asUint();
    outCriteria->minRating = criteriaObject.getNumber("minRating").asFloat();
    outCriteria->maxRating = criteriaObject.getNumber("maxRating").asFloat();
    outCriteria->minPlayTime = criteriaObject.getNumber("minPlayTime").asUint();
    outCriteria->maxPlayTime = criteriaObject.getNumber("maxPlayTime").asUint();

    const nbl::JsonElement* sortByValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "sortBy", nbl::JsonType::STRING, &sortByValue));
    if(sortByValue) {
        NBL_CHECK(nbl::byName(sortByValue->string, &outCriteria->sortBy));
    }

    const nbl::JsonElement* sortOrderValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "sortOrder", nbl::JsonType::STRING, &sortOrderValue));
    if(sortOrderValue) {
        NBL_CHECK(nbl::byName(sortOrderValue->string, &outCriteria->sortOrder));
    }

    outCriteria->offset = criteriaObject.getNumber("offset").asUint();
    outCriteria->limit = criteriaObject.getNumber("limit").asUint();
    outCriteria->titleFilter = criteriaObject.getString("titleFilter");
    outCriteria->gameId = criteriaObject.getNumber("gameId").intValue;

    outCriteria->minAchievements = criteriaObject.getNumber("minAchievements").asUint();
    outCriteria->maxAchievements = criteriaObject.getNumber("maxAchievements").asUint();
    outCriteria->minUnlockedAchievements = criteriaObject.getNumber("minUnlockedAchievements").asUint();
    outCriteria->maxUnlockedAchievements = criteriaObject.getNumber("maxUnlockedAchievements").asUint();

    return nbl::Result::SUCCESS;
}

}


nbl::Result nbl::Filter::count(DatabaseConnection* connection, u64* numFilters) {

    static nbl::PreparedStatement _countFiltersStmt;
    if(!_countFiltersStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT COUNT(fId) FROM filter;
        )", &_countFiltersStmt));
    }

    return connection->execute(&_countFiltersStmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount == 0) return nbl::Result::IO_ERROR;
        *numFilters = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });

}

nbl::Result nbl::Filter::add(DatabaseConnection* connection, const Filter& meta, i64* outFilterId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO filter (name, tabId, icon, criteria, flags, priority) VALUES (?1, ?2, ?3, ?4, ?5, ?6);
        )", &_stmt));
    }

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    StringView criteriaJson;
    auto builder = nbl::StringBuilder(&frame);
    NBL_CHECK(_serializeCriteria(meta.criteria, builder));
    NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

    _stmt.bindAll(1, meta.name, meta.tabId, meta.icon.encoded, criteriaJson, meta.flags, meta.priority);

    return connection->execute(&_stmt, outFilterId);

}


nbl::Result nbl::Filter::remove(DatabaseConnection* connection, i64 filterId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM filter WHERE fId = ?1;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindInt(1, filterId));

    return connection->execute(&_stmt);
}

nbl::Result nbl::Filter::get(DatabaseConnection* connection, i64 filterId, FilterCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT fId, name, icon, criteria, flags FROM filter WHERE fId = ?1;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindInt(1, filterId));

    return connection->execute(&_stmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 5) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 5 columns in getFilter query result, but got ", columnCount);
            return nbl::Result::IO_ERROR;
        }

        nbl::AllocationFrame frame(nbl::Arena::threadLocal());
        Filter meta = {
            .filterId = columnValues[0].intValue,
            .name = columnValues[1].textValue,
            .icon = InlineIcon::fromDataValue(columnValues[2]),
            .flags = columnValues[4].uintValue(),
            .criteria = {},
        };
        
        NBL_CHECK(_deserializeCriteria(columnValues[3].textValue, &frame, &meta.criteria));
        
        return callback(meta);
    });
}

nbl::Result nbl::Filter::getCriteria(DatabaseConnection *connection, i64 filterId, FilterCriteriaCallback callback) {

    return get(connection, filterId, [&](const Filter& filter) {
        return callback(filter.criteria);
    });

}

nbl::Result nbl::Filter::setCriteria(DatabaseConnection* connection, i64 filterId, const SearchCriteria& criteria) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET criteria = ?1 WHERE fId = ?2;
        )", &_stmt));
    }

    {
        nbl::AllocationFrame frame(nbl::Arena::threadLocal());

        StringView criteriaJson;
        auto builder = nbl::StringBuilder(&frame);
        NBL_CHECK(_serializeCriteria(criteria, builder));
        NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

        _stmt.bindString(1, criteriaJson);
    }

    NBL_CHECK(_stmt.bindInt(2, filterId));

    return connection->execute(&_stmt);
}

nbl::Result nbl::Filter::update(DatabaseConnection* connection, const Filter& meta) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET name = ?1, tabId = ?2, icon = ?3, criteria = ?4, flags = ?5, priority = ?6 WHERE fId = ?7;
        )", &_stmt));
    }

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    StringView criteriaJson;
    auto builder = nbl::StringBuilder(&frame);
    NBL_CHECK(_serializeCriteria(meta.criteria, builder));
    NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

    _stmt.bindAll(1, meta.name, meta.tabId, meta.icon.encoded, criteriaJson, meta.flags, meta.priority, meta.filterId);
    return connection->execute(&_stmt);
}


nbl::Result nbl::Filter::enumerateForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT filter.fId, filter.name, filter.icon, filter.flags
FROM filter
WHERE filter.tabId = ?1
ORDER BY filter.priority DESC, filter.fId ASC
LIMIT ?2
OFFSET ?3;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, tabId, limit, offset));

    return connection->execute(&_stmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 4) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 4 columns in get filters for tab query result, but got ", columnCount);
            return nbl::Result::IO_ERROR;
        }

        nbl::Filter meta = {
            .filterId = columnValues[0].intValue, 
            .name = columnValues[1].textValue, 
            .icon = InlineIcon::fromDataValue(columnValues[2]),
            .flags = columnValues[3].uintValue(),
            .criteria = {}
        };

        return callback(meta);
    });

}
