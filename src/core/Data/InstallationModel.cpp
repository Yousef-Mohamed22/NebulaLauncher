// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file InstallationModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to installation management.
 */

#include <Data/Model.hpp>
#include <Platform/StringBuilder.hpp>
#include <Platform/Time.hpp>
#include "Core/Result.hpp"
#include "Data/Database.hpp"


nbl::Result nbl::addInstallation(DatabaseConnection* connection, i64 displayId, i64 gameId, StringView installPath, i64* outInstallationId) {

    static nbl::PreparedStatement _addInstallationStmt;
    if(!_addInstallationStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO installation (gId, command, playTime, playCount, lastPlayed, added, dId) VALUES (?1, ?2, 0, 0, 0, ?3, ?4);
        )", &_addInstallationStmt));
    }

    NBL_CHECK(_addInstallationStmt.bindUint(1, gameId));
    NBL_CHECK(_addInstallationStmt.bindString(2, installPath));
    NBL_CHECK(_addInstallationStmt.bindUint(3, nbl::getCurrentTime()));
    if(displayId == 0) {
        NBL_CHECK(_addInstallationStmt.bindNull(4));
    } else {
        NBL_CHECK(_addInstallationStmt.bindUint(4, displayId));
    }

    return connection->execute(&_addInstallationStmt, nullptr, nullptr, outInstallationId);
}


nbl::Result nbl::removeInstallation(DatabaseConnection* connection, i64 installationId) {

    static nbl::PreparedStatement _removeInstallationStmt;
    if(!_removeInstallationStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM installation WHERE iId = ?1;
        )", &_removeInstallationStmt));
    }

    NBL_CHECK(_removeInstallationStmt.bindUint(1, installationId));
    return connection->execute(&_removeInstallationStmt, nullptr, nullptr);
}

nbl::Result nbl::updateInstallation(DatabaseConnection* connection, i64 installationId, i64 displayId, i64 gameId, StringView installPath) {

    static nbl::PreparedStatement _updateInstallationStmt;
    if(!_updateInstallationStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE installation SET gId=?1, command=?2, dId=?3 WHERE iId=?4;
        )", &_updateInstallationStmt));
    }

    NBL_CHECK(_updateInstallationStmt.bindUint(1, gameId));
    NBL_CHECK(_updateInstallationStmt.bindString(2, installPath));
    if(displayId == 0) {
        NBL_CHECK(_updateInstallationStmt.bindNull(3));
    } else {
        NBL_CHECK(_updateInstallationStmt.bindUint(3, displayId));
    }
    NBL_CHECK(_updateInstallationStmt.bindUint(4, installationId));

    return connection->execute(&_updateInstallationStmt, nullptr, nullptr);
}


nbl::Result nbl::getInstallationMeta(DatabaseConnection* connection, i64 installationId, InstallationCallback callback, void* userData) {

    static nbl::PreparedStatement _getInstallationMetaStmt;
    if(!_getInstallationMetaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT installation.iId, installation.dId, installation.gId, display.title, display.description, display.poster, display.banner, display.icon, display.logo, display.logoPosition, installation.command, installation.playTime, installation.playCount, installation.lastPlayed, installation.added, game.releaseDate, game.rating, game.steamId, game.igdbId, game_display.title AS game_title, game_display.poster AS game_poster
FROM installation
JOIN game ON installation.gId = game.gId
JOIN display ON display.dId = IFNULL(installation.dId, game.dId)
JOIN display AS game_display ON game_display.dId = game.dId
WHERE installation.iId = ?1;
        )", &_getInstallationMetaStmt));
    }

    NBL_CHECK(_getInstallationMetaStmt.bindUint(1, installationId));

    struct _InstallationMetaQueryData {
        InstallationCallback callback;
        void* userData;
    } queryData{ callback, userData };

    return connection->execute(&_getInstallationMetaStmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 21) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 21 columns in installation meta query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _InstallationMetaQueryData* queryData = static_cast<_InstallationMetaQueryData*>(userData);

        InstallationMeta meta = {
            .id = columnValues[0].intValue,
            .dId = columnValues[1].intValue,
            .gId = columnValues[2].intValue,
            .title = columnValues[3].textValue,
            .description = columnValues[4].textValue,
            .poster = columnValues[5].textValue,
            .banner = columnValues[6].textValue,
            .icon = columnValues[7].textValue,
            .logo = columnValues[8].textValue,
            .logoPosition = byId(columnValues[9].uintValue()),
            .command = columnValues[10].textValue,
            .playTime = columnValues[11].uintValue(),
            .playCount = columnValues[12].uintValue(),
            .lastPlayed = columnValues[13].uintValue(),
            .added = columnValues[14].uintValue(),
            .releaseDate = columnValues[15].uintValue(),
            .rating = columnValues[16].floatValue,
            .steamId = columnValues[17].uintValue(),
            .igdbId = columnValues[18].uintValue(),
            .gameTitle = columnValues[19].textValue,
            .gamePoster = columnValues[20].textValue
        };

        return queryData->callback(queryData->userData, meta);

    }, &queryData);
}

nbl::Result nbl::getInstallationCommand(DatabaseConnection* connection, i64 installationId, InstallationCallback callback, void* userData) {

    static nbl::PreparedStatement _getInstallationMetaStmt;
    if(!_getInstallationMetaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT installation.iId, installation.command
FROM installation
WHERE installation.iId = ?1;
        )", &_getInstallationMetaStmt));
    }

    NBL_CHECK(_getInstallationMetaStmt.bindUint(1, installationId));

    struct _InstallationMetaQueryData {
        InstallationCallback callback;
        void* userData;
    } queryData{ callback, userData };

    return connection->execute(&_getInstallationMetaStmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 2) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 2 columns in installation meta query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _InstallationMetaQueryData* queryData = static_cast<_InstallationMetaQueryData*>(userData);

        InstallationMeta meta = {
            .id = columnValues[0].intValue,
            .command = columnValues[1].textValue
        };

        return queryData->callback(queryData->userData, meta);

    }, &queryData);
}


nbl::Result nbl::lookupInstallations(
        DatabaseConnection* connection, 
        const InstallationSearchCriteria& criteria,
        InstallationCallback callback,
        void* userData) {

    
    ArenaFrame frame(Arena::threadLocal());

    static nbl::PreparedStatement _lookupInstallationsStmts[16];

    usize index = static_cast<usize>(criteria.sortBy) * 2 + static_cast<usize>(criteria.sortOrder);
    if(index >= 16) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    nbl::PreparedStatement& lookupInstallationsStmt = _lookupInstallationsStmts[index];

    if(!lookupInstallationsStmt.isValid(connection)) {

        static StringView _stmtBegin = R"(
WITH 
    all_tags AS (
        SELECT descendantId
        FROM tag_closure
        WHERE ?1 IS NULL
           OR tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?1)))
    ),
    excluded_tags AS (
        SELECT descendantId
        FROM tag_closure
        WHERE NOT ?2 IS NULL
          AND tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?2)))
    )
SELECT installation.iId, installation.dId, display.title, display.poster, display.icon, installation.playTime, installation.playCount, installation.lastPlayed, installation.added, game.releaseDate, game.rating
FROM installation
JOIN game ON installation.gId = game.gId
JOIN display ON display.dId = IFNULL(installation.dId, game.dId)
LEFT JOIN installation_tag ON installation_tag.iId = installation.iId
LEFT JOIN game_tag ON game_tag.gId = installation.gId
WHERE game.releaseDate >= ?3
  AND (?4 = 0 OR game.releaseDate <= ?4)
  AND game.rating >= ?5
  AND (?6 = 0 OR game.rating <= ?6)
  AND installation.playTime >= ?7
  AND (?8 = 0 OR installation.playTime <= ?8)
  AND (?1 IS NULL OR installation_tag.tId IN all_tags OR game_tag.tId IN all_tags)
  AND (?2 IS NULL OR NOT (installation_tag.tId IN excluded_tags OR game_tag.tId IN excluded_tags))
  AND (?9 IS NULL OR display.title LIKE '%' || ?9 || '%')
GROUP BY installation.iId
)";

        static StringView _stmtEnd = R"(
LIMIT ?11
OFFSET ?10;
        )";

        StringBuilder stmtBuilder(&frame);
        stmtBuilder.append(_stmtBegin);

        switch(criteria.sortBy) {
            case InstallationSortBy::TITLE:
                stmtBuilder.append("ORDER BY display.title");
                break;
            case InstallationSortBy::LAST_PLAYED:
                stmtBuilder.append("ORDER BY installation.lastPlayed");
                break;
            case InstallationSortBy::ADDED:
                stmtBuilder.append("ORDER BY installation.added");
                break;
            case InstallationSortBy::PLAY_COUNT:
                stmtBuilder.append("ORDER BY installation.playCount");
                break;
            case InstallationSortBy::PLAY_TIME:
                stmtBuilder.append("ORDER BY installation.playTime");
                break;
            case InstallationSortBy::RELEASE_DATE:
                stmtBuilder.append("ORDER BY game.releaseDate");
                break;
            case InstallationSortBy::RATING:
                stmtBuilder.append("ORDER BY game.rating");
                break;
            default:
                return nbl::error::ILLEGAL_ARGUMENT;
        }

        if(criteria.sortOrder == SortOrder::ASCENDING) {
            stmtBuilder.append(" ASC");
        } else {
            stmtBuilder.append(" DESC");
        }
        stmtBuilder.append(_stmtEnd);

        StringView stmt;
        NBL_CHECK(stmtBuilder.buildNullTerminated(&stmt));

        NBL_CHECK(connection->prepare(stmt, &lookupInstallationsStmt));
    }


    if(criteria.tags.length() == 0) {
        NBL_CHECK(lookupInstallationsStmt.bindNull(1));
    } else {
        for(const JsonValue& tag : criteria.tags) {
            if(tag.type != JsonType::STRING) {
                return nbl::error::ILLEGAL_ARGUMENT;
            }
        }
        StringView tagsJson;
        StringBuilder tagsBuilder(&frame);
        tagsBuilder.append(JsonValue::fromArray(criteria.tags));
        tagsBuilder.build(&tagsJson);
        NBL_CHECK(lookupInstallationsStmt.bindString(1, tagsJson));
    }

    if(criteria.excludedTags.length() == 0) {
        NBL_CHECK(lookupInstallationsStmt.bindNull(2));
    } else {
        for(const JsonValue& tag : criteria.excludedTags) {
            if(tag.type != JsonType::STRING) {
                return nbl::error::ILLEGAL_ARGUMENT;
            }
        }
        StringView excludedTagsJson;
        StringBuilder excludedTagsBuilder(&frame);
        excludedTagsBuilder.append(JsonValue::fromArray(criteria.excludedTags));
        excludedTagsBuilder.build(&excludedTagsJson);
        NBL_CHECK(lookupInstallationsStmt.bindString(2, excludedTagsJson));
    }

    NBL_CHECK(lookupInstallationsStmt.bindUint(3, criteria.minReleaseDate));
    NBL_CHECK(lookupInstallationsStmt.bindUint(4, criteria.maxReleaseDate));
    NBL_CHECK(lookupInstallationsStmt.bindFloat(5, criteria.minRating));
    NBL_CHECK(lookupInstallationsStmt.bindFloat(6, criteria.maxRating));
    NBL_CHECK(lookupInstallationsStmt.bindUint(7, criteria.minPlayTime));
    NBL_CHECK(lookupInstallationsStmt.bindUint(8, criteria.maxPlayTime));
    NBL_CHECK(lookupInstallationsStmt.bindStringOrNull(9, criteria.titleFilter));
    NBL_CHECK(lookupInstallationsStmt.bindUint(10, criteria.offset));
    NBL_CHECK(lookupInstallationsStmt.bindUint(11, criteria.limit == 0 ? 10 : criteria.limit));


    struct _InstallationQueryData {
        InstallationCallback callback;
        void* userData;
    } queryData{ callback, userData };

    NBL_CHECK(connection->execute(&lookupInstallationsStmt, [](
        void* userData, 
        i32 columnCount, 
        DataValue* columnValues, 
        const char** columnNames, 
        i64 lastInsertRowId) -> nbl::Result {

        if(columnCount < 11) {
            return nbl::error::IO_ERROR;
        }

        _InstallationQueryData* queryData = static_cast<_InstallationQueryData*>(userData);

        InstallationMeta meta = {
            .id = columnValues[0].intValue,
            .dId = columnValues[1].intValue,
            .title = columnValues[2].textValue,
            .poster = columnValues[3].textValue,
            .icon = columnValues[4].textValue,
            .playTime = columnValues[5].uintValue(),
            .playCount = columnValues[6].uintValue(),
            .lastPlayed = columnValues[7].uintValue(),
            .added = columnValues[8].uintValue(),
            .releaseDate = columnValues[9].uintValue(),
            .rating = columnValues[10].floatValue
        };

        return queryData->callback(
            queryData->userData,
            meta
        );
    }, &queryData));

    return nbl::Result::SUCCESS;
}


nbl::Result nbl::countInstallationTags(nbl::DatabaseConnection* connection, i64 installationId, u64* outCount) {

    static nbl::PreparedStatement _countInstallationTagsStmt;
    if(!_countInstallationTagsStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
WITH 
	parentGame AS (
		SELECT gId FROM installation WHERE iId = ?1
	)
SELECT COUNT(DISTINCT tag.tId)
FROM tag
JOIN tag_closure ON tag_closure.ancestorId = tag.tId
WHERE descendantId IN (SELECT tId FROM installation_tag WHERE iId = ?1)
   OR descendantId IN (SELECT tId FROM game_tag WHERE game_tag.gId = (SELECT gId FROM parentGame));
        )", &_countInstallationTagsStmt));
    }

    NBL_CHECK(_countInstallationTagsStmt.bindInt(1, installationId));

    return connection->execute(&_countInstallationTagsStmt, [](void* userData, i32 columnCount, DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount == 0) return nbl::error::IO_ERROR;

        *static_cast<u64*>(userData) = columnValues[0].uintValue();

        return nbl::Result::SUCCESS;
    }, outCount);


}


nbl::Result nbl::getInstallationTags(DatabaseConnection* connection, i64 installationId, u64 offset, u64 limit, TagCallback callback, void* userData) {

    static nbl::PreparedStatement _getInstallationTagsStmt;
    if(!_getInstallationTagsStmt.isValid(connection)) {

        NBL_CHECK(connection->prepare(R"(
WITH 
	parentGame AS (
		SELECT gId FROM installation WHERE iId = ?1
	)
SELECT tag.name, tag.color
FROM tag
JOIN tag_closure ON tag_closure.ancestorId = tag.tId
WHERE descendantId IN (SELECT tId FROM installation_tag WHERE iId = ?1)
   OR descendantId IN (SELECT tId FROM game_tag WHERE game_tag.gId = (SELECT gId FROM parentGame)) 
GROUP BY tag.tId
LIMIT ?2 OFFSET ?3;
        )", &_getInstallationTagsStmt));
    }

    NBL_CHECK(_getInstallationTagsStmt.bindInt(1, installationId));
    NBL_CHECK(_getInstallationTagsStmt.bindUint(2, limit));
    NBL_CHECK(_getInstallationTagsStmt.bindUint(3, offset));

    struct _QueryData {
        nbl::TagCallback callback;
        void* userData;
    };
    _QueryData queryData{callback, userData};

    return connection->execute(&_getInstallationTagsStmt, [](void* userData, i32 columnCount, DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(columnCount != 2) {
            return nbl::error::IO_ERROR;
        }
        if(columnValues[0].type != nbl::DataType::TEXT || columnValues[1].type != nbl::DataType::INTEGER) {
            return nbl::error::IO_ERROR;
        }
        _QueryData* data = static_cast<_QueryData*>(userData);
        return data->callback(data->userData, columnValues[0].textValue, columnValues[1].intValue);
    }, &queryData);

}


nbl::Result nbl::markInstallationJustPlayed(DatabaseConnection* connection, i64 installationId) {

    static nbl::PreparedStatement _markInstallationJustPlayedStmt;
    if(!_markInstallationJustPlayedStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE installation
SET playCount = playCount + 1,
    lastPlayed = ?1
WHERE iId = ?2;
        )", &_markInstallationJustPlayedStmt));
    }

    NBL_CHECK(_markInstallationJustPlayedStmt.bindUint(1, nbl::getCurrentTime()));
    NBL_CHECK(_markInstallationJustPlayedStmt.bindUint(2, installationId));

    return connection->execute(&_markInstallationJustPlayedStmt, nullptr, nullptr);
}

nbl::Result nbl::bumpInstallationPlayTime(DatabaseConnection* connection, i64 installationId, u64 playTime) {

    static nbl::PreparedStatement _incrementInstallationPlayTimeStmt;
    if(!_incrementInstallationPlayTimeStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE installation
SET playTime = playTime + ?1
WHERE iId = ?2;
        )", &_incrementInstallationPlayTimeStmt));
    }
    
    NBL_CHECK(_incrementInstallationPlayTimeStmt.bindUint(1, playTime));
    NBL_CHECK(_incrementInstallationPlayTimeStmt.bindUint(2, installationId));

    return connection->execute(&_incrementInstallationPlayTimeStmt, nullptr, nullptr);

}
