#pragma once

#include <Data/Database.hpp>

namespace nbl {

enum class OnlineAccountStatus : u8 {
    NOT_LOGGED_IN,
    LOGGED_IN,
    EXPIRED
};

struct OnlineAccount;
typedef std::function_ref<Result(const OnlineAccount&)> OnlineAccountCallback;

struct OnlineAccount {
    i64 id;
    StringView fields;
    StringView provider;
    u64 updatedAt;
    StringView username;
    StringView icon;
    bool iconNearestScaling;
    OnlineAccountStatus status;

    static Result create(DatabaseConnection* connection, const OnlineAccount& account, i64* outAccountId);
    static Result update(DatabaseConnection* connection, const OnlineAccount& account);
    static Result updateStatus(DatabaseConnection* connection, i64 accountId, OnlineAccountStatus status);
    static Result remove(DatabaseConnection* connection, i64 accountId);
    static Result get(DatabaseConnection* connection, i64 accountId, OnlineAccountCallback callback);
    static Result getFirst(DatabaseConnection* connection, StringView provider, OnlineAccountCallback callback);
    static Result enumerate(DatabaseConnection* connection, OnlineAccountCallback callback);
    static Result enumerateForProvider(DatabaseConnection* connection, StringView provider, OnlineAccountCallback callback);
};

}
