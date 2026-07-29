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


nbl::Result nbl::addDisplay(DatabaseConnection *connection, const DisplayMeta &meta, i64 *outDisplayId) {

    static nbl::PreparedStatement _displayStmt;
    if(!_displayStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO display (poster, banner, icon, logo, logoPosition) VALUES (?1, ?2, ?3, ?4, ?5);
        )", &_displayStmt));
    }

    NBL_CHECK(_displayStmt.bindStringOrNull(1, meta.poster));
    NBL_CHECK(_displayStmt.bindStringOrNull(2, meta.banner));
    NBL_CHECK(_displayStmt.bindStringOrNull(3, meta.icon));
    NBL_CHECK(_displayStmt.bindStringOrNull(4, meta.logo));
    NBL_CHECK(_displayStmt.bindInt(5, static_cast<i32>(meta.logoPosition)));

    return connection->execute(&_displayStmt, outDisplayId);
}

nbl::Result nbl::removeDisplay(nbl::DatabaseConnection* connection, i64 displayId) {

    static nbl::PreparedStatement _removeDisplayStmt;
    if(!_removeDisplayStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
DELETE FROM display WHERE dId = ?;
        )", &_removeDisplayStmt));
    }

    NBL_CHECK(_removeDisplayStmt.bindInt(1, displayId));
    return connection->execute(&_removeDisplayStmt);
}


nbl::Result nbl::updateDisplay(DatabaseConnection* connection, i64 displayId, const DisplayMeta& meta) {

    static nbl::PreparedStatement _updateDisplayStmt;
    if(!_updateDisplayStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
UPDATE display SET poster=?1, banner=?2, icon=?3, logo=?4, logoPosition=?5 WHERE dId=?6;
        )", &_updateDisplayStmt));
    }

    NBL_CHECK(_updateDisplayStmt.bindStringOrNull(1, meta.poster));
    NBL_CHECK(_updateDisplayStmt.bindStringOrNull(2, meta.banner));
    NBL_CHECK(_updateDisplayStmt.bindStringOrNull(3, meta.icon));
    NBL_CHECK(_updateDisplayStmt.bindStringOrNull(4, meta.logo));
    NBL_CHECK(_updateDisplayStmt.bindInt(5, static_cast<i32>(meta.logoPosition)));
    NBL_CHECK(_updateDisplayStmt.bindInt(6, displayId));


    return connection->execute(&_updateDisplayStmt);
}




nbl::Result nbl::addGame(DatabaseConnection* connection, const nbl::GameMeta& meta, i64* outGameId) {

    static nbl::PreparedStatement _addGameStmt;
    if(!_addGameStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO game (title, description, steamId, igdbId, releaseDate, rating, dId) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);
        )", &_addGameStmt));
    }

    if(meta.displayId == 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    NBL_CHECK(_addGameStmt.bindStringOrNull(1, meta.title));
    NBL_CHECK(_addGameStmt.bindStringOrNull(2, meta.description));
    NBL_CHECK(_addGameStmt.bindUint(3, meta.steamId));
    NBL_CHECK(_addGameStmt.bindUint(4, meta.igdbId));
    NBL_CHECK(_addGameStmt.bindUint(5, meta.releaseDate));
    NBL_CHECK(_addGameStmt.bindFloat(6, meta.rating));
    NBL_CHECK(_addGameStmt.bindInt(7, meta.displayId));

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
UPDATE game SET steamId=?1, igdbId=?2, releaseDate=?3, rating=?4, dId=?5 WHERE gId=?6;
        )", &_updateGameStmt));
    }

    if(meta.displayId == 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    NBL_CHECK(_updateGameStmt.bindUint(1, meta.steamId));
    NBL_CHECK(_updateGameStmt.bindUint(2, meta.igdbId));
    NBL_CHECK(_updateGameStmt.bindUint(3, meta.releaseDate));
    NBL_CHECK(_updateGameStmt.bindFloat(4, meta.rating));
    NBL_CHECK(_updateGameStmt.bindInt(5, meta.displayId));
    NBL_CHECK(_updateGameStmt.bindInt(6, gameId));

    return connection->execute(&_updateGameStmt); 

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
    return connection->execute(&_addGameTagStmt);
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
    return connection->execute(&_removeGameTagStmt);
}


nbl::Result nbl::getGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback) {

    static nbl::PreparedStatement _getGameTagsStmt;
    if(!_getGameTagsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tag.name, tag.color
FROM tag
JOIN game_tag ON game_tag.tId = tag.tId
WHERE game_tag.gId = ?1
ORDER BY tag.name ASC;
        )", &_getGameTagsStmt));
    }

    NBL_CHECK(_getGameTagsStmt.bindUint(1, gameId));

    return connection->execute(&_getGameTagsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnName, i64 lastInsertRowId) -> nbl::Result {

        if(argc != 2) {
            return nbl::Result::INVALID_STATE;
        }

        if(values[0].type != nbl::DataType::TEXT || values[1].type != nbl::DataType::INTEGER) {
            return nbl::Result::INVALID_STATE;
        }
        return callback(values[0].textValue, static_cast<u32>(values[1].intValue));

    });

}



nbl::Result nbl::getGameMeta(DatabaseConnection* connection, i64 gameId, GameMetaCallback callback) {

 static nbl::PreparedStatement _getGameDetailsStmt;
    if(!_getGameDetailsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT 
game.gId, game.displayId, game.steamId, game.igdbId, game.releaseDate, game.rating, 
FROM game
WHERE game.gId = ?1;
        )", &_getGameDetailsStmt));
    }

    NBL_CHECK(_getGameDetailsStmt.bindUint(1, gameId));

    return connection->execute(&_getGameDetailsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        GameMeta meta = {
            .id = values[0].intValue,
            .displayId = values[1].intValue,
            .steamId = values[2].uintValue(),
            .igdbId = values[3].uintValue(),
            .releaseDate = values[4].uintValue(),
            .rating = values[5].floatValue
        };
        return callback(meta);

    });
}

nbl::Result nbl::getGameDetails(DatabaseConnection* connection, i64 gameId, GameDetailsCallback callback) {

    static nbl::PreparedStatement _getGameDetailsStmt;
    if(!_getGameDetailsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT 
game.gId, game.title, game.description, game.steamId, game.igdbId, game.releaseDate, game.rating, 
display.dId, display.banner, display.poster, display.icon, display.logo, display.logoPosition
FROM game
JOIN display ON display.dId = game.dId
WHERE game.gId = ?1;
        )", &_getGameDetailsStmt));
    }

    NBL_CHECK(_getGameDetailsStmt.bindUint(1, gameId));

    return connection->execute(&_getGameDetailsStmt, [&](i32 argc, nbl::DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        GameDetails details = {
            .game = {
                .id = values[0].intValue,
                .displayId = values[5].intValue,
                .title = values[1].textValue,
                .description = values[2].textValue,
                .steamId = values[3].uintValue(),
                .igdbId = values[4].uintValue(),
                .releaseDate = values[5].uintValue(),
                .rating = values[6].floatValue
            },
            .display = {
                .id = values[7].intValue,
                .banner = values[8].textValue,
                .poster = values[9].textValue,
                .icon = values[10].textValue,
                .logo = values[11].textValue,
                .logoPosition = static_cast<LogoPosition>(values[12].intValue)
            }
        };
        return callback(details);

    });
    
}



nbl::Result nbl::lookupGames(DatabaseConnection *connection, const GameSearchCriteria& criteria, GameLookupCallback callback) {

    usize sortByIndex = static_cast<usize>(criteria.sortBy);
    usize sortOrderIndex = static_cast<usize>(criteria.sortOrder);

    usize statementIndex = sortByIndex * 2 + sortOrderIndex;
    if(statementIndex >= 6) {
        return nbl::Result::INVALID_ARGUMENT;
    }

    static nbl::PreparedStatement _lookupGamesStmts[6];
    nbl::PreparedStatement& lookupGamesStmt = _lookupGamesStmts[statementIndex];

    if(!lookupGamesStmt.isValid(connection)) {

        static StringView _stmtBegin = R"(
WITH 
search_sets AS (
	SELECT
		outer.key AS setId,
		(SELECT tId FROM tag WHERE tag.name = inner.value) AS tId
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
	SELECT descendantId
	FROM tag_closure
	WHERE NOT ?2 IS NULL
	  AND tag_closure.ancestorId IN (SELECT tId FROM tag WHERE name IN (SELECT value FROM json_each(?2)))
)
SELECT DISTINCT
game.gId, game.title, game.releaseDate, game.rating, display.poster, display.icon
FROM game
LEFT JOIN matching_sets ON matching_sets.gId = game.gId
LEFT JOIN set_sizes ON set_sizes.setId = matching_sets.setId
JOIN display ON display.dId = game.dId
LEFT JOIN game_tag ON game_tag.gId = game.gId
WHERE (?1 IS NULL OR matching_sets.matchedTags = set_sizes.requiredTags)
  AND (?2 IS NULL OR NOT EXISTS (SELECT tId FROM excluded_tags INTERSECT SELECT tId FROM all_game_tags WHERE all_game_tags.gId = game.gId))
  AND game.releaseDate >= ?3
  AND (?4 = 0 OR game.releaseDate <= ?4)
  AND game.rating >= ?5
  AND (?6 = 0 OR game.rating <= ?6)
  AND (?7 IS NULL OR game.title LIKE '%' || ?7 || '%')
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
                if(criteria.titleFilter.empty()) {
                    stmtBuilder.append("ORDER BY game.title");
                } else {
                    stmtBuilder.append("ORDER BY nbl_str_distance(game.title, ?7)");
                }
                break;
            case GameSortBy::RELEASE_DATE:
                stmtBuilder.append("ORDER BY game.releaseDate");
                break;
            case GameSortBy::RATING:
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
                    for(ConstSpan<StringView> tagSet : criteria.tags) {
                        arr.writeArray([&](auto inner) {
                            for(StringView tag : tagSet) {
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
                    for(StringView tag : criteria.excludedTags) {
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

