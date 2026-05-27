// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file InstallationModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to installation management.
 */

#include <Data/Model.hpp>
#include <Platform/StringBuilder.hpp>
#include <Platform/Time.hpp>


nbl::Result nbl::addInstallation(DatabaseConnection* connection, i64 gameId, StringView installPath, i64* outInstallationId) {

    static nbl::PreparedStatement _addInstallationStmt;
    if(!_addInstallationStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO installation (gId, command, playTime, playCount, lastPlayed, added, dId) VALUES (?1, ?2, 0, 0, 0, ?3, (SELECT dId FROM display WHERE dId = (SELECT dId FROM game WHERE gId = ?1)));
        )", &_addInstallationStmt));
    }

    NBL_CHECK(_addInstallationStmt.bindUint(1, gameId));
    NBL_CHECK(_addInstallationStmt.bindString(2, installPath));
    NBL_CHECK(_addInstallationStmt.bindUint(3, nbl::getCurrentTime()));

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


nbl::Result nbl::getInstallationMeta(DatabaseConnection* connection, i64 installationId, InstallationCallback callback, void* userData) {

    static nbl::PreparedStatement _getInstallationMetaStmt;
    if(!_getInstallationMetaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT installation.iId, display.title, display.description, display.poster, display.banner, display.icon, display.logo, installation.command, installation.playTime, installation.playCount, installation.lastPlayed, installation.added, game.releaseDate, game.rating, game_display.title AS game_title, game_display.poster AS game_poster
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

        if(columnCount != 16) {
            nbl::log(nbl::LogLevel::ERROR, "Expected 16 columns in installation meta query result, but got ", columnCount);
            return nbl::error::IO_ERROR;
        }

        _InstallationMetaQueryData* queryData = static_cast<_InstallationMetaQueryData*>(userData);

        InstallationMeta meta = {
            .id = columnValues[0].intValue,
            .title = columnValues[1].textValue,
            .description = columnValues[2].textValue,
            .poster = columnValues[3].textValue,
            .banner = columnValues[4].textValue,
            .icon = columnValues[5].textValue,
            .logo = columnValues[6].textValue,
            .command = columnValues[7].textValue,
            .playTime = columnValues[8].uintValue(),
            .playCount = columnValues[9].uintValue(),
            .lastPlayed = columnValues[10].uintValue(),
            .added = columnValues[11].uintValue(),
            .releaseDate = columnValues[12].uintValue(),
            .rating = columnValues[13].floatValue,
            .gameTitle = columnValues[14].textValue,
            .gamePoster = columnValues[15].textValue
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
SELECT installation.iId, display.title, display.poster, installation.playTime, installation.playCount, installation.lastPlayed, installation.added, game.releaseDate, game.rating
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

        if(columnCount < 9) {
            return nbl::error::IO_ERROR;
        }
        
        if(columnValues[0].type != DataType::INTEGER ||
           columnValues[1].type != DataType::TEXT ||
           columnValues[2].type != DataType::TEXT ||
           columnValues[3].type != DataType::INTEGER ||
           columnValues[4].type != DataType::INTEGER ||
           columnValues[5].type != DataType::INTEGER ||
           columnValues[6].type != DataType::INTEGER ||
           columnValues[7].type != DataType::INTEGER ||
           columnValues[8].type != DataType::FLOAT) {
            return nbl::error::IO_ERROR;
        }

        _InstallationQueryData* queryData = static_cast<_InstallationQueryData*>(userData);

        InstallationMeta meta = {
            .id = columnValues[0].intValue,
            .title = columnValues[1].textValue,
            .poster = columnValues[2].textValue,
            .playTime = columnValues[3].uintValue(),
            .playCount = columnValues[4].uintValue(),
            .lastPlayed = columnValues[5].uintValue(),
            .added = columnValues[6].uintValue(),
            .releaseDate = columnValues[7].uintValue(),
            .rating = columnValues[8].floatValue
        };

        return queryData->callback(
            queryData->userData,
            meta
        );
    }, &queryData));

    return nbl::Result::SUCCESS;
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
