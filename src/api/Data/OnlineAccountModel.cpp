#include <Data/OnlineAccountModel.hpp>
#include <Data/Database.hpp>
#include "Core/Result.hpp"

namespace {

nbl::Result _processAccount(nbl::OnlineAccountCallback callback, i32 columns, nbl::DataValue* values, const char** names, i32 lastInsertRowId) {
    return callback({
        .id = values[0].intValue,
        .fields = values[1].textValue,
        .provider = values[2].textValue,
        .updatedAt = values[3].uintValue(),
        .username = values[4].textValue,
        .icon = values[5].textValue,
        .iconNearestScaling = values[6].intValue != 0,
        .status = static_cast<nbl::OnlineAccountStatus>(values[7].uintValue())
    });
}

}

nbl::Result nbl::OnlineAccount::create(DatabaseConnection *connection, const OnlineAccount &account, i64 *outAccountId) { 

    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO online_account
(fields, provider, updatedAt, username, icon, iconNearestScaling, status)
VALUES
(?1, ?2, ?3, ?4, ?5, ?6, ?7);)", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, account.fields, account.provider, account.updatedAt, account.username, account.icon, account.iconNearestScaling, static_cast<u32>(account.status)));

    return connection->execute(&_stmt, outAccountId);
}

nbl::Result nbl::OnlineAccount::update(DatabaseConnection *connection, const OnlineAccount &account) { 
    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE online_account SET 
fields = ?2, provider = ?3, updatedAt = ?4, username = ?5, icon = ?6, iconNearestScaling = ?7, status = ?8
WHERE oaId = ?1;)", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, account.id, account.fields, account.provider, account.updatedAt, account.username, account.icon, account.iconNearestScaling, static_cast<u32>(account.status)));

    return connection->execute(&_stmt);
}


nbl::Result nbl::OnlineAccount::updateStatus(DatabaseConnection* connection, i64 accountId, OnlineAccountStatus status) {
    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("UPDATE online_account SET status = ?2 WHERE oaId = ?1;", &_stmt));
    }

    NBL_CHECK(_stmt.bindAll(1, accountId, static_cast<u32>(status)));

    return connection->execute(&_stmt);
}

nbl::Result nbl::OnlineAccount::remove(DatabaseConnection* connection, i64 accountId) {

    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM online_account
WHERE oaId = ?1;)", &_stmt));
    }

    NBL_CHECK(_stmt.bind(1, accountId));
    return connection->execute(&_stmt);
}

nbl::Result nbl::OnlineAccount::get(DatabaseConnection* connection, i64 accountId, OnlineAccountCallback callback) {
    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT oaId, fields, provider, updatedAt, username, icon, iconNearestScaling, status
FROM online_account
WHERE oaId = ?1;)", &_stmt));
    }

    NBL_CHECK(_stmt.bind(1, accountId));
    return connection->execute(&_stmt, [&](i32 columns, nbl::DataValue* values, const char** names, i32 lastInsertRowId) {
        return _processAccount(callback, columns, values, names, lastInsertRowId);
    });
}

nbl::Result nbl::OnlineAccount::enumerate(DatabaseConnection* connection, OnlineAccountCallback callback) {
    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT oaId, fields, provider, updatedAt, username, icon, iconNearestScaling, status
FROM online_account;)", &_stmt));
    }

    return connection->execute(&_stmt, [&](i32 columns, nbl::DataValue* values, const char** names, i32 lastInsertRowId) {
        return _processAccount(callback, columns, values, names, lastInsertRowId);
    });
}

nbl::Result nbl::OnlineAccount::enumerateForProvider(DatabaseConnection* connection, StringView provider, OnlineAccountCallback callback) {
    PreparedStatement _stmt;
    if(!_stmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT oaId, fields, provider, updatedAt, username, icon, iconNearestScaling, status
FROM online_account
WHERE provider = ?1;)", &_stmt));
    }

    NBL_CHECK(_stmt.bind(1, provider));
    return connection->execute(&_stmt, [&](i32 columns, nbl::DataValue* values, const char** names, i32 lastInsertRowId) {
        return _processAccount(callback, columns, values, names, lastInsertRowId);
    });
}
