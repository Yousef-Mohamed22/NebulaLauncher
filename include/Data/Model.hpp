// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Model.hpp
 * \author jwall64
 * \brief Interface for interacting with the launcher's main data model
 */

#pragma once

#include <Data/Database.hpp>
#include <Codec/Json.hpp>
#include <functional>

namespace nbl {

/**
 * \brief Enumeration for sorting options when looking up installations
 */
enum class SortBy {
    TITLE,
    LAST_PLAYED,
    ADDED,
    PLAY_COUNT,
    PLAY_TIME,
    RELEASE_DATE,
    RATING,
    _END
};

StringView getName(SortBy sortBy);
Result byName(StringView name, SortBy* outSortBy);


/**
 * \brief Enumeration for sort order when looking things up in the database
 */
enum class SortOrder {
    ASCENDING,
    DESCENDING
};

StringView getName(SortOrder sortOrder);
Result byName(StringView name, SortOrder* outSortOrder);


enum class LogoPosition : u32 {
    LEFT,
    CENTER,
    RIGHT,
    HIDDEN,
    INHERIT,
    _END
};

inline LogoPosition byId(u32 id) {
    if(id >= static_cast<u32>(LogoPosition::_END)) {
        return LogoPosition::LEFT;
    }
    return static_cast<LogoPosition>(id);
}


struct SearchCriteria {
    ConstSpan<ConstSpan<i64>> tags;
    ConstSpan<i64> excludedTags;
    u64 minReleaseDate;
    u64 maxReleaseDate;
    f64 minRating;
    f64 maxRating;
    u64 minPlayTime;
    u64 maxPlayTime;
    SortBy sortBy;
    SortOrder sortOrder;
    u64 offset;
    u64 limit;
    StringView titleFilter;
    i64 gameId;
    i64 deviceId;
};


/**
 * \brief Initializes the database schema if it doesn't already exist
 * \param connection The database connection to use for initialization
 * \return A Result indicating success or failure of the operation
 */
Result initDatabase(DatabaseConnection* connection);


//
// Tags
//

struct TagMeta {
    i64 id;
    StringView name;
    u32 color;
};

typedef std::function<Result(const TagMeta&)> TagCallback;

Result getTagMeta(DatabaseConnection* connection, i64 tag, TagCallback callback);

Result descendants(DatabaseConnection* connection, i64 tag, TagCallback callback);
Result ancestors(DatabaseConnection* connection, i64 tag, TagCallback callback);
Result parents(DatabaseConnection* connection, i64 tag, TagCallback callback);

Result addTag(DatabaseConnection* connection, StringView tag, u32 color, i64* outTagId);
Result removeTag(DatabaseConnection* connection, i64 tag);

Result addParentTag(DatabaseConnection* connection, i64 parentTag, i64 childTag);
Result removeParentTag(DatabaseConnection* connection, i64 parentTag, i64 childTag);

Result setTagColor(DatabaseConnection* connection, i64 tag, u32 color);
Result renameTag(DatabaseConnection* connection, i64 tag, StringView newName);

Result enumerateTags(DatabaseConnection* connection, u32 limit, u32 offset, TagCallback callback);
Result searchTags(DatabaseConnection* connection, StringView query, u32 limit, u32 offset, TagCallback callback);
Result findTag(DatabaseConnection* connection, StringView name, TagCallback callback);

Result getTagsFromIds(DatabaseConnection* connection, StringView jsonIds, TagCallback callback);


//
// Games
//

/**
 * \brief A struct representing the metadata of a game in the database
 */
struct GameMeta {
    i64 id;
    StringView title;
    StringView description;
    StringView banner;
    StringView poster;
    StringView icon;
    StringView logo;
    LogoPosition logoPosition;
    u64 releaseDate;
    f64 rating;
    u64 previousPlayTime;
    u64 previousPlayCount;
    u64 lastPlayedDeleted;
    u64 added;
};

struct GameDetails {
    GameMeta game;
    u64 totalPlayTime;
    u64 totalPlayCount;
    u64 lastPlayed;
    u64 installationCount;
};

struct GameLookupResult {
    i64 id;
    StringView title;
    u64 releaseDate;
    f64 rating;
    StringView poster;
    StringView icon;
};

/**
 * \brief Callback type for game lookup functions
 * \param userData A pointer to user-defined data that will be passed to the callback
 * \param meta A GameMeta struct containing the metadata of the game being processed
 * \return A Result indicating success or failure of the callback operation. If the callback returns a failure Result, the lookup will be aborted.
 */
typedef std::function_ref<Result(const GameMeta&)> GameMetaCallback;
typedef std::function_ref<Result(const GameDetails&)> GameDetailsCallback;
typedef std::function_ref<Result(const GameLookupResult&)> GameLookupCallback;

/**
 * \brief Adds a game to the database with the specified information
 * \param connection The database connection to use for the query
 * \param meta A GameMeta struct containing the metadata of the game to be added (the gameId field will be ignored and overwritten with the newly assigned ID for the game in the database)
 * \param outGameId An output parameter that will be set to the ID of the newly added game in the database
 * \return A Result indicating success or failure of the operation
 */
Result addGame(DatabaseConnection* connection, const GameMeta& meta, i64* outGameId);

/**
 * \brief Removes a game from the database by its ID, along with any associated installations and tag relationships
 * \param gameId The ID of the game to be removed
 * \return A Result indicating success or failure of the operation
 */
Result removeGame(DatabaseConnection* connection, i64 gameId);


Result updateGame(DatabaseConnection* connection, const GameMeta& meta);

/**
 * \brief Adds a tag to a game in the database
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game to which the tag will be added
 * \param tag The name of the tag to add to the game
 * \return A Result indicating success or failure of the operation
 */
Result addGameTag(DatabaseConnection* connection, i64 gameId, i64 tag);

/**
 * \brief Removes a tag from a game in the database
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game from which the tag will be removed
 * \param tag The name of the tag to remove from the game
 * \return A Result indicating success or failure of the operation
 */
Result removeGameTag(DatabaseConnection* connection, i64 gameId, i64 tag);


/**
 * \brief Retrieves the tags associated with a game and invokes a callback for each tag
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game whose tags are to be retrieved
 * \param callback The callback function to invoke for each tag found
 */
Result getGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback);

/**
 * \brief Retrieves the metadata of a game by its ID and invokes a callback with the game's information
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game whose metadata is to be retrieved
 * \param callback The callback function to invoke with the game's metadata
 * \return A Result indicating success or failure of the operation.
 */
Result getGameMeta(DatabaseConnection* connection, i64 gameId, GameMetaCallback callback);

Result getGameDetails(DatabaseConnection* connection, i64 gameId, GameDetailsCallback callback);

/**
 * \brief Looks up games in the database that match the specified criteria and invokes a callback for each matching game
 * \param connection The database connection to use for the query
 * \param criteria A GameSearchCriteria struct containing the criteria for filtering and sorting the games to be retrieved
 * \param callback The callback function ""to invoke for each matching game found
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupGames(
        DatabaseConnection* connection, 
        const SearchCriteria& criteria,
        GameLookupCallback callback);

//
// Devices
//
struct DeviceMeta {
    i64 id; 
    StringView name;
};

typedef std::function_ref<Result(const DeviceMeta&)> DeviceCallback;

Result addDevice(DatabaseConnection* connection, DeviceMeta meta, i64* outId);
Result getDevice(DatabaseConnection* connection, i64 id, DeviceCallback callback);
Result removeDevice(DatabaseConnection* connection, i64 id);
Result updateDevice(DatabaseConnection* connection, DeviceMeta meta); 
Result getOrAddDevice(DatabaseConnection* connection, StringView hostname, i64* outId);

//
// Installations
//

struct InstallationMeta {
    i64 id;
    i64 gameId;
    i64 deviceId;
    StringView title;
    StringView description;
    StringView banner;
    StringView poster;
    StringView icon;
    StringView logo;
    LogoPosition logoPosition;
    StringView type;
    StringView command;
    u64 playTime;
    u64 playCount;
    u64 lastPlayed;
    u64 added;
};

struct InstallationLookupResult {
    i64 installationId;
    i64 gameId;
    i64 deviceId;

    StringView title;
    StringView gameTitle;
    u64 playTime;
    u64 playCount;
    u64 lastPlayed;
    u64 added;

    StringView poster;
    StringView icon;

    u64 releaseDate;
    f64 rating;
};

struct InstallationDetails {
    InstallationMeta installation;
    GameMeta game;
};


/**
 * \brief Callback type for installation lookup functions
 * \param userData A pointer to user-defined data that will be passed to the callback
 * \return A Result indicating success or failure of the callback operation. If the callback returns a failure Result, the lookup will be aborted.
 */
typedef std::function_ref<Result(const InstallationLookupResult& result)> InstallationLookupCallback;
typedef std::function_ref<Result(const InstallationDetails& details)> InstallationDetailsCallback;
typedef std::function_ref<Result(const InstallationMeta& meta)> InstallationMetaCallback;

/**
 * \brief Adds an installation of a game to the database with the specified information
 * \param gameId The ID of the game being installed
 * \param command The command or executable path used to launch the game
 * \param outInstallationId An output parameter that will be set to the ID of the newly added installation in the database
 * \return A Result indicating success or failure of the operation
 */
Result addInstallation(DatabaseConnection* connection, const InstallationMeta& meta, i64* outInstallationId);


/**
 * \brief Removes an installation from the database by its ID
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation to be removed
 * \return A Result indicating success or failure of the operation
 */
Result removeInstallation(DatabaseConnection* connection, i64 installationId);

/**
 * \brief Retrieves the metadata of an installation by its ID
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation whose metadata is to be retrieved
 * \param callback The callback function to invoke with the installation's metadata. 
 * \return A Result indicating success or failure of the operation.
 */
Result getInstallationMeta(DatabaseConnection* connection, i64 installationId, InstallationMetaCallback callback);

/**
 * \brief Retrieves all the details about an installation by its ID
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation whose details are to be retrieved
 * \param callback The callback function to invoke with the installation's details. 
 * \return A Result indicating success or failure of the operation.
 */
Result getInstallationDetails(DatabaseConnection* connection, i64 installationId, InstallationDetailsCallback callback);

Result updateInstallation(DatabaseConnection* connection, const InstallationMeta& meta);

Result addInstallationTag(DatabaseConnection* connection, i64 installationId, i64 tag);
Result removeInstallationTag(DatabaseConnection* connection, i64 installationId, i64 tag);

Result countInstallationTags(DatabaseConnection* connection, i64 installationId, u64* outCount);
Result getInstallationTags(DatabaseConnection* connection, i64 installationId, u64 offset, u64 limit, TagCallback callback);
Result getDirectInstallationTags(DatabaseConnection* connection, i64 installationId, TagCallback callback);

/**
 * \brief Looks up installations in the database that match the specified criteria and invokes a callback for each matching installation
 * \param connection The database connection to use for the query
 * \param criteria An SearchCriteria struct containing the criteria for filtering and sorting the installations to be retrieved
 * \param callback The callback function to invoke for each matching installation found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupInstallations(
        DatabaseConnection* connection, 
        const SearchCriteria& criteria,
        InstallationLookupCallback callback); 


Result getInstallationsOfGame(DatabaseConnection* connection, i64 gameId, i64 deviceId, u64 limit, u64 offset, InstallationLookupCallback callback);

/**
 * \brief Marks an installation as just played by updating its play count, and last played timestamp in the database
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation to be marked as just played
 * \return A Result indicating success or failure of the operation
 */
Result markInstallationJustPlayed(DatabaseConnection* connection, i64 installationId);

/**
 * \brief Increments the play time of an installation by a specified amount
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation whose play time is to be incremented
 * \param playTimeToAdd The amount of play time, in seconds, to add to the installation's existing play time
 * \return A Result indicating success or failure of the operation
 */
Result bumpInstallationPlayTime(DatabaseConnection* connection, i64 installationId, u64 playTimeToAdd);

//
// Filters
// 

struct FilterMeta {
    i64 filterId;
    i64 tabId;
    i64 priority;
    StringView name;
    StringView icon;
    bool colorizeIcon;
    bool multiLine;
    bool showTitles;
    bool useIcons;
    bool onlyInstalled;
    bool showInstallations;
    SearchCriteria criteria;
};

typedef std::function_ref<Result(const FilterMeta&)> FilterCallback;
typedef std::function_ref<Result(const SearchCriteria&)> FilterCriteriaCallback;

Result countFilters(DatabaseConnection* connection, u64* numFilters);

Result addFilter(DatabaseConnection* connection, const FilterMeta& meta, i64* outFilterId);

Result removeFilter(DatabaseConnection* connection, i64 filterId);

Result getFilter(DatabaseConnection* connection, i64 filterId, FilterCallback callback);

Result setFilterCriteria(DatabaseConnection* connection, i64 filterId, const SearchCriteria& criteria);

Result updateFilter(DatabaseConnection* connection, const FilterMeta& meta);

Result lookupInstallationsForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, InstallationLookupCallback callback);

Result lookupGamesForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, GameLookupCallback callback);

Result getFilterCriteria(DatabaseConnection* connection, i64 filterId, FilterCriteriaCallback callback);

//
// Tabs
//
struct TabMeta {
    i64 tabId;
    StringView name;
    StringView icon;
    bool colorizeIcon;
    bool hidden;
};

typedef std::function_ref<Result(const TabMeta&)> TabCallback;

Result countTabs(DatabaseConnection* connection, u64* numTabs);

Result addTab(DatabaseConnection* connection, const TabMeta& meta, i64* outTabId);

Result updateTab(DatabaseConnection* connection, const TabMeta& meta);

Result removeTab(DatabaseConnection* connection, i64 tabId);

Result enumerateTabs(DatabaseConnection* connection, u64 offset, u64 limit, TabCallback callback);

Result getTabMeta(DatabaseConnection* connection, i64 tabId, TabCallback callback);

Result getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback);

}

