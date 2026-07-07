// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file FilterModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to filters for installation tabs.
 */

#include <Data/Model.hpp>

#include <Core/Math.hpp>
#include <Platform/StringBuilder.hpp>
#include "Core/Result.hpp"

namespace {

nbl::Result _serializeCriteria(const nbl::InstallationSearchCriteria& criteria, nbl::ArenaFrame* frame, nbl::JsonObject* outObject) {

    return nbl::JsonObject::Builder(frame)
        .add("tags", criteria.tags)
        .add("excludedTags", criteria.excludedTags)
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
        .build(nullptr, outObject);

}

nbl::Result _deserializeCriteria(nbl::StringView criteriaJson, nbl::ArenaFrame* frame, nbl::InstallationSearchCriteria* outCriteria) {
    
    nbl::JsonValue criteriaValue;
    NBL_CHECK(nbl::parseJson(nbl::StringReader(criteriaJson), frame, &criteriaValue, true));

    nbl::log(nbl::LogLevel::DEBUG, "Parsing: ", criteriaJson, " as filter criteria");

    if(criteriaValue.type != nbl::JsonType::OBJECT) {
        nbl::log(nbl::LogLevel::ERROR, "Expected criteria JSON to be an object, but got ", criteriaValue.type);
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    nbl::JsonObject& criteriaObject = criteriaValue.object;

    auto getChecked = [](const nbl::JsonObject& criteriaObj, nbl::StringView key, nbl::JsonType type, auto* outValue) -> nbl::Result {
        const nbl::JsonValue* value = criteriaObj.get(key);
        if(value) {
            if(value->type != type) {
                nbl::log(nbl::LogLevel::ERROR, "Expected criteria ", key, " to be of type ", type, " but got ", value->type);
                return nbl::error::ILLEGAL_ARGUMENT;
            }
            *outValue = value;
        }
        return nbl::Result::SUCCESS;
    };

    const nbl::JsonValue* tagsValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "tags", nbl::JsonType::ARRAY, &tagsValue));
    if(tagsValue) outCriteria->tags = tagsValue->array;

    const nbl::JsonValue* excludedTagsValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "excludedTags", nbl::JsonType::ARRAY, &excludedTagsValue));
    if(excludedTagsValue) outCriteria->excludedTags = excludedTagsValue->array;

    const nbl::JsonValue* minReleaseDateValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minReleaseDate", nbl::JsonType::NUMBER, &minReleaseDateValue));
    if(minReleaseDateValue) outCriteria->minReleaseDate = minReleaseDateValue->number.asUint();

    const nbl::JsonValue* maxReleaseDateValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxReleaseDate", nbl::JsonType::NUMBER, &maxReleaseDateValue));
    if(maxReleaseDateValue) outCriteria->maxReleaseDate = maxReleaseDateValue->number.asUint();

    const nbl::JsonValue* minRatingValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minRating", nbl::JsonType::NUMBER, &minRatingValue));
    if(minRatingValue) outCriteria->minRating = minRatingValue->number.asFloat();

    const nbl::JsonValue* maxRatingValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxRating", nbl::JsonType::NUMBER, &maxRatingValue));
    if(maxRatingValue) outCriteria->maxRating = maxRatingValue->number.asFloat();

    const nbl::JsonValue* minPlayTimeValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "minPlayTime", nbl::JsonType::NUMBER, &minPlayTimeValue));
    if(minPlayTimeValue) outCriteria->minPlayTime = minPlayTimeValue->number.asUint();

    const nbl::JsonValue* maxPlayTimeValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "maxPlayTime", nbl::JsonType::NUMBER, &maxPlayTimeValue));
    if(maxPlayTimeValue) outCriteria->maxPlayTime = maxPlayTimeValue->number.asUint();

    const nbl::JsonValue* sortByValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "sortBy", nbl::JsonType::STRING, &sortByValue));
    if(sortByValue) {
        NBL_CHECK(nbl::byName(sortByValue->string, &outCriteria->sortBy));
    }

    const nbl::JsonValue* sortOrderValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "sortOrder", nbl::JsonType::STRING, &sortOrderValue));
    if(sortOrderValue) {
        NBL_CHECK(nbl::byName(sortOrderValue->string, &outCriteria->sortOrder));
    }

    const nbl::JsonValue* offsetValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "offset", nbl::JsonType::NUMBER, &offsetValue));
    if(offsetValue) outCriteria->offset = offsetValue->number.asUint();

    const nbl::JsonValue* limitValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "limit", nbl::JsonType::NUMBER, &limitValue));
    if(limitValue) outCriteria->limit = limitValue->number.asUint();

    const nbl::JsonValue* titleFilterValue = nullptr;
    NBL_CHECK(getChecked(criteriaObject, "titleFilter", nbl::JsonType::STRING, &titleFilterValue));
    if(titleFilterValue) outCriteria->titleFilter = titleFilterValue->string;

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

        if(columnCount == 0) return nbl::error::IO_ERROR;

        *numFilters = values[0].uintValue();
        return nbl::Result::SUCCESS;

    });

}

nbl::Result nbl::addFilter(DatabaseConnection* connection, const FilterMeta& meta, i64* outFilterId) {

    static nbl::PreparedStatement _addFilterStmt;
    if(!_addFilterStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO filter (name, tabId, icon, colorizeIcon, criteria, multiLine, showTitles, useIcons) VALUES (?1, ?2 ?3, ?4, ?5, ?6, ?7, ?8);
        )", &_addFilterStmt));
    }

    _addFilterStmt.bindString(1, meta.name);
    _addFilterStmt.bindInt(2, meta.tabId);
    _addFilterStmt.bindString(3, meta.icon);
    _addFilterStmt.bindInt(4, meta.colorizeIcon ? 1 : 0);

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::JsonObject criteriaObject;
        NBL_CHECK(_serializeCriteria(meta.criteria, &frame, &criteriaObject));

        StringView criteriaJson;
        NBL_CHECK(nbl::StringBuilder(&frame).append(criteriaObject).buildNullTerminated(&criteriaJson));
        _addFilterStmt.bindString(5, criteriaJson);
    }

    _addFilterStmt.bindInt(6, meta.multiLine ? 1 : 0);
    _addFilterStmt.bindInt(7, meta.showTitles ? 1 : 0);
    _addFilterStmt.bindInt(8, meta.useIcons ? 1 : 0);

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
SELECT fId, name, icon, colorizeIcon, criteria, multiLine, showTitles, useIcons FROM filter WHERE fId = ?1;
        )", &_getFilterStmt));
    }

    NBL_CHECK(_getFilterStmt.bindInt(1, filterId));

    return connection->execute(&_getFilterStmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 8) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 8 columns in getFilter query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
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
            .criteria = {},
        };

        NBL_CHECK(_deserializeCriteria(columnValues[4].textValue, &frame, &meta.criteria));
        
        return callback(meta);
    });
}

nbl::Result nbl::setFilterCriteria(DatabaseConnection* connection, i64 filterId, const InstallationSearchCriteria& criteria) {

    static nbl::PreparedStatement _setFilterCriteriaStmt;
    if(!_setFilterCriteriaStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET criteria = ?1 WHERE fId = ?2;
        )", &_setFilterCriteriaStmt));
    }

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::JsonObject criteriaObject;
        NBL_CHECK(_serializeCriteria(criteria, &frame, &criteriaObject));

        StringView criteriaJson;
        NBL_CHECK(nbl::StringBuilder(&frame).append(criteriaObject).buildNullTerminated(&criteriaJson));
        _setFilterCriteriaStmt.bindString(1, criteriaJson);
    }

    NBL_CHECK(_setFilterCriteriaStmt.bindInt(2, filterId));

    return connection->execute(&_setFilterCriteriaStmt);
}

nbl::Result nbl::updateFilter(DatabaseConnection* connection, const FilterMeta& meta) {

    static nbl::PreparedStatement _updateFilterStmt;
    if(!_updateFilterStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
UPDATE filter SET name = ?1, tabId = ?2, icon = ?3, colorizeIcon = ?4, criteria = ?5, multiLine = ?6, showTitles = ?7, useIcons = ?8, WHERE fId = ?9;
        )", &_updateFilterStmt));
    }

    _updateFilterStmt.bindString(1, meta.name);
    _updateFilterStmt.bindInt(2, meta.tabId);
    _updateFilterStmt.bindString(3, meta.icon);
    _updateFilterStmt.bindInt(4, meta.colorizeIcon ? 1 : 0);

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::JsonObject criteriaObject;
        NBL_CHECK(_serializeCriteria(meta.criteria, &frame, &criteriaObject));

        StringView criteriaJson;
        NBL_CHECK(nbl::StringBuilder(&frame).append(criteriaObject).buildNullTerminated(&criteriaJson));
        _updateFilterStmt.bindString(5, criteriaJson);
    }

    _updateFilterStmt.bindInt(6, meta.multiLine ? 1 : 0);
    _updateFilterStmt.bindInt(7, meta.showTitles ? 1 : 0);
    _updateFilterStmt.bindInt(8, meta.useIcons ? 1 : 0);

    NBL_CHECK(_updateFilterStmt.bindInt(9, meta.filterId));

    return connection->execute(&_updateFilterStmt);
}

nbl::Result nbl::lookupInstallationsForFilter(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, InstallationLookupCallback callback) {

    NBL_CHECK(getFilter(connection, tabId, [&](const nbl::FilterMeta& meta) -> nbl::Result {

        InstallationSearchCriteria updatedCriteria = meta.criteria;
        updatedCriteria.offset = offset;

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


nbl::Result nbl::getFilterCriteria(DatabaseConnection* connection, i64 filterId, FilterCriteriaCallback callback) {

    return getFilter(connection, filterId, [&](const FilterMeta& meta) {
        return callback(meta.criteria);
    });
}
