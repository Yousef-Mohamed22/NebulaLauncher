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
#include <Data/DeviceModel.hpp>
#include <Data/GameModel.hpp>
#include <Data/InstallationModel.hpp>
#include <Data/Database.hpp>


namespace {

nbl::Game _gameMeta = {
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

nbl::Installation _installMeta = {
    .gameId = 0,
    .type = "dummy",
    .command = "C:/Games/Test Game"
};

}


TEST(Installation, AddInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::HostDevice::add(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::Game::add(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::Installation::add(&connection, _installMeta, &_installMeta.id).success());
}

TEST(Installation, RemoveInstallation) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::HostDevice::add(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::Game::add(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::Installation::add(&connection, _installMeta, &_installMeta.id).success());
    ASSERT_TRUE(nbl::Installation::remove(&connection, _installMeta.id).success());
}

TEST(Installation, GetInstallationMeta) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    ASSERT_TRUE(nbl::HostDevice::add(&connection, { .name = "test" }, &_installMeta.deviceId).success());

    ASSERT_TRUE(nbl::Game::add(&connection, _gameMeta, &_installMeta.gameId).success());
    ASSERT_TRUE(nbl::Installation::add(&connection, _installMeta, &_installMeta.id).success());

    bool found = false;
    ASSERT_TRUE(nbl::Installation::getExtended(&connection, _installMeta.id, [&](const nbl::Installation& installation, const nbl::Game& game) -> nbl::Result {
        if(installation.command == "C:/Games/Test Game" && game.title == "Test Game") {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}
