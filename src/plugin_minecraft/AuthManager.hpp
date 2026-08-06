#pragma once

#include <Core/Result.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Network/HttpClient.hpp>
#include <Plugin/Field.hpp>


namespace nbl::mc {

inline Array MC_AUTH_FIELDS {
    nbl::Field {
        .type = FieldType::STRING,
        .key = "username",
        .flags = FieldFlag::HIDDEN | 0
    },
    nbl::Field {
        .type = FieldType::STRING,
        .key = "uuid",
        .flags = FieldFlag::HIDDEN | 0
    },
    nbl::Field {
        .type = FieldType::STRING,
        .key = "access",
        .flags = FieldFlag::HIDDEN | FieldFlag::SECRET
    },
    nbl::Field {
        .type = FieldType::STRING,
        .key = "refresh",
        .flags = FieldFlag::HIDDEN | FieldFlag::SECRET
    },
    nbl::Field {
        .type = FieldType::DATE,
        .key = "expiry",
        .flags = FieldFlag::HIDDEN | 0
    }
};
inline constexpr StringView MC_CLIENT_ID="8f22712e-da45-4972-a441-3c0aa9bdb6b7";

Result login(PluginContext* context, Span<FieldValue> values, LoginTracker* tracker, void* userData);
Result refresh(PluginContext* context, const OnlineAccount& account, LoginTracker* tracker, void* userData);
Result logout(PluginContext* context, i64 accountId, void* userData);

}
