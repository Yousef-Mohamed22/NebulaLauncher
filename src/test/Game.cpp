#include <gtest/gtest.h>

#include <Data/Model.hpp>
#include <Core/Array.hpp>

namespace {

nbl::GameMeta _gameMeta = {
    .id = 0,
    .title = "Test Game",
    .description = "A test game",
    .banner = 1,
    .poster = 2,
    .icon = 3,
    .logo = 4,
    .releaseDate = 1600000000,
    .rating = 4.5
};

}

TEST(Game, AddGame) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);
}

TEST(Game, RemoveGame) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);
    ASSERT_TRUE(nbl::removeGame(&connection, gameId).success());
}

TEST(Game, AddGameTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 tId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, tId).success());
}

TEST(Game, RemoveGameTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 tId;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());

    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, tId).success());
    ASSERT_TRUE(nbl::removeGameTag(&connection, gameId, tId).success());
}

TEST(Game, GetGameTags) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 actId;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &actId).success());

    i64 advId;
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00, &advId).success());

    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, actId).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, advId).success());

    struct _FoundData {
        bool action = false;
        bool adventure = false;
    } found;

    ASSERT_TRUE(nbl::getGameTags(&connection, gameId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Action") {
            found.action = true;
        }
        if(tag.name == "Adventure") {
            found.adventure = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found.action);
    ASSERT_TRUE(found.adventure);
}

TEST(Game, GetGameMeta) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);


    bool found = false;
    ASSERT_TRUE(nbl::getGameDetails(&connection, gameId, [&](const nbl::GameDetails& details) -> nbl::Result {
        if(details.game.title == "Test Game" 
        && details.game.description == "A test game" 
        && details.game.banner == 1
        && details.game.poster == 2 
        && details.game.icon == 3 
        && details.game.logo == 4 
        && details.game.releaseDate == 1600000000 
        && details.game.rating == 4.5) {

            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);

}

TEST(Game, LookupSingleGame) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 foundId = 0;
    ASSERT_TRUE(nbl::lookupGames(
        &connection, {}, 
        [&](const nbl::GameLookupResult& res) -> nbl::Result {

        if(res.title == "Test Game" 
        && res.poster == 2
        && res.icon == 3 
        && res.releaseDate == 1600000000 
        && res.rating == 4.5) {

            foundId = res.id;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_EQ(foundId, gameId);

}


TEST(Game, LookupSingleGameByName) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 foundId = 0;

    nbl::SearchCriteria criteria = {
        .titleFilter = "Test Game"
    };


    ASSERT_TRUE(nbl::lookupGames(
        &connection, { .criteria = criteria }, 
        [&](const nbl::GameLookupResult& details) -> nbl::Result {

        if(details.title == "Test Game" 
        && details.poster == 2
        && details.icon == 3 
        && details.releaseDate == 1600000000 
        && details.rating == 4.5) {

            foundId = details.id;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_EQ(foundId, gameId);
}

TEST(Game, LookupSingleGameByTag) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 tId;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, tId).success());

    i64 foundId = 0;
    nbl::SearchCriteria criteria = {};

    nbl::Array tags = { tId };
    nbl::Array tagSets = { tags.constSpan() };

    criteria.tags = tagSets.span();

    ASSERT_TRUE(nbl::lookupGames(
        &connection, { .criteria = criteria }, 
        [&](const nbl::GameLookupResult& res) -> nbl::Result {

        if(res.title == "Test Game" 
        && res.poster == 2 
        && res.icon == 3 
        && res.releaseDate == 1600000000 
        && res.rating == 4.5) {

            foundId = res.id;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_EQ(foundId, gameId);

}
