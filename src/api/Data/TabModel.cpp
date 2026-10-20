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
 * \file Data/TabModel.cpp
 * \author jwall64
 * \brief Implementation of Data/TabModel.hpp
 */


#include <Data/TabModel.hpp>
#include <Data/Model.hpp>
#include <Data/FilterModel.hpp>
#include <Core/Result.hpp>


nbl::Result nbl::Tab::count(DatabaseConnection* connection, u64* numTabs) {

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

nbl::Result nbl::Tab::add(DatabaseConnection* connection, const Tab& meta, i64* outTabId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tab (name, icon, flags) VALUES (?1, ?2, ?3);
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, meta.name, meta.icon.encoded, meta.flags));
    return connection->execute(&_stmt, outTabId);
}

nbl::Result nbl::Tab::update(DatabaseConnection* connection, const Tab& meta) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE tab SET name=?1, icon=?2, flags=?3  WHERE tabId=?4;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, meta.name, meta.icon.encoded, meta.flags, meta.tabId));
    return connection->execute(&_stmt);
}



nbl::Result nbl::Tab::remove(DatabaseConnection* connection, i64 tabId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM tab WHERE tabId = ?1;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindUint(1, tabId));
    return connection->execute(&_stmt);
}

nbl::Result nbl::Tab::enumerate(DatabaseConnection* connection, u64 offset, u64 limit, TabCallback callback) {

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
            Tab {
                .tabId = columnValues[0].intValue,
                .name = columnValues[1].textValue,
                .icon = InlineIcon::fromDataValue(columnValues[2]),
                .flags = columnValues[3].uintValue()
            }
        );
    });

}

nbl::Result nbl::Tab::get(DatabaseConnection* connection, i64 tabId, TabCallback callback) {

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
            Tab {
                .tabId = columnValues[0].intValue,
                .name = columnValues[1].textValue,
                .icon = InlineIcon::fromDataValue(columnValues[2]),
                .flags = columnValues[3].uintValue()
            }
        );
    });
}
