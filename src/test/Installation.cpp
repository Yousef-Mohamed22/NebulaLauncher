#include <gtest/gtest.h>

#include <Data/Model.hpp>
#include <Data/Database.hpp>


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

nbl::InstallationMeta _installMeta = {
    .displayId = 0,
    .gameId = 0,
    .type = "dummy",
    .command = "C:/Games/Test Game"
};

}


TEST(Installation, AddInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());
}

TEST(Installation, RemoveInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());
    ASSERT_TRUE(nbl::removeInstallation(&connection, _installMeta.id).success());
}

TEST(Installation, GetInstallationMeta) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDisplay(&connection, _displayMeta, &_gameMeta.displayId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());

    bool found = false;
    ASSERT_TRUE(nbl::getInstallationDetails(&connection, _installMeta.id, [](void* userData, const nbl::InstallationDetails& details) -> nbl::Result {
        if(details.installation.command == "C:/Games/Test Game" && details.game.title == "Test Game") {
            *static_cast<bool*>(userData) = true;
        }
        return nbl::Result::SUCCESS;
    }, &found).success());

    ASSERT_TRUE(found);
}
