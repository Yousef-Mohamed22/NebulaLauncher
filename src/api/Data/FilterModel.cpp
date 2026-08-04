// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file FilterModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to filters for installation tabs.
 */

#include <Data/Model.hpp>

#include <Core/Math.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Platform/StringBuilder.hpp>
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
                    });
                }
            })
            .writeArray("excludedTags", [&](auto tags) {
                for(i64 tag : criteria.excludedTags) {
                    tags.add(tag);
                }
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
            .add("gameId", criteria.gameId);
        });
}

nbl::Result _deserializeCriteria(nbl::StringView criteriaJson, nbl::ArenaFrame* frame, nbl::SearchCriteria* outCriteria) {
    
    nbl::JsonElement criteriaValue = {};
    nbl::log(nbl::LogLevel::DEBUG, "Parsing: ", criteriaJson, " as filter criteria");

    NBL_CHECK(nbl::parseJson(nbl::StringReader(criteriaJson), frame, &criteriaValue));

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

    const nbl::JsonElement* minReleaseDateValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minReleaseDate", nbl::JsonType::NUMBER, &minReleaseDateValue));
    if(minReleaseDateValue) outCriteria->minReleaseDate = minReleaseDateValue->number.asUint();

    const nbl::JsonElement* maxReleaseDateValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxReleaseDate", nbl::JsonType::NUMBER, &maxReleaseDateValue));
    if(maxReleaseDateValue) outCriteria->maxReleaseDate = maxReleaseDateValue->number.asUint();

    const nbl::JsonElement* minRatingValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minRating", nbl::JsonType::NUMBER, &minRatingValue));
    if(minRatingValue) outCriteria->minRating = minRatingValue->number.asFloat();

    const nbl::JsonElement* maxRatingValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxRating", nbl::JsonType::NUMBER, &maxRatingValue));
    if(maxRatingValue) outCriteria->maxRating = maxRatingValue->number.asFloat();

    const nbl::JsonElement* minPlayTimeValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minPlayTime", nbl::JsonType::NUMBER, &minPlayTimeValue));
    if(minPlayTimeValue) outCriteria->minPlayTime = minPlayTimeValue->number.asUint();

    const nbl::JsonElement* maxPlayTimeValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxPlayTime", nbl::JsonType::NUMBER, &maxPlayTimeValue));
    if(maxPlayTimeValue) outCriteria->maxPlayTime = maxPlayTimeValue->number.asUint();

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

    const nbl::JsonElement* offsetValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "offset", nbl::JsonType::NUMBER, &offsetValue));
    if(offsetValue) outCriteria->offset = offsetValue->number.asUint();

    const nbl::JsonElement* limitValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "limit", nbl::JsonType::NUMBER, &limitValue));
    if(limitValue) outCriteria->limit = limitValue->number.asUint();

    const nbl::JsonElement* titleFilterValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "titleFilter", nbl::JsonType::STRING, &titleFilterValue));
    if(titleFilterValue) outCriteria->titleFilter = titleFilterValue->string;

    const nbl::JsonElement* gameIdValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "gameId", nbl::JsonType::NUMBER, &gameIdValue));
    if(gameIdValue) outCriteria->gameId = gameIdValue->number.intValue;



    return nbl::Result::SUCCESS;
}

}


nbl::Result nbl::countFilters(DatabaseConnection* connection, u64* numFilters) {

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

nbl::Result nbl::addFilter(DatabaseConnection* connection, const FilterMeta& meta, i64* outFilterId) {

    static nbl::PreparedStatement _addFilterStmt;
    if(!_addFilterStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO filter (name, tabId, icon, colorizeIcon, criteria, multiLine, showTitles, useIcons, onlyInstalled, showInstallations, priority) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11);
        )", &_addFilterStmt));
    }

    _addFilterStmt.bindString(1, meta.name);
    _addFilterStmt.bindInt(2, meta.tabId);
    _addFilterStmt.bindString(3, meta.icon);
    _addFilterStmt.bindInt(4, meta.colorizeIcon ? 1 : 0);

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());

        StringView criteriaJson;
        auto builder = nbl::StringBuilder(&frame);
        NBL_CHECK(_serializeCriteria(meta.criteria, builder));
        NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

        _addFilterStmt.bindString(5, criteriaJson);
    }

    _addFilterStmt.bindInt(6, meta.multiLine ? 1 : 0);
    _addFilterStmt.bindInt(7, meta.showTitles ? 1 : 0);
    _addFilterStmt.bindInt(8, meta.useIcons ? 1 : 0);
    _addFilterStmt.bindInt(9, meta.onlyInstalled ? 1 : 0);
    _addFilterStmt.bindInt(10, meta.showInstallations ? 1 : 0);
    _addFilterStmt.bindInt(11, meta.priority);

    return connection->execute(&_addFilterStmt, outFilterId);

}


nbl::Result nbl::removeFilter(DatabaseConnection* connection, i64 filterId) {

    static nbl::PreparedStatement _removeFilterStmt;
    if(!_removeFilterStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM filter WHERE fId = ?1;
        )", &_removeFilterStmt));
    }

    NBL_CHECK(_removeFilterStmt.bindInt(1, filterId));

    return connection->execute(&_removeFilterStmt);
}

nbl::Result nbl::getFilter(DatabaseConnection* connection, i64 filterId, FilterCallback callback) {

    static nbl::PreparedStatement _getFilterStmt;
    if(!_getFilterStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT fId, name, icon, colorizeIcon, criteria, multiLine, showTitles, useIcons, onlyInstalled, showInstallations FROM filter WHERE fId = ?1;
        )", &_getFilterStmt));
    }

    NBL_CHECK(_getFilterStmt.bindInt(1, filterId));

    return connection->execute(&_getFilterStmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 10) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 10 columns in getFilter query result, but got ", columnCount);
            return nbl::Result::IO_ERROR;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        FilterMeta meta = {
            .filterId = columnValues[0].intValue,
            .name = columnValues[1].textValue,
            .icon = columnValues[2].textValue,
            .colorizeIcon = columnValues[3].intValue != 0,
            .multiLine = columnValues[5].intValue != 0,
            .showTitles = columnValues[6].intValue != 0,
            .useIcons = columnValues[7].intValue != 0,
            .onlyInstalled = columnValues[8].intValue != 0,
            .showInstallations = columnValues[9].intValue != 0,
            .criteria = {},
        };

        NBL_CHECK(_deserializeCriteria(columnValues[4].textValue, &frame, &meta.criteria));
        
        return callback(meta);
    });
}

nbl::Result nbl::setFilterCriteria(DatabaseConnection* connection, i64 filterId, const SearchCriteria& criteria) {

    static nbl::PreparedStatement _setFilterCriteriaStmt;
    if(!_setFilterCriteriaStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET criteria = ?1 WHERE fId = ?2;
        )", &_setFilterCriteriaStmt));
    }

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());

        StringView criteriaJson;
        auto builder = nbl::StringBuilder(&frame);
        NBL_CHECK(_serializeCriteria(criteria, builder));
        NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

        _setFilterCriteriaStmt.bindString(1, criteriaJson);
    }

    NBL_CHECK(_setFilterCriteriaStmt.bindInt(2, filterId));

    return connection->execute(&_setFilterCriteriaStmt);
}

nbl::Result nbl::updateFilter(DatabaseConnection* connection, const FilterMeta& meta) {

    static nbl::PreparedStatement _updateFilterStmt;
    if(!_updateFilterStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET name = ?1, tabId = ?2, icon = ?3, colorizeIcon = ?4, criteria = ?5, multiLine = ?6, showTitles = ?7, useIcons = ?8, onlyInstalled = ?9, showInstallations = ?10, priority = ?11 WHERE fId = ?12;
        )", &_updateFilterStmt));
    }

    _updateFilterStmt.bindString(1, meta.name);
    _updateFilterStmt.bindInt(2, meta.tabId);
    _updateFilterStmt.bindString(3, meta.icon);
    _updateFilterStmt.bindInt(4, meta.colorizeIcon ? 1 : 0);

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());

        StringView criteriaJson;
        auto builder = nbl::StringBuilder(&frame);
        NBL_CHECK(_serializeCriteria(meta.criteria, builder));
        NBL_CHECK(builder.buildNullTerminated(&criteriaJson));

        _updateFilterStmt.bindString(5, criteriaJson);
    }

    _updateFilterStmt.bindInt(6, meta.multiLine ? 1 : 0);
    _updateFilterStmt.bindInt(7, meta.showTitles ? 1 : 0);
    _updateFilterStmt.bindInt(8, meta.useIcons ? 1 : 0);
    _updateFilterStmt.bindInt(9, meta.onlyInstalled ? 1 : 0);
    _updateFilterStmt.bindInt(10, meta.showInstallations ? 1 : 0);
    _updateFilterStmt.bindInt(11, meta.priority);

    NBL_CHECK(_updateFilterStmt.bindInt(12, meta.filterId));

    return connection->execute(&_updateFilterStmt);
}

nbl::Result nbl::lookupInstallationsForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, InstallationLookupCallback callback) {

    NBL_CHECK(getFilter(connection, filterId, [&](const nbl::FilterMeta& meta) -> nbl::Result {

        SearchCriteria updatedCriteria = meta.criteria;
        updatedCriteria.offset = offset;
        updatedCriteria.deviceId = deviceId;

        if(limit > 0) {
            updatedCriteria.limit = updatedCriteria.limit > 0 ? nbl::min(limit, meta.criteria.limit) : limit;
        }

        return lookupInstallations(
            connection,
            updatedCriteria,
            callback
        );

     }));

     return nbl::Result::SUCCESS;
}


nbl::Result nbl::lookupGamesForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, GameLookupCallback callback) {

    NBL_CHECK(getFilter(connection, filterId, [&](const nbl::FilterMeta& meta) -> nbl::Result {

        SearchCriteria updatedCriteria = meta.criteria;
        updatedCriteria.offset = offset;
        updatedCriteria.deviceId = meta.onlyInstalled ? deviceId : 0;

        if(limit > 0) {
            updatedCriteria.limit = updatedCriteria.limit > 0 ? nbl::min(limit, meta.criteria.limit) : limit;
        }

        return lookupGames(
            connection,
            updatedCriteria,
            callback
        );

     }));

     return nbl::Result::SUCCESS;

}


nbl::Result nbl::getFilterCriteria(DatabaseConnection* connection, i64 filterId, FilterCriteriaCallback callback) {

    return getFilter(connection, filterId, [&](const FilterMeta& meta) {
        return callback(meta.criteria);
    });
}
