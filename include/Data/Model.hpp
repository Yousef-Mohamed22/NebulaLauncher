// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Model.hpp
 * \author jwall64
 * \brief Interface for interacting with the launcher's main data model
 */

#pragma once

#include <Data/Database.hpp>
#include <Codec/Json.hpp>
#include <Core/Span.hpp>
#include <Core/Either.hpp>

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
    ACHIEVEMENT_PROGRESS,
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

enum class AchievementSortBy : u32 {
    NUMBER,             // The achievement's provider-determined index
    NAME,               // The achievement's display name
    DATE_UNLOCKED,      // The date the achievement was unlocked
    PROGRESS,           // How close the user is to completing the achievement
    GLOBAL_UNLOCK_RATE, // The percentage of players worldwide who have unlocked the achievement
};

enum class AchievementProviderStatus : u32 {
    NOT_ADDED,           // The achievement provider has not considered this game yet.
    ADDED_BY_PROVIDER,   // The achievement provdier has automatically determined that it can provide achievements for this game.
    ADDED_BY_USER,       // The user manually added information for this achievement provider and game.
    REMOVED_BY_PROVIDER, // The achievement provider has automatically determined that it cannot provide achievements for this game.
    REMOVED_BY_USER      // The user manually removed information for this achievement provider and game. Providers should not attempt to re-add achievement sets for states with this status.
};

union InlineIcon {
    i64 encoded;
    i64 imageId;
    struct {
        char data[4];
        u8 length;
    } utf;

    static InlineIcon fromDataValue(DataValue value) {
        if(value.type == DataType::INTEGER) {
            return { .encoded = value.intValue };
        } else if(value.type == DataType::TEXT) {
            usize length = min(value.textValue.length, static_cast<usize>(4));
            InlineIcon out{};
            memcpy(out.utf.data, value.textValue.data, length);
            out.utf.length = length;
            return out;
        } else {
            return { .encoded = 0 };
        }
    }
};


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
    u64 minAchievements;
    u64 maxAchievements;
    u64 minUnlockedAchievements;
    u64 maxUnlockedAchievements;
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
// Filters
// 

enum class FilterFlag : u64 {
    MULTI_LINE = 0x1,
    SHOW_TITLES = 0x2,
    USE_ICONS = 0x4,
    ONLY_INSTALLED = 0x8,
    SHOW_INSTALLATIONS = 0x10,
    COLORIZE_ICON = 0x20,
    SHOW_FILTER_INFO = 0x40,
    IMAGE_ICON = 0x80
};
typedef u64 FilterFlags;


inline FilterFlags operator&(FilterFlag flag, FilterFlag other) { return static_cast<FilterFlags>(flag) & static_cast<FilterFlags>(other); }
inline FilterFlags operator&(FilterFlag flag, FilterFlags other) { return static_cast<FilterFlags>(flag) & other; }
inline FilterFlags operator&(FilterFlags other, FilterFlag flag) { return static_cast<FilterFlags>(flag) & other; }

inline FilterFlags& operator&=(FilterFlags& flags, FilterFlag other) { flags = flags & other; return flags; }

inline FilterFlags operator|(FilterFlag flag, FilterFlag other) { return static_cast<FilterFlags>(flag) | static_cast<FilterFlags>(other); }
inline FilterFlags operator|(FilterFlag flag, FilterFlags other) { return static_cast<FilterFlags>(flag) | other; }
inline FilterFlags operator|(FilterFlags other, FilterFlag flag) { return static_cast<FilterFlags>(flag) | other; }

inline FilterFlags& operator|=(FilterFlags& flags, FilterFlag other) { flags = flags | other; return flags; }

struct FilterMeta {
    i64 filterId;
    i64 tabId;
    i64 priority;
    StringView name;
    InlineIcon icon;
    FilterFlags flags;
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

Result getFilterCriteria(DatabaseConnection* connection, i64 filterId, FilterCriteriaCallback callback);


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
// Games
//

/**
 * \brief A struct representing the metadata of a game in the database
 */
struct GameMeta {
    i64 id;
    StringView title;
    StringView description;
    i64 banner;
    i64 poster;
    i64 icon;
    i64 logo;
    LogoPosition logoPosition;
    u64 releaseDate;
    f64 rating;
    u64 previousPlayTime;
    u64 previousPlayCount;
    u64 lastPlayedDeleted;
    u64 added;
    Optional<u32> colorOverride;
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
    i64 poster;
    i64 icon;

    union {
        struct {
            u64 numAchievements;
            u64 numUnlockedAchievements;
        };
        u64 firstAdded;
        u64 playTime;
        u64 playCount;
        u64 lastPlayed;
    };

    const FilterMeta& filter;
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
typedef std::function_ref<Result(i64 gameId, StringView platformName, StringView platformId)> GameExternalIdCallback;

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


Result enumerateGameExternalIds(DatabaseConnection* connection, i64 gameId, GameExternalIdCallback callback);

Result getGameExternalId(DatabaseConnection* connection, i64 gameId, StringView platformName, GameExternalIdCallback callback);

Result setGameExternalId(DatabaseConnection* connection, i64 gameId, StringView platformName, StringView platformId);

Result removeGameExternalId(DatabaseConnection* connection, i64 gameId, StringView platformName);

Result clearGameExternalIds(DatabaseConnection* connection, i64 gameId);


Result getGameLastPlayed(DatabaseConnection* connection, i64 gameId, u64* outTime);
Result getGameFirstAdded(DatabaseConnection* connection, i64 gameId, u64* outTime);
Result getGamePlayCount(DatabaseConnection* connection, i64 gameId, u64* outCount);
Result getGamePlayTime(DatabaseConnection* connection, i64 gameId, u64* outTime);


/**
 * \brief Looks up games in the database that match the specified criteria and invokes a callback for each matching game
 * \param connection The database connection to use for the query
 * \param criteria A FilterMeta struct containing the criteria for filtering and sorting the games to be retrieved
 * \param callback The callback function ""to invoke for each matching game found
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupGames(
        DatabaseConnection* connection, 
        const FilterMeta& criteria,
        GameLookupCallback callback);


//
// Installations
//

struct InstallationMeta {
    i64 id;
    i64 gameId;
    i64 deviceId;
    StringView title;
    StringView description;
    i64 banner;
    i64 poster;
    i64 icon;
    i64 logo;
    LogoPosition logoPosition;
    StringView type;
    StringView command;
    u64 playTime;
    u64 playCount;
    u64 lastPlayed;
    u64 added;
    Optional<u32> colorOverride;
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

    i64 poster;
    i64 icon;

    u64 releaseDate;
    f64 rating;

    const FilterMeta& filter;
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
 * \param criteria A FilterMeta struct containing the criteria for filtering and sorting the installations to be retrieved
 * \param callback The callback function to invoke for each matching installation found
 * \param userData A pointer to user-defined data that will be passed to the callback function
 * \return A Result indicating success or failure of the operation. If the callback returns a failure Result, the lookup will be aborted and this function will return that failure Result.
 */
Result lookupInstallations(
        DatabaseConnection* connection, 
        const FilterMeta& criteria,
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

Result lookupInstallationsForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, InstallationLookupCallback callback);

Result lookupGamesForFilter(DatabaseConnection* connection, i64 filterId, i64 deviceId, u64 offset, u64 limit, GameLookupCallback callback);


//
// Tabs
//

enum class TabFlag : u64 {
    COLORIZE_ICON = 0x1,
    HIDDEN = 0x2,
    IMAGE_ICON = 0x4,
};

typedef u64 TabFlags;

inline TabFlags operator|(TabFlag flag, TabFlag other) { return static_cast<u8>(flag) | static_cast<u8>(other); }
inline TabFlags operator|(TabFlag flag, TabFlags other) { return static_cast<u8>(flag) | other; }
inline TabFlags operator|(TabFlags other, TabFlag flag) { return static_cast<u8>(flag) | other; }

inline TabFlags& operator|=(TabFlags& flags, TabFlag other) { flags = flags | other; return flags; }

inline TabFlags operator&(TabFlag flag, TabFlag other) { return static_cast<u8>(flag) & static_cast<u8>(other); }
inline TabFlags operator&(TabFlag flag, TabFlags other) { return static_cast<u8>(flag) & other; }
inline TabFlags operator&(TabFlags other, TabFlag flag) { return static_cast<u8>(flag) & other; }

inline TabFlags& operator&=(TabFlags& flags, TabFlag other) { flags = flags | other; return flags; }

struct TabMeta {
    i64 tabId;
    StringView name;
    InlineIcon icon;
    TabFlags flags;
};

typedef std::function_ref<Result(const TabMeta&)> TabCallback;

Result countTabs(DatabaseConnection* connection, u64* numTabs);

Result addTab(DatabaseConnection* connection, const TabMeta& meta, i64* outTabId);

Result updateTab(DatabaseConnection* connection, const TabMeta& meta);

Result removeTab(DatabaseConnection* connection, i64 tabId);

Result enumerateTabs(DatabaseConnection* connection, u64 offset, u64 limit, TabCallback callback);

Result getTabMeta(DatabaseConnection* connection, i64 tabId, TabCallback callback);

Result getFiltersForTab(DatabaseConnection* connection, i64 tabId, u64 offset, u64 limit, FilterCallback callback);


// Plugin Fields
struct PluginFields {
    StringView pluginId;
    StringView fieldsJson;
    StringView version;
};

typedef std::function_ref<Result(const PluginFields&)> PluginFieldsCallback;

Result getPluginFields(DatabaseConnection* connection, StringView pluginId, PluginFieldsCallback callback);

Result getPluginFieldsVersion(DatabaseConnection* connection, StringView pluginId, PluginFieldsCallback callback);

Result setPluginFields(DatabaseConnection* connection, StringView pluginid, StringView fieldsJson, StringView version);

Result prunePluginFields(DatabaseConnection* connection, JsonArray keepIds);


// Achievements

enum class AchievementFlag : u8 {
    PROGRESSION = 0b001,   // This achievement is required to beat the game
    WIN_CONDITION = 0b010, // Unlocking this achievement is the same as beating the game
    SPOILER = 0b100,       // This achievement's title and/or description would spoil the story for a new player
    MISSABLE = 0b1000,     // It is possible to put your save file in a state in which this achievement is no longer obtainable without restarting
    STALE = 0b10000000     // Used to easily identify achivements which have been removed from a set without iterating over all of them.
};
typedef u8 AchievementFlags;

inline AchievementFlags operator|(AchievementFlag flag, AchievementFlag other) { return static_cast<u8>(flag) | static_cast<u8>(other); }
inline AchievementFlags operator|(AchievementFlag flag, AchievementFlags other) { return static_cast<u8>(flag) | other; }
inline AchievementFlags operator|(AchievementFlags other, AchievementFlag flag) { return static_cast<u8>(flag) | other; }

inline AchievementFlags& operator|=(AchievementFlags& flags, AchievementFlag other) { flags = flags | other; return flags; }

inline AchievementFlags operator&(AchievementFlag flag, AchievementFlag other) { return static_cast<u8>(flag) & static_cast<u8>(other); }
inline AchievementFlags operator&(AchievementFlag flag, AchievementFlags other) { return static_cast<u8>(flag) & other; }
inline AchievementFlags operator&(AchievementFlags other, AchievementFlag flag) { return static_cast<u8>(flag) & other; }

inline AchievementFlags& operator&=(AchievementFlags& flags, AchievementFlag other) { flags = flags | other; return flags; }

struct Achievement {
    i64 id;
    i64 setId;
    u64 number;
    StringView providerId;
    StringView title;
    StringView description;
    i64 icon;
    i64 lockedIcon;
    f64 globalUnlockRate;
    struct {
        u32 numerator;
        u32 denominator;
    } progress;
    u64 dateUnlocked;
    AchievementFlags flags;
    u32 borderColor;
    ConstSpan<u8> providerData;

    template<typename T>
    void getProviderData(T* output) const {
        if(providerData.length != sizeof(T)) return;
        memcpy(output, providerData.data, sizeof(T));
    }
};

typedef std::function_ref<Result(const Achievement&)> AchievementCallback;


Result addAchievement(DatabaseConnection* connection, const Achievement& achievement, i64* outId);

Result upsertAchievement(DatabaseConnection* connection, const Achievement& achievement, i64* outId);

Result upsertAchievementMeta(DatabaseConnection* connection, const Achievement& achievement, i64* outId);

Result updateAchievement(DatabaseConnection* connection, const Achievement& achievement);

Result updateAchievementProgress(DatabaseConnection* connection, const Achievement& achievement);

Result deleteAchievement(DatabaseConnection* connection, i64 id);

Result getAchievement(DatabaseConnection* connection, i64 id, AchievementCallback callback);

Result enumerateGameAchievements(DatabaseConnection* connection, i64 limit, i64 offset, AchievementSortBy sortBy, SortOrder sortOrder, i64 gameId, AchievementCallback callback);

Result enumerateSetAchievements(DatabaseConnection* connection, i64 limit, i64 offset, AchievementSortBy sortBy, SortOrder sortOrder, i64 setId, AchievementCallback callback);

Result countSetAchievements(DatabaseConnection* connection, i64 setId, u64* outNumAchievements);

Result countUnlockedSetAchievements(DatabaseConnection* connection, i64 setId, u64* outNumAchievements);

Result countGameAchievements(DatabaseConnection* connection, i64 gameId, u64* outNumAchievements, bool includeHidden = false);

Result countUnlockedGameAchievements(DatabaseConnection* connection, i64 gameId, u64* outNumAchievements, bool includeHidden = false);



struct AchievementSet {
    i64 id;
    i64 gameId;
    u64 lastUpdated;
    bool hidden;
    u32 awardColor;
    StringView name;
    StringView provider;
    StringView providerId;
    ConstSpan<u8> providerData;

    template<typename T>
    void getProviderData(T* output) const {
        if(providerData.length != sizeof(T)) return;
        memcpy(output, providerData.data, sizeof(T));
    }
};

typedef std::function_ref<Result(const AchievementSet&)> AchievementSetCallback;

Result addAchievementSet(DatabaseConnection* connection, const AchievementSet& achievementSet, i64* outId);

Result upsertAchievementSet(DatabaseConnection* connection, const AchievementSet& achievement, i64* outId);

Result updateAchivementSet(DatabaseConnection* connection, const AchievementSet& achievementSet);

Result updateAchivementSetHidden(DatabaseConnection* connection, i64 id, bool hidden);

Result deleteAchievementSet(DatabaseConnection* connection, i64 id);

Result deleteAchievementSetsForProvider(DatabaseConnection* connection, i64 gameId, StringView provider);

Result getAchievementSet(DatabaseConnection* connection, i64 id, AchievementSetCallback callback);

Result enumerateGameAchievementSets(DatabaseConnection* connection, i64 limit, i64 offset, i64 gameId, AchievementSetCallback callback);

Result countGameAchivementSetsWithProvider(DatabaseConnection* connection, i64 gameId, StringView providerId, u64* outNumSets);


struct AchievementState {
    i64 gameId;
    StringView provider;
    StringView fields;
    AchievementProviderStatus status;
};

typedef std::function_ref<Result(const AchievementState&)> AchievementStateCallback;

Result enumerateAchievementStates(DatabaseConnection* connection, i64 gameId, AchievementStateCallback);

Result upsertAchievementState(DatabaseConnection* connection, i64 gameId, const AchievementState& state);

}

