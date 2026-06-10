#include <gtest/gtest.h>

#include <Data/Model.hpp>
#include <Data/Database.hpp>


namespace {

nbl::DisplayMeta _displayMeta = {
    .id = 0,
    .title = "Test Game",
    .description = "A test game",
    .banner = "banner.jpg",
    .poster = "poster.jpg",
    .icon = "icon.jpg",
    .logo = "logo.jpg",
};

nbl::GameMeta _gameMeta = {
    .id = 0,
    .releaseDate = 1600000000,
    .rating = 4.5
};

}


TEST(Installation, AddInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.dId));

    i64 gameId;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());

    i64 installationId;
    ASSERT_TRUE(nbl::addInstallation(&connection, gameId, "C:/Games/Test Game", &installationId).success());
}

TEST(Installation, RemoveInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.dId));

    i64 gameId;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());

    i64 installationId;
    ASSERT_TRUE(nbl::addInstallation(&connection, gameId, "C:/Games/Test Game", &installationId).success());

    ASSERT_TRUE(nbl::removeInstallation(&connection, installationId).success());
}

TEST(Installation, GetInstallationMeta) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.dId));

    i64 gameId;
    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &gameId).success());

    i64 installationId;
    ASSERT_TRUE(nbl::addInstallation(&connection, gameId, "C:/Games/Test Game", &installationId).success());

    bool found = false;
    ASSERT_TRUE(nbl::getInstallationMeta(&connection, installationId, [](void* userData, const nbl::InstallationMeta& meta) -> nbl::Result {
        if(meta.command == "C:/Games/Test Game" && meta.gameTitle == "Test Game") {
            *static_cast<bool*>(userData) = true;
        }
        return nbl::Result::SUCCESS;
    }, &found).success());

    ASSERT_TRUE(found);
}
