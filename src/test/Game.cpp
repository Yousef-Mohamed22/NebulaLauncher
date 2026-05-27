#include <gtest/gtest.h>

#include <Data/Model.hpp>


namespace {

nbl::GameMeta _gameMeta = {
    .id = 0,
    .title = "Test Game",
    .description = "A test game",
    .poster = "poster.jpg",
    .banner = "banner.jpg",
    .icon = "icon.jpg",
    .logo = "logo.jpg",
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

    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addGameTag(&connection, gameId, "Action").success());
}

TEST(Game, RemoveGameTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
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

    ASSERT_TRUE(nbl::getGameTags(&connection, gameId, [](void* userData, nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        _FoundData* found = static_cast<_FoundData*>(userData);
        if(tagName == "Action") {
            found->action = true;
        }
        if(tagName == "Adventure") {
            found->adventure = true;
        }
        return nbl::Result::SUCCESS;
    }, &found).success());

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
    ASSERT_TRUE(nbl::getGameMeta(&connection, gameId, [](void* userData, const nbl::GameMeta& meta) -> nbl::Result {
        if(meta.title == "Test Game" 
        && meta.description == "A test game" 
        && meta.poster == "poster.jpg" 
        && meta.banner == "banner.jpg" 
        && meta.icon == "icon.jpg" 
        && meta.logo == "logo.jpg" 
        && meta.releaseDate == 1600000000 
        && meta.rating == 4.5) {

            *static_cast<bool*>(userData) = true;
        }
        return nbl::Result::SUCCESS;
    }, &found).success());

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
        [](void* userData, const nbl::GameMeta& meta) -> nbl::Result {

        nbl::log(nbl::LogLevel::INFO, "Found game: ", meta.title);

        if(meta.title == "Test Game" 
        && meta.description == "A test game" 
        && meta.poster == "poster.jpg" 
        && meta.banner == "banner.jpg" 
        && meta.icon == "icon.jpg" 
        && meta.logo == "logo.jpg" 
        && meta.releaseDate == 1600000000 
        && meta.rating == 4.5) {

            *static_cast<i64*>(userData) = meta.id;
        }
        return nbl::Result::SUCCESS;
    }, &foundId).success());

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

    nbl::GameSearchCriteria criteria = {
        .titleFilter = "Test Game"
    };


    ASSERT_TRUE(nbl::lookupGames(
        &connection, criteria, 
        [](void* userData, const nbl::GameMeta& meta) -> nbl::Result {

        nbl::log(nbl::LogLevel::INFO, "Found game: ", meta.title);

        if(meta.title == "Test Game" 
        && meta.description == "A test game" 
        && meta.poster == "poster.jpg" 
        && meta.banner == "banner.jpg" 
        && meta.icon == "icon.jpg" 
        && meta.logo == "logo.jpg" 
        && meta.releaseDate == 1600000000 
        && meta.rating == 4.5) {

            *static_cast<i64*>(userData) = meta.id;
        }
        return nbl::Result::SUCCESS;
    }, &foundId).success());

    ASSERT_EQ(foundId, gameId);
}

TEST(Game, LookupSingleGameByTag) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
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
        [](void* userData, const nbl::GameMeta& meta) -> nbl::Result {

        if(meta.title == "Test Game" 
        && meta.description == "A test game" 
        && meta.poster == "poster.jpg" 
        && meta.banner == "banner.jpg" 
        && meta.icon == "icon.jpg" 
        && meta.logo == "logo.jpg" 
        && meta.releaseDate == 1600000000 
        && meta.rating == 4.5) {

            *static_cast<i64*>(userData) = meta.id;
        }
        return nbl::Result::SUCCESS;
    }, &foundId).success());

    ASSERT_EQ(foundId, gameId);

}
