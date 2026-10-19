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
 * \file Data/PluginModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp
 */

#include <Data/Model.hpp>
#include <Utility/StringBuilder.hpp>

nbl::Result nbl::getPluginFields(DatabaseConnection* connection, StringView pluginId, PluginFieldsCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT pluginId, fields, version FROM plugin_fields WHERE pluginId = ?;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindString(1, pluginId));

    return connection->execute(&_stmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {
        if(argc != 3) {
            return nbl::Result::INVALID_STATE;
        }
        return callback({
            .pluginId = values[0].textValue, 
            .fieldsJson = values[1].textValue, 
            .version = values[2].textValue
        });
    });
}


nbl::Result nbl::getPluginFieldsVersion(DatabaseConnection* connection, StringView pluginId, PluginFieldsCallback callback) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT pluginId, version FROM plugin_fields WHERE pluginId = ?;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindString(1, pluginId));

    return connection->execute(&_stmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {
        if(argc != 2) {
            return nbl::Result::INVALID_STATE;
        }
        return callback({
            .pluginId = values[0].textValue, 
            .version = values[1].textValue
        });
    });
}

nbl::Result nbl::setPluginFields(DatabaseConnection* connection, StringView pluginId, StringView fieldsJson, StringView version) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO plugin_fields (pluginId, fields, version) VALUES (?1, ?2, ?3) ON CONFLICT(pluginId) DO UPDATE SET fields = ?2, version = ?3;
        )", &_stmt));
    }

    NBL_CHECK(_stmt.bindString(1, pluginId));
    NBL_CHECK(_stmt.bindString(2, fieldsJson));
    NBL_CHECK(_stmt.bindString(3, version));

    return connection->execute(&_stmt);
}

nbl::Result nbl::prunePluginFields(DatabaseConnection* connection, JsonArray ids) {

    static nbl::PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM plugin_fields WHERE pluginId NOT IN (SELECT value FROM json_each(?1));
        )", &_stmt));
    }

    AllocationFrame frame(Arena::threadLocal());
    StringView json{};
    NBL_CHECK(StringBuilder(&frame).append(ids).buildNullTerminated(&json));

    NBL_CHECK(_stmt.bindString(1, json));

    return connection->execute(&_stmt);
}

