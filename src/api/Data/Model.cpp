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



nbl::StringView nbl::getName(InstallationSortBy sortBy) {
    switch(sortBy) {
        default:
        case InstallationSortBy::TITLE:
            return "title";
        case InstallationSortBy::LAST_PLAYED:
            return "last_played";
        case InstallationSortBy::ADDED:
            return "added";
        case InstallationSortBy::PLAY_COUNT:
            return "play_count";
        case InstallationSortBy::PLAY_TIME:
            return "play_time";
        case InstallationSortBy::RELEASE_DATE:
            return "release_date";
        case InstallationSortBy::RATING:
            return "rating";
    }
}

nbl::Result nbl::byName(StringView name, InstallationSortBy* outSortBy) {
    for(auto i = 0 ; i < static_cast<i32>(InstallationSortBy::_END) ; i++) {
        if(name == getName(static_cast<InstallationSortBy>(i))) {
            *outSortBy = static_cast<InstallationSortBy>(i);
            return nbl::Result::SUCCESS;
        }
    }
    nbl::log(nbl::LogLevel::WARN, "Unknown installation sort by: ", name);
    return nbl::Result::INVALID_ARGUMENT;
}

nbl::StringView nbl::getName(GameSortBy sortBy) {
    switch(sortBy) {
        default:
        case GameSortBy::TITLE:
            return "title";
        case GameSortBy::RELEASE_DATE:
            return "release_date";
        case GameSortBy::RATING:
            return "rating";
    }
}

nbl::Result nbl::byName(StringView name, GameSortBy* outSortBy) {
    for(auto i = 0 ; i < static_cast<i32>(GameSortBy::_END) ; i++) {
        if(name == getName(static_cast<GameSortBy>(i))) {
            *outSortBy = static_cast<GameSortBy>(i);
            return nbl::Result::SUCCESS;
        }
    }
    nbl::log(nbl::LogLevel::WARN, "Unknown game sort by: ", name);
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

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

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
CREATE TABLE IF NOT EXISTS display (
    dId INTEGER PRIMARY KEY AUTOINCREMENT,
    poster TEXT,
    banner TEXT,
    icon TEXT,
    logo TEXT,
    logoPosition INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS game (
    gId INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    steamId INTEGER,
    igdbId INTEGER,
    dId INTEGER NOT NULL,
    releaseDate INTEGER,
    rating REAL,
    FOREIGN KEY(dId) REFERENCES display(dId)
);
CREATE TABLE IF NOT EXISTS installation (
    iId INTEGER PRIMARY KEY AUTOINCREMENT,
    gId INTEGER NOT NULL,
    dId INTEGER,
    title TEXT,
    description TEXT,
    type TEXT NOT NULL,
    command TEXT NOT NULL,
    playTime INTEGER NOT NULL DEFAULT 0,
    playCount INTEGER NOT NULL DEFAULT 0,
    added INTEGER NOT NULL,
    lastPlayed INTEGER,
    FOREIGN KEY(gId) REFERENCES game(gId),
    FOREIGN KEY(dId) REFERENCES display(dId)
);
CREATE TABLE IF NOT EXISTS tag (
    tId INTEGER PRIMARY KEY AUTOINCREMENT,
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
    tabId INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    icon TEXT NOT NULL,
    colorizeIcon BOOLEAN DEFAULT 1,
    hidden BOOLEAN NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS filter (
    fId INTEGER PRIMARY KEY AUTOINCREMENT,
    tabId INTEGER NOT NULL,
    name TEXT NOT NULL,
    icon TEXT NOT NULL,
    colorizeIcon BOOLEAN NOT NULL DEFAULT 1,
    criteria TEXT NOT NULL,
    multiLine BOOLEAN NOT NULL DEFAULT 0,
    useIcons BOOLEAN NOT NULL DEFAULT 0,
    showTitles BOOLEAN NOT NULL DEFAULT 0,
    priority INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(tabId) REFERENCES tab(tabId) ON DELETE CASCADE
);

-- Default Rows
INSERT INTO tag
(name, color)
VALUES
('favorite', 16018372);

INSERT INTO tag_closure
(ancestorId, descendantId, depth)
VALUES
(last_insert_rowid(), last_insert_rowid(), 0);

INSERT INTO tab 
(tabId,name,icon,hidden) 
VALUES 
(1,'Home','',0);

INSERT INTO filter 
(fId,tabId,name,icon,criteria,multiLine,priority) 
VALUES
(1,1,'All Installations','󰊗','{"sortBy":"title","sortOrder":"ascending"}',1,0),
(2,1,'Recently Played','','{"sortBy":"last_played","sortOrder":"descending"}',0,1);

INSERT INTO tab 
(tabId,name,icon,hidden) 
VALUES 
(2,'Favorites','',0);

INSERT INTO filter 
(fId,tabId,name,icon,criteria,multiLine,priority) 
VALUES
(3,2,'Favorites','','{"sortBy":"title","sortOrder":"ascending","tags":[["favorite"]]}',1,0);

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
