// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file DeviceModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to tab management
 */

#include <Data/Model.hpp>


nbl::Result nbl::addDevice(DatabaseConnection* connection, DeviceMeta meta, i64* outId) {

    static nbl::PreparedStatement _addDeviceStmt;
    if(!_addDeviceStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO device (name) VALUES (?1);
        )", &_addDeviceStmt));
    }
    _addDeviceStmt.bindString(1, meta.name);
    return connection->execute(&_addDeviceStmt, outId);
}

nbl::Result nbl::getDevice(DatabaseConnection* connection, i64 id, DeviceCallback callback) {

    static nbl::PreparedStatement _getDeviceStmt;
    if(!_getDeviceStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT deviceId, name FROM device WHERE deviceId = ?1;
        )", &_getDeviceStmt));
    }
    _getDeviceStmt.bindInt(1, id);
    return connection->execute(&_getDeviceStmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 2) return nbl::Result::IO_ERROR;
        return callback(DeviceMeta { .id = values[0].intValue, .name = values[1].textValue });
    });
}

nbl::Result nbl::removeDevice(DatabaseConnection* connection, i64 id) {

    static nbl::PreparedStatement _removeDeviceStmt;
    if(!_removeDeviceStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM device WHERE deviceId = ?1;
        )", &_removeDeviceStmt));
    }
    _removeDeviceStmt.bindInt(1, id);
    return connection->execute(&_removeDeviceStmt);
}

nbl::Result nbl::updateDevice(DatabaseConnection* connection, DeviceMeta meta) {

    static nbl::PreparedStatement _updateDeviceStmt;
    if(!_updateDeviceStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE device SET name = ?1 WHERE deviceId = ?2;
        )", &_updateDeviceStmt));
    }
    _updateDeviceStmt.bindString(1, meta.name);
    _updateDeviceStmt.bindInt(2, meta.id);
    return connection->execute(&_updateDeviceStmt);
}
