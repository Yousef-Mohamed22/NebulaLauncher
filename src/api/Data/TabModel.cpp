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

nbl::Result nbl::addTab(DatabaseConnection* connection, const TabMeta& meta, i64* outTabId) {

    static nbl::PreparedStatement _addTabStmt;
    if(!_addTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tab (name, icon, colorizeIcon, hidden) VALUES (?1, ?2, ?3, ?4);
        )", &_addTabStmt));
    }

    NBL_CHECK(_addTabStmt.bindString(1, meta.name));
    NBL_CHECK(_addTabStmt.bindString(2, meta.icon));
    NBL_CHECK(_addTabStmt.bindInt(3, meta.colorizeIcon ? 1 : 0));
    NBL_CHECK(_addTabStmt.bindInt(4, meta.hidden ? 1 : 0));

    return connection->execute(&_addTabStmt, nullptr, nullptr, outTabId);
}

nbl::Result nbl::updateTab(DatabaseConnection* connection, const TabMeta& meta) {

    static nbl::PreparedStatement _updateTabStmt;
    if(!_updateTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE tab SET name=?1, icon=?2, colorizeIcon=?3, hidden=?4 WHERE tabId=?5;
        )", &_updateTabStmt));
    }

    NBL_CHECK(_updateTabStmt.bindString(1, meta.name));
    NBL_CHECK(_updateTabStmt.bindString(2, meta.icon));
    NBL_CHECK(_updateTabStmt.bindInt(3, meta.colorizeIcon ? 1 : 0));
    NBL_CHECK(_updateTabStmt.bindInt(4, meta.hidden ? 1 : 0));
    NBL_CHECK(_updateTabStmt.bindInt(5, meta.tabId));

    return connection->execute(&_updateTabStmt, nullptr, nullptr, nullptr);
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
SELECT tabId, name, icon, colorizeIcon, hidden FROM tab;
        )", &_enumerateTabsStmt));
    }

    struct _EnumerateTabsQueryData {
        TabCallback callback;
        void* userData;
    } queryData{ callback, userData };

    return connection->execute(&_enumerateTabsStmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 5) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 5 columns in enumerate tabs query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _EnumerateTabsQueryData* queryData = static_cast<_EnumerateTabsQueryData*>(userData);
    
        return queryData->callback(
            queryData->userData,
            TabMeta {
                .tabId = columnValues[0].intValue,
                .name = columnValues[1].textValue,
                .icon = columnValues[2].textValue,
                .colorizeIcon = columnValues[3].intValue != 0,
                .hidden = columnValues[4].intValue != 0
            }
        );
    }, &queryData);

}

nbl::Result nbl::getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback, void* userData) {

    static nbl::PreparedStatement _getFiltersForTabStmt;
    if(!_getFiltersForTabStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT filter.fId, filter.name, filter.icon, filter.colorizeIcon, filter.multiLine, filter.showTitles, filter.useIcons
FROM filter
WHERE filter.tabId = ?1
ORDER BY filter.priority DESC, filter.fId ASC
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

        if(columnCount != 7) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 7 columns in get filters for tab query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _GetFiltersForTabQueryData* queryData = static_cast<_GetFiltersForTabQueryData*>(userData);
    
        nbl::FilterMeta meta = {
            .filterId = columnValues[0].intValue, 
            .name = columnValues[1].textValue, 
            .icon = columnValues[2].textValue, 
            .colorizeIcon = columnValues[3].intValue != 0,
            .multiLine = columnValues[4].intValue != 0,
            .showTitles = columnValues[5].intValue != 0,
            .useIcons = columnValues[6].intValue != 0,
            .criteria = {}
        };

        return queryData->callback(
            queryData->userData, 
            meta
        );
    }, &queryData);

}
