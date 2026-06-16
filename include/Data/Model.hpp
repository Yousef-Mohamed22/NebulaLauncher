// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Model.hpp
 * \author jwall64
 * \brief Interface for interacting with the launcher's main data model
 */

#pragma once

#include <Data/Database.hpp>
#include <Codec/Json.hpp>

namespace nbl {

/**
 * \brief Enumeration for sorting options when looking up installations
 */
enum class InstallationSortBy {
    TITLE,
    LAST_PLAYED,
    ADDED,
    PLAY_COUNT,
    PLAY_TIME,
    RELEASE_DATE,
    RATING,
    _END
};

StringView getName(InstallationSortBy sortBy);
Result byName(StringView name, InstallationSortBy* outSortBy);


/**
 * \brief Enumeration for sorting options when looking up games
 */
enum class GameSortBy {
    TITLE,
    RELEASE_DATE,
    RATING,
    _END
};

StringView getName(GameSortBy sortBy);
Result byName(StringView name, GameSortBy* outSortBy);


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
    _END
};

inline LogoPosition byId(u32 id) {
    if(id >= static_cast<u32>(LogoPosition::_END)) {
        return LogoPosition::LEFT;
    }
    return static_cast<LogoPosition>(id);
}




/**
 * \brief Initializes the database schema if it doesn't already exist
 * \param connection The database connection to use for initialization
 * \return A Result indicating success or failure of the operation
 */
Result initDatabase(DatabaseConnection* connection);


//
// Tags
//

/**
 * \brief Callback type for tag traversal functions (descendants and ancestors)
 * \param userData A pointer to user-defined data that will be passed to the callback
 * \param tagName The name of the tag being processed
 * \param tagColor The color of the tag being processed, represented as a 32-bit unsigned integer (e.g., 0xRRGGBBAA)
 * \return A Result indicating success or failure of the callback operation. If the callback returns a failure Result, the traversal will be aborted.
 */
typedef Result (*TagCallback)(void* userData, StringView tagName, u32 tagColor);

/**
 * \brief Retrieves the descendant tags of a given tag and invokes a callback for each descendant
 * \param connection The database connection to use for the query
 * \param tag The name of the tag whose descendants are to be retrieved
 * \param callback The callback function to invoke for each descendant tag found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the traversal will be aborted and this function will return that failure Result.
 */
Result descendants(DatabaseConnection* connection, StringView tag, TagCallback callback, void* userData);

/**
 * \brief Retrieves the ancestor tags of a given tag and invokes a callback for each ancestor
 * \param connection The database connection to use for the query
 * \param tag The name of the tag whose ancestors are to be retrieved
 * \param callback The callback function to invoke for each ancestor tag found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the traversal will be aborted and this function will return that failure Result.
 */
Result ancestors(DatabaseConnection* connection, StringView tag, TagCallback callback, void* userData);

/**
 * \brief Adds a tag to the database with the specified name and color
 * \param connection The database connection to use for the query
 * \param tag The name of the tag whose color is to be retrieved
 * \param color The color of the tag, represented as a 32-bit unsigned integer (e.g., 0xRRGGBBAA)
 * \return A Result indicating success or failure of the operation
 */
Result addTag(DatabaseConnection* connection, StringView tag, u32 color);

/**
 * \brief Adds a parent-child relationship between two tags in the database
 * \param connection The database connection to use for the query
 * \param parentTag The name of the parent tag
 * \param childTag The name of the child tag
 * \return A Result indicating success or failure of the operation
 */
Result addParentTag(DatabaseConnection* connection, StringView parentTag, StringView childTag);

/**
 * \brief Removes a tag from the database, along with any parent-child relationships involving that tag
 * \param connection The database connection to use for the query
 * \param tag The name of the tag to be removed
 * \return A Result indicating success or failure of the operation
 */
Result removeTag(DatabaseConnection* connection, StringView tag);

/**
 * \brief Removes a parent-child relationship between two tags in the database
 * \param connection The database connection to use for the query
 * \param parentTag The name of the parent tag
 * \param childTag The name of the child tag
 * \return A Result indicating success or failure of the operation
 */
Result removeParentTag(DatabaseConnection* connection, StringView parentTag, StringView childTag);

/**
 * \brief Sets the color of a tag in the database
 * \param connection The database connection to use for the query
 * \param tag The name of the tag whose color is to be set
 * \param color The new color for the tag, represented as a 32-bit unsigned integer (e.g., 0xRRGGBBAA)
 * \return A Result indicating success or failure of the operation
 */
Result setTagColor(DatabaseConnection* connection, StringView tag, u32 color);

/**
 * \brief Renames a tag in the database
 * \param connection The database connection to use for the query
 * \param oldName The current name of the tag to be renamed
 * \param newName The new name for the tag
 * \return A Result indicating success or failure of the operation
 */
Result renameTag(DatabaseConnection* connection, StringView oldName, StringView newName);


//
// Display
//
struct DisplayMeta {
    i64 id;
    StringView banner;
    StringView poster;
    StringView icon;
    StringView logo;
    LogoPosition logoPosition;
};

typedef Result (*DisplayCallback)(void* userData, const DisplayMeta& meta);


Result addDisplay(DatabaseConnection* connection, const DisplayMeta& meta, i64* outDisplayId);

Result removeDisplay(DatabaseConnection* connection, i64 displayId);

Result updateDisplay(DatabaseConnection* connection, i64 displayId, const DisplayMeta& meta);

//
// Games
//

/**
 * \brief A struct representing the metadata of a game in the database
 */
struct GameMeta {
    i64 id;
    i64 displayId;
    StringView title;
    StringView description;
    u64 steamId;
    u64 igdbId;
    u64 releaseDate;
    f64 rating;
};

struct GameDetails {
    GameMeta game;
    DisplayMeta display;
};

struct GameLookupResult {
    i64 id;
    u64 releaseDate;
    f64 rating;
    StringView title;
    StringView poster;
    StringView icon;
};

struct GameSearchCriteria {
    JsonArray tags;
    JsonArray excludedTags;
    u64 minReleaseDate;
    u64 maxReleaseDate;
    f64 minRating;
    f64 maxRating;
    GameSortBy sortBy;
    SortOrder sortOrder;
    u64 offset;
    u64 limit;
    StringView titleFilter;
};

/**
 * \brief Callback type for game lookup functions
 * \param userData A pointer to user-defined data that will be passed to the callback
 * \param meta A GameMeta struct containing the metadata of the game being processed
 * \return A Result indicating success or failure of the callback operation. If the callback returns a failure Result, the lookup will be aborted.
 */
typedef Result (*GameMetaCallback)(void* userData, const GameMeta& meta);
typedef Result (*GameDetailsCallback)(void* userData, const GameDetails& meta);
typedef Result (*GameLookupCallback)(void* userData, const GameLookupResult& meta);

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


Result updateGame(DatabaseConnection* connection, i64 gameId, const GameMeta& meta);

/**
 * \brief Adds a tag to a game in the database
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game to which the tag will be added
 * \param tag The name of the tag to add to the game
 * \return A Result indicating success or failure of the operation
 */
Result addGameTag(DatabaseConnection* connection, i64 gameId, StringView tag);

/**
 * \brief Removes a tag from a game in the database
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game from which the tag will be removed
 * \param tag The name of the tag to remove from the game
 * \return A Result indicating success or failure of the operation
 */
Result removeGameTag(DatabaseConnection* connection, i64 gameId, StringView tag);


/**
 * \brief Retrieves the tags associated with a game and invokes a callback for each tag
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game whose tags are to be retrieved
 * \param callback The callback function to invoke for each tag found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 */
Result getGameTags(DatabaseConnection* connection, i64 gameId, TagCallback callback, void* userData);

/**
 * \brief Retrieves the metadata of a game by its ID and invokes a callback with the game's information
 * \param connection The database connection to use for the query
 * \param gameId The ID of the game whose metadata is to be retrieved
 * \param callback The callback function to invoke with the game's metadata
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation.
 */
Result getGameMeta(DatabaseConnection* connection, i64 gameId, GameMetaCallback callback, void* userData);

Result getGameDetails(DatabaseConnection* connection, i64 gameId, GameDetailsCallback callback, void* userData);

/**
 * \brief Looks up games in the database that match the specified criteria and invokes a callback for each matching game
 * \param connection The database connection to use for the query
 * \param criteria A GameSearchCriteria struct containing the criteria for filtering and sorting the games to be retrieved
 * \param callback The callback function to invoke for each matching game found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupGames(
        DatabaseConnection* connection, 
        const GameSearchCriteria& criteria,
        GameLookupCallback callback,
        void* userData);


//
// Installations
//

struct InstallationMeta {
    i64 id;
    i64 displayId;
    i64 gameId;
    StringView title;
    StringView description;
    StringView type;
    StringView command;
    u64 playTime;
    u64 playCount;
    u64 lastPlayed;
    u64 added;
};

struct InstallationLookupResult {
    i64 installationId;
    i64 displayId;
    i64 gameId;

    StringView title;
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
    DisplayMeta display;
    DisplayMeta gameDisplay;
};

struct InstallationSearchCriteria {
    JsonArray tags;
    JsonArray excludedTags;
    u64 minReleaseDate;
    u64 maxReleaseDate;
    f64 minRating;
    f64 maxRating;
    u64 minPlayTime;
    u64 maxPlayTime;
    InstallationSortBy sortBy;
    SortOrder sortOrder;
    u64 offset;
    u64 limit;
    StringView titleFilter;
};


/**
 * \brief Callback type for installation lookup functions
 * \param userData A pointer to user-defined data that will be passed to the callback
 * \return A Result indicating success or failure of the callback operation. If the callback returns a failure Result, the lookup will be aborted.
 */
typedef Result (*InstallationLookupCallback)(void* userData, const InstallationLookupResult& result);

typedef Result (*InstallationDetailsCallback)(void* userData, const InstallationDetails& details);

typedef Result (*InstallationMetaCallback)(void* userData, const InstallationMeta& meta);

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
Result getInstallationMeta(DatabaseConnection* connection, i64 installationId, InstallationMetaCallback callback, void* userData);

/**
 * \brief Retrieves all the details about an installation by its ID
 * \param connection The database connection to use for the query
 * \param installationId The ID of the installation whose details are to be retrieved
 * \param callback The callback function to invoke with the installation's details. 
 * \return A Result indicating success or failure of the operation.
 */
Result getInstallationDetails(DatabaseConnection* connection, i64 installationId, InstallationDetailsCallback callback, void* userData);

Result updateInstallation(DatabaseConnection* connection, const InstallationMeta& meta);

Result countInstallationTags(DatabaseConnection* connection, i64 installationId, u64* outCount);
Result getInstallationTags(DatabaseConnection* connection, i64 installationId, u64 offset, u64 limit, TagCallback callback, void* userData);

/**
 * \brief Looks up installations in the database that match the specified criteria and invokes a callback for each matching installation
 * \param connection The database connection to use for the query
 * \param criteria An InstallationSearchCriteria struct containing the criteria for filtering and sorting the installations to be retrieved
 * \param callback The callback function to invoke for each matching installation found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupInstallations(
        DatabaseConnection* connection, 
        const InstallationSearchCriteria& criteria,
        InstallationLookupCallback callback,
        void* userData); 


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
    StringView name;
    StringView icon;
    bool multiLine;
    bool showTitles;
    bool useIcons;
    InstallationSearchCriteria criteria;
};

typedef Result (*FilterCallback)(void* userData, const FilterMeta& meta);

Result countFilters(DatabaseConnection* connection, u64* numFilters);

Result addFilter(DatabaseConnection* connection, StringView name, StringView icon, const InstallationSearchCriteria& criteria, bool multiLine, i64* outFilterId);

Result removeFilter(DatabaseConnection* connection, i64 filterId);

Result getFilter(DatabaseConnection* connection, i64 filterId, FilterCallback callback, void* userData);

Result setFilterCriteria(DatabaseConnection* connection, i64 filterId, const InstallationSearchCriteria& criteria);

Result lookupInstallationsForFilter(DatabaseConnection* connection, i64 filterId, InstallationLookupCallback callback, u64 offset, u64 limit, void* userData);


//
// Tabs
//

typedef Result (*TabCallback)(void* userData, i64 tabId, StringView name, StringView icon, bool hidden);

Result countTabs(DatabaseConnection* connection, u64* numTabs);

Result addTab(DatabaseConnection* connection, StringView name, StringView icon, bool hidden, i64* outTabId);

Result removeTab(DatabaseConnection* connection, i64 tabId);

Result enumerateTabs(DatabaseConnection* connection, TabCallback callback, void* userData);

Result addFilterToTab(DatabaseConnection* connection, i64 tabId, i64 filterId, i64 priority);

Result removeFilterFromTab(DatabaseConnection* connection, i64 tabId, i64 filterId);

Result getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback, void* userData);



}

