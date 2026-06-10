// Copyright (c) 2026, jwall64. All rights reserved.

/**
 * \file GameModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp. Functions related to game management.
 */


#include <Core/Array.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>


namespace {

    struct _GameQueryUserData {
        nbl::GameCallback callback;
        void* userData;
    } queryData;


    nbl::Result _gameMetaCallback(void* userData, i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) {

        if(argc != 10) {
            return nbl::error::ILLEGAL_STATE;
        }

        _GameQueryUserData* queryData = static_cast<_GameQueryUserData*>(userData);

        nbl::GameMeta meta = {
            .id = values[0].intValue,
            .title = values[1].textValue,
            .description = values[2].textValue,
            .poster = values[3].textValue,
            .banner = values[4].textValue,
            .icon = values[5].textValue,
            .logo = values[6].textValue,
            .logoPosition = nbl::byId(values[7].uintValue()),
            .releaseDate = values[8].uintValue(),
            .rating = values[9].floatValue
        };

        return queryData->callback(
            queryData->userData, 
            meta
        );
    }


}


nbl::Result nbl::addDisplay(DatabaseConnection *connection, const DisplayMeta &meta, i64 *outDisplayId) {

    static nbl::PreparedStatement _displayStmt;
    if(!_displayStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO display (title, description, poster, banner, icon, logo, logoPosition) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);
        )", &_displayStmt));
    }

    NBL_CHECK(_displayStmt.bindString(1, meta.title));
    NBL_CHECK(_displayStmt.bindStringOrNull(2, meta.description));
    NBL_CHECK(_displayStmt.bindStringOrNull(3, meta.poster));
    NBL_CHECK(_displayStmt.bindStringOrNull(4, meta.banner));
    NBL_CHECK(_displayStmt.bindStringOrNull(5, meta.icon));
    NBL_CHECK(_displayStmt.bindStringOrNull(6, meta.logo));
    NBL_CHECK(_displayStmt.bindInt(7, static_cast<i32>(meta.logoPosition)));

    return connection->execute(&_displayStmt, nullptr, nullptr, outDisplayId);
}

nbl::Result nbl::removeDisplay(nbl::DatabaseConnection* connection, i64 displayId) {

    static nbl::PreparedStatement _removeDisplayStmt;
    if(!_removeDisplayStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM display WHERE dId = ?;
        )", &_removeDisplayStmt));
    }

    NBL_CHECK(_removeDisplayStmt.bindInt(1, displayId));
    return connection->execute(&_removeDisplayStmt, nullptr, nullptr);
}


nbl::Result nbl::addGame(DatabaseConnection* connection, const nbl::GameMeta& meta, i64* outGameId) {

    static nbl::PreparedStatement _addGameStmt;
    if(!_addGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO game (steamId, igdbId, releaseDate, rating, dId) VALUES (?1, ?2, ?3, ?4, ?5);
        )", &_addGameStmt));
    }

    if(meta.dId == 0) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }
    NBL_CHECK(_addGameStmt.bindUint(1, meta.steamId));
    NBL_CHECK(_addGameStmt.bindUint(2, meta.igdbId));
    NBL_CHECK(_addGameStmt.bindUint(3, meta.releaseDate));
    NBL_CHECK(_addGameStmt.bindFloat(4, meta.rating));
    NBL_CHECK(_addGameStmt.bindInt(5, meta.dId));

    return connection->execute(&_addGameStmt, nullptr, nullptr, outGameId); 
}

nbl::Result nbl::removeGame(DatabaseConnection* connection, i64 gameId) {

    static nbl::PreparedStatement _removeGameStmt;
    if(!_removeGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM game WHERE gId = ?1;
        )", &_removeGameStmt));
    }

    NBL_CHECK(_removeGameStmt.bindUint(1, gameId));
    return connection->execute(&_removeGameStmt, nullptr, nullptr);
}

nbl::Result nbl::addGameTag(DatabaseConnection* connection, i64 gameId, StringView tag) {

    static nbl::PreparedStatement _addGameTagStmt;
    if(!_addGameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO game_tag (gId, tId) VALUES (?1, (SELECT tId FROM tag WHERE name = ?2));
        )", &_addGameTagStmt));
    }

    NBL_CHECK(_addGameTagStmt.bindUint(1, gameId));
    NBL_CHECK(_addGameTagStmt.bindString(2, tag));
    return connection->execute(&_addGameTagStmt, nullptr, nullptr);
}

nbl::Result nbl::removeGameTag(DatabaseConnection* connection, i64 gameId, StringView tag) {

    static nbl::PreparedStatement _removeGameTagStmt;
    if(!_removeGameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM game_tag WHERE gId = ?1 AND tId = (SELECT tId FROM tag WHERE name = ?2);
        )", &_removeGameTagStmt));
    }
    
    NBL_CHECK(_removeGameTagStmt.bindUint(1, gameId));
    NBL_CHECK(_removeGameTagStmt.bindString(2, tag));
    return connection->execute(&_removeGameTagStmt, nullptr, nullptr);
}


nbl::Result nbl::getGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback, void* userData) {

    static nbl::PreparedStatement _getGameTagsStmt;
    if(!_getGameTagsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tag.name, tag.color
FROM tag
JOIN game_tag ON game_tag.tId = tag.tId
WHERE game_tag.gId = ?1;
        )", &_getGameTagsStmt));
    }

    NBL_CHECK(_getGameTagsStmt.bindUint(1, gameId));

    struct _TagQueryData {
        TagCallback callback;
        void* userData;
    } data{ callback, userData };

    return connection->execute(&_getGameTagsStmt, [](void* userData, i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {

        if(argc != 2) {
            return nbl::error::ILLEGAL_STATE;
        }

        if(values[0].type != nbl::DataType::TEXT || values[1].type != nbl::DataType::INTEGER) {
            return nbl::error::ILLEGAL_STATE;
        }

        _TagQueryData* data = static_cast<_TagQueryData*>(userData);
        return data->callback(data->userData, values[0].textValue, static_cast<u32>(values[1].intValue));

    }, &data);

}


nbl::Result nbl::getGameMeta(DatabaseConnection* connection, i64 gameId, GameCallback callback, void* userData) {

    static nbl::PreparedStatement _getGameMetaStmt;
    if(!_getGameMetaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT game.gId, display.title, display.description, display.poster, display.banner, display.icon, display.logo, display.logoPosition, game.releaseDate, game.rating
FROM game
JOIN display ON display.dId = game.dId
WHERE game.gId = ?1;
        )", &_getGameMetaStmt));
    }

    NBL_CHECK(_getGameMetaStmt.bindUint(1, gameId));

    _GameQueryUserData queryData{ callback, userData };
    return connection->execute(&_getGameMetaStmt, _gameMetaCallback, &queryData);
    
}



nbl::Result nbl::lookupGames(DatabaseConnection *connection, const GameSearchCriteria& criteria, GameCallback callback, void *userData) {

    usize sortByIndex = static_cast<usize>(criteria.sortBy);
    usize sortOrderIndex = static_cast<usize>(criteria.sortOrder);

    usize statementIndex = sortByIndex * 2 + sortOrderIndex;
    if(statementIndex >= 6) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    static nbl::PreparedStatement _lookupGamesStmts[6];
    nbl::PreparedStatement& lookupGamesStmt = _lookupGamesStmts[statementIndex];

    if(!lookupGamesStmt.isValid(connection)) {

        static StringView _stmtBegin = R"(
WITH 
    all_tags AS (
        SELECT descendantId
        FROM tag_closure
        WHERE NOT ?1 IS NULL
          AND tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?1)))
    ),
    excluded_tags AS (
        SELECT descendantId
        FROM tag_closure
        WHERE NOT ?2 IS NULL
          AND tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?2)))
    )
SELECT game.gId, display.title, display.description, display.poster, display.banner, display.icon, display.logo, display.logoPosition, game.releaseDate, game.rating
FROM game
JOIN display ON display.dId = game.dId
LEFT JOIN game_tag ON game_tag.gId = game.gId
WHERE game.releaseDate >= ?
  AND (?4 = 0 OR game.releaseDate <= ?4)
  AND game.rating >= ?5
  AND (?6 = 0 OR game.rating <= ?6)
  AND (?1 IS NULL OR game_tag.tId IN all_tags)
  AND (?2 IS NULL OR NOT game_tag.tId IN excluded_tags)
  AND (?7 IS NULL OR display.title LIKE '%' || ?7 || '%')
GROUP BY game.gId
)";

        static StringView _stmtEnd = R"(
LIMIT ?9
OFFSET ?8;
        )";

    
        ArenaFrame frame(Arena::threadLocal());

        StringBuilder stmtBuilder(&frame);
        stmtBuilder.append(_stmtBegin);

        switch(criteria.sortBy) {
            case GameSortBy::TITLE:
                stmtBuilder.append("ORDER BY display.title");
                break;
            case GameSortBy::RELEASE_DATE:
                stmtBuilder.append("ORDER BY game.releaseDate");
                break;
            case GameSortBy::RATING:
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

        StringView finalStmt;
        NBL_CHECK(stmtBuilder.buildNullTerminated(&finalStmt));

        NBL_CHECK(connection->prepare(finalStmt, &lookupGamesStmt));
    }

    {
        
        ArenaFrame frame(Arena::threadLocal());
        if(criteria.tags.length() == 0) {
            NBL_CHECK(lookupGamesStmt.bindNull(1));
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

            NBL_CHECK(lookupGamesStmt.bindString(1, tagsJson));
        }

        if(criteria.excludedTags.length() == 0) {
            NBL_CHECK(lookupGamesStmt.bindNull(2));
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
            NBL_CHECK(lookupGamesStmt.bindString(2, excludedTagsJson));
        }

        u64 maxReleaseDate = criteria.maxReleaseDate;
        if(maxReleaseDate > INT64_MAX) {
            maxReleaseDate = INT64_MAX;
        }

        
        NBL_CHECK(lookupGamesStmt.bindInt(3, criteria.minReleaseDate));
        NBL_CHECK(lookupGamesStmt.bindInt(4, criteria.maxReleaseDate));
        NBL_CHECK(lookupGamesStmt.bindFloat(5, criteria.minRating));
        NBL_CHECK(lookupGamesStmt.bindFloat(6, criteria.maxRating));
        NBL_CHECK(lookupGamesStmt.bindStringOrNull(7, criteria.titleFilter));
        NBL_CHECK(lookupGamesStmt.bindUint(8, criteria.offset));
        NBL_CHECK(lookupGamesStmt.bindUint(9, criteria.limit == 0 ? 10 : 0));
    }

    _GameQueryUserData queryData{ callback, userData };
    return connection->execute(&lookupGamesStmt, _gameMetaCallback, &queryData);

}

