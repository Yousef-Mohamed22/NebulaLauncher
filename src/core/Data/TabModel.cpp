// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file TabModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to tab management
 */

#include <Data/Model.hpp>


nbl::Result nbl::countTabs(DatabaseConnection* connection, u64* numTabs) {

    static nbl::PreparedStatement _countFiltersStmt;
    if(!_countFiltersStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT COUNT(tId) FROM tab;
        )", &_countFiltersStmt));
    }

    return connection->execute(&_countFiltersStmt, [](void* userData, i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount == 0) return nbl::error::IO_ERROR;

        *static_cast<u64*>(userData) = values[0].uintValue();
        return nbl::Result::SUCCESS;

    }, numTabs);

}

nbl::Result nbl::addTab(DatabaseConnection* connection, StringView name, StringView icon, bool hidden, i64* outTabId) {

    static nbl::PreparedStatement _addTabStmt;
    if(!_addTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tab (name, icon, hidden) VALUES (?1, ?2, ?3);
        )", &_addTabStmt));
    }

    NBL_CHECK(_addTabStmt.bindString(1, name));
    NBL_CHECK(_addTabStmt.bindString(2, icon));
    NBL_CHECK(_addTabStmt.bindInt(3, hidden ? 1 : 0));

    return connection->execute(&_addTabStmt, nullptr, nullptr, outTabId);
}

nbl::Result nbl::removeTab(DatabaseConnection* connection, i64 tabId) {

    static nbl::PreparedStatement _removeTabStmt;
    if(!_removeTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM tab WHERE tabId = ?1;
        )", &_removeTabStmt));
    }

    NBL_CHECK(_removeTabStmt.bindUint(1, tabId));
    return connection->execute(&_removeTabStmt, nullptr, nullptr);
}

nbl::Result nbl::enumerateTabs(DatabaseConnection* connection, TabCallback callback, void* userData) {

    static nbl::PreparedStatement _enumerateTabsStmt;
    if(!_enumerateTabsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tabId, name, icon, hidden FROM tab;
        )", &_enumerateTabsStmt));
    }

    struct _EnumerateTabsQueryData {
        TabCallback callback;
        void* userData;
    } queryData{ callback, userData };

    return connection->execute(&_enumerateTabsStmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 4) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 4 columns in enumerate tabs query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _EnumerateTabsQueryData* queryData = static_cast<_EnumerateTabsQueryData*>(userData);
    
        return queryData->callback(
            queryData->userData,
            columnValues[0].intValue,
            columnValues[1].textValue,
            columnValues[2].textValue,
            columnValues[3].intValue != 0
        );
    }, &queryData);

}

nbl::Result nbl::addFilterToTab(DatabaseConnection* connection, i64 tabId, i64 filterId, i64 priority) {

    static nbl::PreparedStatement _addFilterToTabStmt;
    if(!_addFilterToTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tab_filter (tabId, fId, priority) VALUES (?1, ?2, ?3);
        )", &_addFilterToTabStmt));
    }

    NBL_CHECK(_addFilterToTabStmt.bindUint(1, tabId));
    NBL_CHECK(_addFilterToTabStmt.bindUint(2, filterId));
    NBL_CHECK(_addFilterToTabStmt.bindUint(3, priority));

    return connection->execute(&_addFilterToTabStmt, nullptr, nullptr);
}

nbl::Result nbl::removeFilterFromTab(DatabaseConnection* connection, i64 tabId, i64 filterId) {

    static nbl::PreparedStatement _removeFilterFromTabStmt;
    if(!_removeFilterFromTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM tab_filter WHERE tabId = ?1 AND fId = ?2;
        )", &_removeFilterFromTabStmt));
    }

    NBL_CHECK(_removeFilterFromTabStmt.bindUint(1, tabId));
    NBL_CHECK(_removeFilterFromTabStmt.bindUint(2, filterId));

    return connection->execute(&_removeFilterFromTabStmt, nullptr, nullptr);
}

nbl::Result nbl::getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback, void* userData) {

    static nbl::PreparedStatement _getFiltersForTabStmt;
    if(!_getFiltersForTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT filter.fId, filter.name, filter.icon, filter.multiLine
FROM filter
JOIN tab_filter ON filter.fId = tab_filter.fId
WHERE tab_filter.tabId = ?1
ORDER BY tab_filter.priority DESC, filter.fId ASC
LIMIT ?2
OFFSET ?3;
        )", &_getFiltersForTabStmt));
    }

    NBL_CHECK(_getFiltersForTabStmt.bindInt(1, tabId));
    NBL_CHECK(_getFiltersForTabStmt.bindUint(2, limit));
    NBL_CHECK(_getFiltersForTabStmt.bindUint(3, offset));
    
    struct _GetFiltersForTabQueryData {
        FilterCallback callback;
        void* userData;
    } queryData{ callback, userData };


    return connection->execute(&_getFiltersForTabStmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 4) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 5 columns in get filters for tab query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _GetFiltersForTabQueryData* queryData = static_cast<_GetFiltersForTabQueryData*>(userData);
    
        return queryData->callback(
            queryData->userData, 
            columnValues[0].intValue, 
            columnValues[1].textValue, 
            columnValues[2].textValue, 
            {}, 
            columnValues[3].intValue != 0
        );
    }, &queryData);

}
