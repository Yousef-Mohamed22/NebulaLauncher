/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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
