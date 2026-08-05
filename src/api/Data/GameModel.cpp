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


nbl::Result nbl::addGame(DatabaseConnection* connection, const nbl::GameMeta& meta, i64* outGameId) {

    static nbl::PreparedStatement _addGameStmt;
    if(!_addGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO game (title, description, banner, poster, icon, logo, logoPosition, releaseDate, rating) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);
        )", &_addGameStmt));
    }

    NBL_CHECK(_addGameStmt.bindStringOrNull(1, meta.title));
    NBL_CHECK(_addGameStmt.bindStringOrNull(2, meta.description));
    NBL_CHECK(_addGameStmt.bindStringOrNull(3, meta.banner));
    NBL_CHECK(_addGameStmt.bindStringOrNull(4, meta.poster));
    NBL_CHECK(_addGameStmt.bindStringOrNull(5, meta.icon));
    NBL_CHECK(_addGameStmt.bindStringOrNull(6, meta.logo));
    NBL_CHECK(_addGameStmt.bindUint(7, static_cast<u32>(meta.logoPosition)));
    NBL_CHECK(_addGameStmt.bindUint(8, meta.releaseDate));
    NBL_CHECK(_addGameStmt.bindFloat(9, meta.rating));

    return connection->execute(&_addGameStmt, outGameId); 
}

nbl::Result nbl::removeGame(DatabaseConnection* connection, i64 gameId) {

    static nbl::PreparedStatement _removeGameStmt;
    if(!_removeGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM game WHERE gId = ?1;
        )", &_removeGameStmt));
    }

    NBL_CHECK(_removeGameStmt.bindUint(1, gameId));
    return connection->execute(&_removeGameStmt);
}


nbl::Result nbl::updateGame(DatabaseConnection* connection, i64 gameId, const GameMeta& meta) {

    static nbl::PreparedStatement _updateGameStmt;
    if(!_updateGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE game SET title=?1, description=?2, banner=?3, poster=?4, icon=?5, logo=?6, logoPosition=?7, releaseDate=?8, rating=?9 WHERE gId=?10;
        )", &_updateGameStmt));
    }

    NBL_CHECK(_updateGameStmt.bindStringOrNull(1, meta.title));
    NBL_CHECK(_updateGameStmt.bindStringOrNull(2, meta.description));
    NBL_CHECK(_updateGameStmt.bindStringOrNull(3, meta.banner));
    NBL_CHECK(_updateGameStmt.bindStringOrNull(4, meta.poster));
    NBL_CHECK(_updateGameStmt.bindStringOrNull(5, meta.icon));
    NBL_CHECK(_updateGameStmt.bindStringOrNull(6, meta.logo));
    NBL_CHECK(_updateGameStmt.bindUint(7, static_cast<u32>(meta.logoPosition)));
    NBL_CHECK(_updateGameStmt.bindUint(8, meta.releaseDate));
    NBL_CHECK(_updateGameStmt.bindFloat(9, meta.rating));
    NBL_CHECK(_updateGameStmt.bindInt(10, meta.id));

    return connection->execute(&_updateGameStmt); 

}


nbl::Result nbl::addGameTag(DatabaseConnection* connection, i64 gameId, i64 tag) {

    static nbl::PreparedStatement _addGameTagStmt;
    if(!_addGameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO game_tag (gId, tId) VALUES (?1, ?2);
        )", &_addGameTagStmt));
    }

    NBL_CHECK(_addGameTagStmt.bindUint(1, gameId));
    NBL_CHECK(_addGameTagStmt.bindInt(2, tag));
    return connection->execute(&_addGameTagStmt);
}

nbl::Result nbl::removeGameTag(DatabaseConnection* connection, i64 gameId, i64 tag) {

    static nbl::PreparedStatement _removeGameTagStmt;
    if(!_removeGameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM game_tag WHERE gId = ?1 AND tId = ?2;
        )", &_removeGameTagStmt));
    }
    
    NBL_CHECK(_removeGameTagStmt.bindUint(1, gameId));
    NBL_CHECK(_removeGameTagStmt.bindInt(2, tag));
    return connection->execute(&_removeGameTagStmt);
}


nbl::Result nbl::getGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback) {

    static nbl::PreparedStatement _getGameTagsStmt;
    if(!_getGameTagsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tag.tId, tag.name, tag.color
FROM tag
JOIN game_tag ON game_tag.tId = tag.tId
WHERE game_tag.gId = ?1
ORDER BY tag.name ASC;
        )", &_getGameTagsStmt));
    }

    NBL_CHECK(_getGameTagsStmt.bindUint(1, gameId));

    return connection->execute(&_getGameTagsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {

        if(argc != 3) {
            return nbl::Result::INVALID_STATE;
        }
        return callback({
            .id = values[0].intValue, 
            .name = values[1].textValue, 
            .color = static_cast<u32>(values[2].intValue)
        });

    });

}



nbl::Result nbl::getGameMeta(DatabaseConnection* connection, i64 gameId, GameMetaCallback callback) {

 static nbl::PreparedStatement _getGameMetaStmt;
    if(!_getGameMetaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT 
game.gId, game.title, game.description, game.banner, game.poster, game.icon, game.logo, game.logoPosition, game.releaseDate, game.rating 
FROM game
WHERE game.gId = ?1;
        )", &_getGameMetaStmt));
    }

    NBL_CHECK(_getGameMetaStmt.bindUint(1, gameId));

    return connection->execute(&_getGameMetaStmt, [&](i32 argc, nbl::DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        if(argc != 10) return nbl::Result::IO_ERROR;

        GameMeta meta = {
            .id = values[0].intValue,
            .title = values[1].textValue,
            .description = values[2].textValue,
            .banner = values[3].textValue,
            .poster = values[4].textValue,
            .icon = values[5].textValue,
            .logo = values[6].textValue,
            .logoPosition = static_cast<LogoPosition>(values[7].uintValue()),
            .releaseDate = values[8].uintValue(),
            .rating = values[9].floatValue
        };
        return callback(meta);

    });
}

nbl::Result nbl::getGameDetails(DatabaseConnection* connection, i64 gameId, GameDetailsCallback callback) {

    static nbl::PreparedStatement _getGameDetailsStmt;
    if(!_getGameDetailsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT
game.gId, game.title, game.description, game.banner, game.poster, game.icon, game.logo, game.logoPosition, game.releaseDate, game.rating,
sum(installation.playTime), sum(installation.playCount), max(installation.lastPlayed), min(installation.added), count(installation.iId)
FROM game
FULL OUTER JOIN installation ON installation.gId = game.gId
WHERE game.gId = ?1;
        )", &_getGameDetailsStmt));
    }

    NBL_CHECK(_getGameDetailsStmt.bindUint(1, gameId));

    return connection->execute(&_getGameDetailsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        GameDetails details = {
            .game = {
                .id = values[0].intValue,
                .title = values[1].textValue,
                .description = values[2].textValue,
                .banner = values[3].textValue,
                .poster = values[4].textValue,
                .icon = values[5].textValue,
                .logo = values[6].textValue,
                .logoPosition = static_cast<LogoPosition>(values[7].intValue),
                .releaseDate = values[8].uintValue(),
                .rating = values[9].floatValue
            },
            .playTime = values[10].uintValue(),
            .playCount = values[11].uintValue(),
            .lastPlayed = values[12].uintValue(),
            .added = values[13].uintValue(),
            .installationCount = values[14].uintValue()
        };
        return callback(details);

    });
    
}



nbl::Result nbl::lookupGames(DatabaseConnection *connection, const SearchCriteria& criteria, GameLookupCallback callback) {

    usize sortByIndex = static_cast<usize>(criteria.sortBy);
    usize sortOrderIndex = static_cast<usize>(criteria.sortOrder);

    usize statementIndex = sortByIndex * 2 + sortOrderIndex;
    if(statementIndex >= 16) {
        return nbl::Result::INVALID_ARGUMENT;
    }

    static nbl::PreparedStatement _lookupGamesStmts[16];
    nbl::PreparedStatement& lookupGamesStmt = _lookupGamesStmts[statementIndex];

    if(!lookupGamesStmt.isValid(connection)) {

        static StringView _stmtBegin = R"(
WITH 
search_sets AS (
	SELECT
		outer.key AS setId,
		inner.value AS tId
	FROM json_each(?1) AS outer
	JOIN json_each(outer.value) AS inner
),
set_sizes AS (
	SELECT
		setId,
		COUNT(*) AS requiredTags
	FROM search_sets
	GROUP BY setId
),
all_game_tags AS (
	SELECT game.gId, tag_closure.ancestorId as tId
	FROM game, tag_closure
	WHERE tag_closure.descendantId IN (SELECT game_tag.tId FROM game_tag WHERE game_tag.gId = game.gId)
),
matching_sets AS (
	SELECT 
		all_game_tags.gId,
		search_sets.setId,
		COUNT(DISTINCT search_sets.tId) AS matchedTags
	FROM all_game_tags
	JOIN search_sets 
	  ON search_sets.tId = all_game_tags.tId
	GROUP BY
		all_game_tags.gId,
		search_sets.setId
),
excluded_tags AS (
	SELECT descendantId as tId
	FROM tag_closure
	WHERE NOT ?2 IS NULL
	  AND tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?2)))
)
SELECT DISTINCT
game.gId, game.title, game.releaseDate, game.rating, game.poster, game.icon
FROM game
LEFT JOIN matching_sets ON matching_sets.gId = game.gId
LEFT JOIN set_sizes ON set_sizes.setId = matching_sets.setId
FULL OUTER JOIN installation ON installation.gId = game.gId
WHERE game.releaseDate >= ?3
  AND (?4 = 0 OR game.releaseDate <= ?4)
  AND game.rating >= ?5
  AND (?6 = 0 OR game.rating <= ?6)
  AND (?7 IS NULL OR game.title LIKE '%' || ?7 || '%')
  AND (?1 IS NULL OR matching_sets.matchedTags = set_sizes.requiredTags)
  AND (?2 IS NULL OR NOT EXISTS (SELECT tId FROM excluded_tags INTERSECT SELECT tId FROM all_game_tags WHERE all_game_tags.gId = game.gId))
  AND (?10 = 0 OR installation.deviceId = ?10)
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
            case SortBy::TITLE:
                if(criteria.titleFilter.empty()) {
                    stmtBuilder.append("ORDER BY game.title");
                } else {
                    stmtBuilder.append("ORDER BY nbl_str_distance(game.title, ?7)");
                }
                break;
            case SortBy::LAST_PLAYED:
                stmtBuilder.append("ORDER BY max(installation.lastPlayed)");
                break;
            case SortBy::RELEASE_DATE:
                stmtBuilder.append("ORDER BY game.releaseDate");
                break;
            case SortBy::ADDED:
                stmtBuilder.append("ORDER BY min(installation.added)");
                break;
            case SortBy::PLAY_COUNT:
                stmtBuilder.append("ORDER BY sum(installation.playCount)");
                break;
            case SortBy::PLAY_TIME:
                stmtBuilder.append("ORDER BY sum(installation.playTime)");
                break;
            case SortBy::RATING:
                stmtBuilder.append("ORDER BY game.rating");
                break;
            default:
                return nbl::Result::INVALID_ARGUMENT;
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
        if(criteria.tags.length == 0) {
            NBL_CHECK(lookupGamesStmt.bindNull(1));
        } else {
   
            StringBuilder tagsBuilder(&frame);
            NBL_CHECK(nbl::JsonWriter(tagsBuilder.writer())
                .writeArray([&](auto arr) {
                    for(ConstSpan<i64> tagSet : criteria.tags) {
                        arr.writeArray([&](auto inner) {
                            for(i64 tag : tagSet) {
                                inner.add(tag);
                            }
                        });
                    }
                }));

            StringView tagsJson;
            tagsBuilder.build(&tagsJson);

            NBL_CHECK(lookupGamesStmt.bindString(1, tagsJson));
        }

        if(criteria.excludedTags.length == 0) {
            NBL_CHECK(lookupGamesStmt.bindNull(2));
        } else {

            StringBuilder tagsBuilder(&frame);
            NBL_CHECK(nbl::JsonWriter(tagsBuilder.writer())
                .writeArray([&](auto arr) {
                    for(i64 tag : criteria.excludedTags) {
                        arr.add(tag);
                    }
                }));

            StringView tagsJson;
            tagsBuilder.build(&tagsJson);

            NBL_CHECK(lookupGamesStmt.bindString(2, tagsJson));
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
        NBL_CHECK(lookupGamesStmt.bindUint(9, criteria.limit == 0 ? 10 : criteria.limit));
        NBL_CHECK(lookupGamesStmt.bindInt(10, criteria.deviceId));
    }

    return connection->execute(&lookupGamesStmt, [&](i32 argc, nbl::DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        GameLookupResult res = {
            .id = values[0].intValue,
            .title = values[1].textValue,
            .releaseDate = values[2].uintValue(),
            .rating = values[3].floatValue,
            .poster = values[4].textValue,
            .icon = values[5].textValue,
        };
        return callback(res);

    });
}

