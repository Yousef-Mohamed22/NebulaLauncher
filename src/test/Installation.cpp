#include <gtest/gtest.h>

#include <Data/Model.hpp>
#include <Data/Database.hpp>


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

nbl::InstallationMeta _installMeta = {
    .gameId = 0,
    .type = "dummy",
    .command = "C:/Games/Test Game"
};

}


TEST(Installation, AddInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDevice(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());
}

TEST(Installation, RemoveInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDevice(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());
    ASSERT_TRUE(nbl::removeInstallation(&connection, _installMeta.id).success());
}

TEST(Installation, GetInstallationMeta) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::addDevice(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::addGame(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::addInstallation(&connection, _installMeta, &_installMeta.id).success());

    bool found = false;
    ASSERT_TRUE(nbl::getInstallationDetails(&connection, _installMeta.id, [&](const nbl::InstallationDetails& details) -> nbl::Result {
        if(details.installation.command == "C:/Games/Test Game" && details.game.title == "Test Game") {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}
