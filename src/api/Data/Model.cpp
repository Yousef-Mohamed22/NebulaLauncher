// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Model.cpp
 * \author jwall64
 * \brief Partial Implementation of Data/Model.hpp. Functions related to the database schema and initialization.
 */

#include <Core/Result.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>
#include <Codec/Json.hpp>



nbl::StringView nbl::getName(SortBy sortBy) {
    switch(sortBy) {
        default:
        case SortBy::TITLE:
            return "title";
        case SortBy::LAST_PLAYED:
            return "last_played";
        case SortBy::ADDED:
            return "added";
        case SortBy::PLAY_COUNT:
            return "play_count";
        case SortBy::PLAY_TIME:
            return "play_time";
        case SortBy::RELEASE_DATE:
            return "release_date";
        case SortBy::RATING:
            return "rating";
        case SortBy::ACHIEVEMENT_PROGRESS:
            return "achievement_progress";
    }
}

nbl::Result nbl::byName(StringView name, SortBy* outSortBy) {
    for(auto i = 0 ; i < static_cast<i32>(SortBy::_END) ; i++) {
        if(name == getName(static_cast<SortBy>(i))) {
            *outSortBy = static_cast<SortBy>(i);
            return nbl::Result::SUCCESS;
        }
    }
    nbl::log(nbl::LogLevel::WARN, "Unknown installation sort by: ", name);
    return nbl::Result::INVALID_ARGUMENT;
}

nbl::StringView nbl::getName(SortOrder sortOrder) {
    switch(sortOrder) {
        default:
        case SortOrder::ASCENDING:
            return "ascending";
        case SortOrder::DESCENDING:
            return "descending";
    }
}

nbl::Result nbl::byName(StringView name, SortOrder* outSortOrder) {
    if(name == getName(SortOrder::ASCENDING)) {
        *outSortOrder = SortOrder::ASCENDING;
        return nbl::Result::SUCCESS;
    } else if(name == getName(SortOrder::DESCENDING)) {
        *outSortOrder = SortOrder::DESCENDING;
        return nbl::Result::SUCCESS;
    }
    nbl::log(nbl::LogLevel::WARN, "Unknown sort order: ", name);
    return nbl::Result::INVALID_ARGUMENT;
}


nbl::Result nbl::initDatabase(DatabaseConnection* connection) {

    i32 userVersion = 0;

    nbl::PreparedStatement stmt = {};
    NBL_CHECK(connection->prepare("PRAGMA user_version;", &stmt));

    NBL_CHECK(connection->execute(&stmt, [&](i32 argc, DataValue* argv, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        userVersion = argv[0].intValue;
        return nbl::Result::SUCCESS;
    }));

    nbl::log(nbl::LogLevel::DEBUG, "Database user_version: ", userVersion);

    if(userVersion == 0) {
        // Database is uninitialized

        nbl::log(nbl::LogLevel::DEBUG, "Initializing database...");
        nbl::StringView initQuery = R"(
PRAGMA journal_mode = WAL;
PRAGMA foreign_keys = ON;

BEGIN EXCLUSIVE TRANSACTION;
CREATE TABLE IF NOT EXISTS game (
    gId INTEGER PRIMARY KEY,
    title TEXT NOT NULL,
    description TEXT,
    poster TEXT,
    banner TEXT,
    icon TEXT,
    logo TEXT,
    logoPosition INTEGER NOT NULL DEFAULT 0,
    releaseDate INTEGER,
    rating REAL,
    previousPlayTime INTEGER,
    previousPlayCount INTEGER,
    lastPlayedDeleted INTEGER,
    added INTEGER
);
CREATE TABLE IF NOT EXISTS game_external_id (
    gId INTEGER,
    platformName TEXT,
    platformId TEXT,
    UNIQUE (gId, platformName),
    FOREIGN KEY(gId) REFERENCES game(gId) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS device (
    deviceId INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);
CREATE TABLE IF NOT EXISTS installation (
    iId INTEGER PRIMARY KEY,
    gId INTEGER NOT NULL,
    deviceId INTEGER,
    title TEXT,
    description TEXT,
    poster TEXT,
    banner TEXT,
    icon TEXT,
    logo TEXT,
    logoPosition INTEGER NOT NULL DEFAULT 4,
    type TEXT NOT NULL,
    command TEXT NOT NULL,
    playTime INTEGER NOT NULL DEFAULT 0,
    playCount INTEGER NOT NULL DEFAULT 0,
    added INTEGER NOT NULL,
    lastPlayed INTEGER,
    FOREIGN KEY(gId) REFERENCES game(gId),
    FOREIGN KEY(deviceId) REFERENCES device(deviceId)
);
CREATE TABLE IF NOT EXISTS tag (
    tId INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    color INTEGER NOT NULL DEFAULT 0xFFFFFFFF
);
CREATE UNIQUE INDEX idx_tag_name ON tag(name);

CREATE TABLE IF NOT EXISTS tag_closure (
    ancestorId INTEGER NOT NULL,
    descendantId INTEGER NOT NULL,
    depth INTEGER NOT NULL,
    PRIMARY KEY(ancestorId, descendantId),
    FOREIGN KEY(ancestorId) REFERENCES tag(tId),
    FOREIGN KEY(descendantId) REFERENCES tag(tId)
);
CREATE TABLE IF NOT EXISTS game_tag (
    gId INTEGER NOT NULL,
    tId INTEGER NOT NULL,
    PRIMARY KEY(gId, tId),
    FOREIGN KEY(gId) REFERENCES game(gId) ON DELETE CASCADE,
    FOREIGN KEY(tId) REFERENCES tag(tId) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS installation_tag (
    iId INTEGER NOT NULL,
    tId INTEGER NOT NULL,
    PRIMARY KEY(iId, tId),
    FOREIGN KEY(iId) REFERENCES installation(iId) ON DELETE CASCADE,
    FOREIGN KEY(tId) REFERENCES tag(tId) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS tab (
    tabId INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    icon TEXT NOT NULL,
    flags INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS filter (
    fId INTEGER PRIMARY KEY,
    tabId INTEGER NOT NULL,
    name TEXT NOT NULL,
    icon TEXT NOT NULL,
    criteria TEXT NOT NULL,
    flags INTEGER NOT NULL DEFAULT 0,
    priority INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(tabId) REFERENCES tab(tabId) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS plugin_fields (
    pluginId TEXT UNIQUE PRIMARY KEY,
    fields TEXT NOT NULL,
    version TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS achievement_set (
    asId INTEGER PRIMARY KEY,
    gId INTEGER NOT NULL,
    lastUpdated INTEGER DEFAULT 0,
    hidden BOOLEAN NOT NULL DEFAULT 0,
    awardColor INTEGER NOT NULL DEFAULT 0,
    name TEXT NOT NULL,
    providerId TEXT NOT NULL,
    provider TEXT NOT NULL,
    providerData BLOB,
    FOREIGN KEY(gId) REFERENCES game(gId) ON DELETE CASCADE,
    UNIQUE (gId, providerId, provider)
);

CREATE TABLE IF NOT EXISTS achievement (
    aId INTEGER PRIMARY KEY,
    asId INTEGER NOT NULL,
    providerId TEXT,
    title TEXT,
    description TEXT,
    icon TEXT,
    lockedIcon TEXT,
    globalUnlockRate REAL,
    progressNumerator INTEGER DEFAULT 0,
    progressDenominator INTEGER DEFAULT 0,
    dateUnlocked INTEGER,
    flags INTEGER DEFAULT 0,
    borderColor INTEGER DEFAULT 0,
    providerData BLOB,
    FOREIGN KEY(asId) REFERENCES achievement_set(asId) ON DELETE CASCADE,
    UNIQUE (providerId, asId)
);

CREATE TABLE IF NOT EXISTS achievement_state (
    gId INTEGER NOT NULL,
    provider TEXT NOT NULL,
    fields TEXT,
    status INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(gId) REFERENCES game(gId) ON DELETE CASCADE,
    UNIQUE (gId, provider)
);

CREATE TABLE IF NOT EXISTS online_account (
    oaId INTEGER PRIMARY KEY,
    fields TEXT,
    provider TEXT NOT NULL,
    updatedAt INTEGER DEFAULT 0,
    username TEXT,
    icon TEXT,
    iconNearestScaling INTEGER DEFAULT 0,
    status INTEGER DEFAULT 0
);

-- Default Rows
INSERT INTO tag
(tId, name, color)
VALUES
(1, 'favorite', 16018372);

INSERT INTO tag_closure
(ancestorId, descendantId, depth)
VALUES
(1, 1, 0);

INSERT INTO tab 
(tabId,name,icon,hidden) 
VALUES 
(1,'Home','',0);

INSERT INTO filter 
(fId,tabId,name,icon,criteria,multiLine,priority) 
VALUES
(1,1,'All Games','󰊗','{"sortBy":"title","sortOrder":"ascending"}',1,0),
(2,1,'Recently Played','','{"sortBy":"last_played","sortOrder":"descending"}',0,1);

INSERT INTO tab 
(tabId,name,icon,hidden) 
VALUES 
(2,'Favorites','',0);

INSERT INTO filter 
(fId,tabId,name,icon,criteria,multiLine,priority) 
VALUES
(3,2,'Favorites','','{"sortBy":"title","sortOrder":"ascending","tags":[[1]]}',1,0);

PRAGMA user_version = 1;
COMMIT;
)";

        NBL_CHECK(connection->execute(initQuery));

        return nbl::Result::SUCCESS;

    } else if(userVersion == 1) {

        nbl::StringView initQuery = "PRAGMA foreign_keys = ON;";
        NBL_CHECK(connection->execute(initQuery));

        return nbl::Result::SUCCESS;

    } else {
        nbl::log(nbl::LogLevel::ERROR, "Unsupported database version: ", userVersion);
        return nbl::Result::INVALID_STATE;
    }
}
