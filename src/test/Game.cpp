#include <gtest/gtest.h>

#include <Data/Model.hpp>


namespace {

nbl::DisplayMeta _displayMeta = {
    .id = 0,
    .banner = "banner.jpg",
    .poster = "poster.jpg",
    .icon = "icon.jpg",
    .logo = "logo.jpg",
};

nbl::GameMeta _gameMeta = {
    .id = 0,
    .title = "Test Game",
    .description = "A test game",
    .releaseDate = 1600000000,
    .rating = 4.5
};

}

TEST(Game, AddGame) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);
}

TEST(Game, RemoveGame) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);
    ASSERT_TRUE(nbl::removeGame(&connection, gameId).success());
}

TEST(Game, AddGameTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Action").success());
}

TEST(Game, RemoveGameTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());

    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Action").success());
    ASSERT_TRUE(nbl::removeGameTag(&connection, gameId, "Action").success());
}

TEST(Game, GetGameTags) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00).success());

    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Action").success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Adventure").success());

    struct _FoundData {
        bool action = false;
        bool adventure = false;
    } found;

    ASSERT_TRUE(nbl::getGameTags(&connection, gameId, [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Action") {
            found.action = true;
        }
        if(tagName == "Adventure") {
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

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);


    bool found = false;
    ASSERT_TRUE(nbl::getGameDetails(&connection, gameId, [&](const nbl::GameDetails& details) -> nbl::Result {
        if(details.game.title == "Test Game" 
        && details.game.description == "A test game" 
        && details.display.poster == "poster.jpg" 
        && details.display.banner == "banner.jpg" 
        && details.display.icon == "icon.jpg" 
        && details.display.logo == "logo.jpg" 
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

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 foundId = 0;
    ASSERT_TRUE(nbl::lookupGames(
        &connection, {}, 
        [&](const nbl::GameLookupResult& res) -> nbl::Result {

        if(res.title == "Test Game" 
        && res.poster == "poster.jpg" 
        && res.icon == "icon.jpg" 
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

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    i64 foundId = 0;

    nbl::GameSearchCriteria criteria = {
        .titleFilter = "Test Game"
    };


    ASSERT_TRUE(nbl::lookupGames(
        &connection, criteria, 
        [&](const nbl::GameLookupResult& details) -> nbl::Result {

        if(details.title == "Test Game" 
        && details.poster == "poster.jpg" 
        && details.icon == "icon.jpg" 
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

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    i64 gameId = 0;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());
    ASSERT_NE(gameId, 0);

    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Action").success());

    i64 foundId = 0;
    nbl::GameSearchCriteria criteria = {};

    ASSERT_TRUE(
        nbl::JsonArray::Builder()
            .add(nbl::JsonValue::fromString("Action"))
            .build(nullptr, &criteria.tags)
            .success());

    ASSERT_TRUE(nbl::lookupGames(
        &connection, criteria, 
        [&](const nbl::GameLookupResult& res) -> nbl::Result {

        if(res.title == "Test Game" 
        && res.poster == "poster.jpg" 
        && res.icon == "icon.jpg" 
        && res.releaseDate == 1600000000 
        && res.rating == 4.5) {

            foundId = res.id;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_EQ(foundId, gameId);

}
