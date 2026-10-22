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
