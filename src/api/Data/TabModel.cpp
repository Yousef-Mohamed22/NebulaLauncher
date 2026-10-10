// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file TabModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to tab management
 */

#include <Data/Model.hpp>
#include "Core/Result.hpp"


nbl::Result nbl::countTabs(DatabaseConnection* connection, u64* numTabs) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT COUNT(tId) FROM tab;
        )", &_stmt));
    }

    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount == 0) return nbl::Result::IO_ERROR;

        *numTabs = values[0].uintValue();
        return nbl::Result::SUCCESS;

    });

}

nbl::Result nbl::addTab(DatabaseConnection* connection, const TabMeta& meta, i64* outTabId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tab (name, icon, flags) VALUES (?1, ?2, ?3);
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, meta.name, meta.icon, meta.flags));
    return connection->execute(&_stmt, outTabId);
}

nbl::Result nbl::updateTab(DatabaseConnection* connection, const TabMeta& meta) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE tab SET name=?1, icon=?2, flags=?3  WHERE tabId=?4;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, meta.name, meta.icon, meta.flags, meta.tabId));
    return connection->execute(&_stmt);
}



nbl::Result nbl::removeTab(DatabaseConnection* connection, i64 tabId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM tab WHERE tabId = ?1;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindUint(1, tabId));
    return connection->execute(&_stmt);
}

nbl::Result nbl::enumerateTabs(DatabaseConnection* connection, u64 offset, u64 limit, TabCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tabId, name, icon, flags FROM tab LIMIT ?1 OFFSET ?2;
        )", &_stmt));
    }

    _stmt.bindUint(1, limit);
    _stmt.bindUint(2, offset);

    return connection->execute(&_stmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 4) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 4 columns in enumerate tabs query result, but got ", columnCount);
            return nbl::Result::IO_ERROR;
        }

        return callback(
            TabMeta {
                .tabId = columnValues[0].intValue,
                .name = columnValues[1].textValue,
                .icon = columnValues[2].textValue,
                .flags = columnValues[3].uintValue()
            }
        );
    });

}

nbl::Result nbl::getTabMeta(DatabaseConnection* connection, i64 tabId, TabCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tabId, name, icon, flags FROM tab WHERE tabId=?1;
        )", &_stmt));
    }

    _stmt.bindInt(1, tabId);

    return connection->execute(&_stmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 4) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 4 columns in enumerate tabs query result, but got ", columnCount);
            return nbl::Result::IO_ERROR;
        }

        return callback( 
            TabMeta {
                .tabId = columnValues[0].intValue,
                .name = columnValues[1].textValue,
                .icon = columnValues[2].textValue,
                .flags = columnValues[3].uintValue()
            }
        );
    });
}

nbl::Result nbl::getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback) {

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

        nbl::FilterMeta meta = {
            .filterId = columnValues[0].intValue, 
            .name = columnValues[1].textValue, 
            .icon = columnValues[2].textValue, 
            .flags = columnValues[3].uintValue(),
            .criteria = {}
        };

        return callback(meta);
    });

}
