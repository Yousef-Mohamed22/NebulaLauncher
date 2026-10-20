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

#include <Core/String.hpp>
#include <Data/Model.hpp>

namespace nbl {


struct Tag;
typedef std::function<Result(const Tag&)> TagCallback;

struct Tag {
    i64 id;
    StringView name;
    u32 color;

    static Result add(DatabaseConnection* connection, StringView tag, u32 color, i64* outTagId);
    static Result remove(DatabaseConnection* connection, i64 tag);
    static Result get(DatabaseConnection* connection, i64 tag, TagCallback callback);

    static Result setColor(DatabaseConnection* connection, i64 tag, u32 color);
    static Result rename(DatabaseConnection* connection, i64 tag, StringView newName);

    static Result descendants(DatabaseConnection* connection, i64 tag, TagCallback callback);
    static Result ancestors(DatabaseConnection* connection, i64 tag, TagCallback callback);
    static Result parents(DatabaseConnection* connection, i64 tag, TagCallback callback);

    static Result addParent(DatabaseConnection* connection, i64 parentTag, i64 childTag);
    static Result removeParent(DatabaseConnection* connection, i64 parentTag, i64 childTag);


    static Result enumerate(DatabaseConnection* connection, u32 limit, u32 offset, TagCallback callback);
    static Result search(DatabaseConnection* connection, StringView query, u32 limit, u32 offset, TagCallback callback);
    static Result find(DatabaseConnection* connection, StringView name, TagCallback callback);

    static Result lookupByIds(DatabaseConnection* connection, StringView jsonIds, TagCallback callback);

    
    static Result tagGame(DatabaseConnection* connection, i64 gameId, i64 tag);
    static Result untagGame(DatabaseConnection* connection, i64 gameId, i64 tag);
    static Result enumerateGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback);

    static Result tagInstallation(DatabaseConnection* connection, i64 installationId, i64 tag);
    static Result untagInstallation(DatabaseConnection* connection, i64 installationId, i64 tag);

    static Result countInstallationTags(DatabaseConnection* connection, i64 installationId, u64* outCount);
    static Result enumerateInstallationTags(DatabaseConnection* connection, i64 installationId, u64 offset, u64 limit, TagCallback callback);
    static Result enumerateDirectInstallationTags(DatabaseConnection* connection, i64 installationId, TagCallback callback);
};

}
