#include <Data/Model.hpp>
#include <Platform/StringBuilder.hpp>

nbl::Result nbl::getPluginFields(DatabaseConnection* connection, StringView pluginId, PluginFieldsCallback callback) {

    static nbl::PreparedStatement _getFieldsStmt;
    if(!_getFieldsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT pluginId, fields, version FROM plugin_fields WHERE pluginId = ?;
        )", &_getFieldsStmt));
    }

    NBL_CHECK(_getFieldsStmt.bindString(1, pluginId));

    return connection->execute(&_getFieldsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {
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

    static nbl::PreparedStatement _getFieldsStmt;
    if(!_getFieldsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT pluginId, version FROM plugin_fields WHERE pluginId = ?;
        )", &_getFieldsStmt));
    }

    NBL_CHECK(_getFieldsStmt.bindString(1, pluginId));

    return connection->execute(&_getFieldsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {
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

    static nbl::PreparedStatement _setFieldsStmt;
    if(!_setFieldsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO plugin_fields (pluginId, fields, version) VALUES (?1, ?2, ?3) ON CONFLICT(pluginId) DO UPDATE SET fields = ?2, version = ?3;
        )", &_setFieldsStmt));
    }

    NBL_CHECK(_setFieldsStmt.bindString(1, pluginId));
    NBL_CHECK(_setFieldsStmt.bindString(2, fieldsJson));
    NBL_CHECK(_setFieldsStmt.bindString(3, version));

    return connection->execute(&_setFieldsStmt);
}

nbl::Result nbl::prunePluginFields(DatabaseConnection* connection, JsonArray ids) {

    static nbl::PreparedStatement _pruneFieldsStmt;
    if(!_pruneFieldsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM plugin_fields WHERE pluginId NOT IN (SELECT value FROM json_each(?1));
        )", &_pruneFieldsStmt));
    }

    ArenaFrame frame(Arena::threadLocal());
    StringView json{};
    NBL_CHECK(StringBuilder(&frame).append(ids).buildNullTerminated(&json));

    NBL_CHECK(_pruneFieldsStmt.bindString(1, json));

    return connection->execute(&_pruneFieldsStmt);
}

