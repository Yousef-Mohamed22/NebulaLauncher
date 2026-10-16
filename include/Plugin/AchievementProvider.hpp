#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Data/Database.hpp>
#include <Plugin/PluginContext.hpp>
#include <Data/Model.hpp>
#include <Platform/Time.hpp>
#include "Data/MediaManager.hpp"

namespace nbl {

class AchievementHelper {
public:

    AchievementHelper(DatabaseConnection* connection, StringView provider, i64 gameId) :
        _pConnection(connection),
        _mProvider(provider),
        _mGameId(gameId)
    { }

    Result setProviderStatus(AchievementProviderStatus status);

    Result hasAchievementSet(StringView id, bool* present);
    Result addOrUpdateAchievementSet(StringView id, StringView name, u64 lastUpdated = nbl::getCurrentTime(), ConstSpan<u8> providerData = {});
    template<typename T>
    Result addOrUpdateAchievementSet(StringView id, StringView name, u64 lastUpdated, const T& providerData) {
        return addOrUpdateAchievementSet(id, name, lastUpdated, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(&providerData), .length = sizeof(T) });
    }

    Result addAchievementSet(StringView id, StringView name, u64 lastUpdated = nbl::getCurrentTime(), ConstSpan<u8> providerData = {});

    template<typename T>
    Result addAchievementSet(StringView id, StringView name, u64 lastUpdated, const T& providerData) {
        return addAchievementSet(id, name, lastUpdated, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(&providerData), .length = sizeof(T) });
    }

    Result getSetProviderData(StringView id, void* dest, usize destSize);

    template<typename T>
    Result getSetProviderData(StringView id, T* dest) {
        return getSetProviderData(id, dest, sizeof(T));
    }

    Result updateSetProviderData(StringView id, ConstSpan<u8> providerData);
    inline Result updateSetProviderData(StringView id, StringView providerData) {
        return updateSetProviderData(id, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(providerData.data), .length = providerData.length });
    }
    template<typename T>
    inline Result updateSetProviderData(StringView id, const T& data) {
        return updateSetProviderData(id, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(&data), .length = sizeof(T) });
    }


    Result updateSetProviderData(i64 setId, ConstSpan<u8> providerData);
    inline Result updateSetProviderData(i64 setId, StringView providerData) {
        return updateSetProviderData(setId, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(providerData.data), .length = providerData.length });
    }
    template<typename T>
    Result updateSetProviderData(i64 setId, const T& data) {
        return updateSetProviderData(setId, ConstSpan<u8>{ .data = reinterpret_cast<const u8*>(&data), .length = sizeof(T) });
    }


    Result setAwardColor(i64 setId, u32 awardColor);

    Result dropAchievementSet(StringView id);
    Result dropUnknownSets(Span<StringView> knownSets);

    Result dropAllAchievements(i64 setId);
    Result dropStaleAchievements(i64 setId);
    Result markAchievementsStale(i64 setId);

    Result addOrUpdateAchievement(i64 setId, u64 number, StringView id, StringView name, StringView description, StringView icon, StringView lockedIcon, AchievementFlags flags, Span<u8> providerData = {});
    
    Result addOrUpdateAchievementWithProgress(i64 setId, u64 number, StringView id, StringView name, StringView description, StringView icon, StringView lockedIcon, f64 globalUnlockRate, u32 progressNumerator, u32 progressDenominator, u64 dateUnlocked, AchievementFlags flags, u32 borderColor, Span<u8> providerData = {});

    Result setAchievementProgress(i64 setId, StringView id, f64 globalUnlockRate, u32 progressNumerator, u32 progressDenominator, u64 dateUnlocked, u32 borderColor);

    u64 countAchievementSets();

    Result bumpSetUpdateTime(i64 setId, u64 time = nbl::getCurrentTime());

private:
    DatabaseConnection* _pConnection;
    MediaManager* _pMediaManager;
    StringView _mProvider;
    i64 _mGameId;
};

struct AchievementProvider {
    StringView id;
    StringView name;
    StringView requestedExternalId;
    Span<Field> fields;

    /**
     * \brief Called whenever a game is updated in the database.
     * \param context The global PluginContext
     * \param game The added/updated game
     * \param status The game's current active status for this provider
     * \param fieldValues The current values for the above fields. If the status states that the achievement provider is not added, then this will be empty.
     * \param helper An AchievementHelper designed to make the updating achievements easier
     * \param userData The same as the userData pointer in this struct
     * \return The result of the operation.
     */
    Result(*pfnUpdateAchievementState)(PluginContext* context, const GameMeta& game, AchievementProviderStatus status, Span<FieldValue> fieldValues, AchievementHelper& helper, StringView externalId, void* userData);

    /**
     * \brief Called asynchronously whenever a game is clicked in the UI, for each achievement set corresponding to this provider.
     * \param context The global PluginContext
     * \param gameId The ID of the set to consider
     * \param gameId The ID of the updated game
     * \param helper An AchievementHelper designed to make the updating achievements easier
     * \param userData The same as the userData pointer in this struct
     * \return The result of the operation. If the UI should not be refreshed, this should return nbl::Result::NO_CHANGE.
     */
    Result(*pfnUpdateAchievements)(PluginContext* context, const AchievementSet& set, AchievementHelper& helper, void* userData);

    /**
     * \brief The userData parameter passed to function pointers in this provider.
     */
    void* userData;

    Result updateAchievementState(PluginContext* context, const GameMeta& game, AchievementProviderStatus status, Span<FieldValue> fieldValues, AchievementHelper& helper, StringView externalId) const {
        if(!pfnUpdateAchievementState) return nbl::Result::PLUGIN_ERROR;
        return pfnUpdateAchievementState(context, game, status, fieldValues, helper, externalId, userData);
    }

    Result updateAchievements(PluginContext* context, const AchievementSet& set, AchievementHelper& helper) const {
        if(!pfnUpdateAchievements) return nbl::Result::PLUGIN_ERROR;
        return pfnUpdateAchievements(context, set, helper, userData);
    }

};


}
