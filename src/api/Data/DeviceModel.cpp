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
 * \file Data/DeviceModel.cpp
 * \author jwall64
 * \brief Implementation of Data/DeviceModel.hpp
 */

#include <Data/Model.hpp>
#include <Data/DeviceModel.hpp>

nbl::Result nbl::HostDevice::add(DatabaseConnection* connection, HostDevice meta, i64* outId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO device (name) VALUES (?1);
        )", &_stmt));
    }
    _stmt.bindString(1, meta.name);
    return connection->execute(&_stmt, outId);
}

nbl::Result nbl::HostDevice::get(DatabaseConnection* connection, i64 id, DeviceCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT deviceId, name FROM device WHERE deviceId = ?1;
        )", &_stmt));
    }
    _stmt.bindInt(1, id);
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 2) return nbl::Result::IO_ERROR;
        return callback(HostDevice { .id = values[0].intValue, .name = values[1].textValue });
    });
}

nbl::Result nbl::HostDevice::remove(DatabaseConnection* connection, i64 id) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM device WHERE deviceId = ?1;
        )", &_stmt));
    }
    _stmt.bindInt(1, id);
    return connection->execute(&_stmt);
}

nbl::Result nbl::HostDevice::update(DatabaseConnection* connection, HostDevice meta) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE device SET name = ?1 WHERE deviceId = ?2;
        )", &_stmt));
    }
    _stmt.bindString(1, meta.name);
    _stmt.bindInt(2, meta.id);
    return connection->execute(&_stmt);
}


nbl::Result nbl::HostDevice::upsert(DatabaseConnection* connection, StringView hostname, i64* outId) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO device (name) VALUES (?1) ON CONFLICT(name) DO UPDATE SET name = name RETURNING deviceId;
        )", &_stmt));
    }
    _stmt.bindString(1, hostname);
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 1) return nbl::Result::IO_ERROR;
        *outId = values[0].intValue;
        return nbl::Result::SUCCESS;
    });

}
