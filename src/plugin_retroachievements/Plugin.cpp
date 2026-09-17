#include <Plugin/PluginInitializer.hpp>
#include <Plugin/AchievementProvider.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Network/HttpClient.hpp>

#include "Codec/Json.hpp"
#include "Core/String.hpp"
#include "Data/Model.hpp"
#include "Platform/Allocator.hpp"
#include "Platform/File.hpp"
#include "Platform/StringBuilder.hpp"
#include "Plugin/Field.hpp"

#include "Constants.hpp"
#include "Utility.hpp"
#include "Profile.hpp"

namespace {

nbl::HttpClient _httpClient;
nbl::ra::UserProfile _profile;

u8 _profileBuffer[4096];
nbl::InPlaceAllocator _profileAlloc(_profileBuffer, 4096);

nbl::Result _login(nbl::PluginContext* context, nbl::StringView username, nbl::StringView apiKey, nbl::LoginTracker* tracker, void* userData) {
    tracker->postStatus("Downloading profile...");

    NBL_CHECK(nbl::ra::downloadProfile(&_httpClient, apiKey, username, &_profileAlloc, &_profile));

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView apiKeyPath;

    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/retroachievements/auth.json").buildNullTerminated(&apiKeyPath));

    nbl::FileChannel channel{};
    NBL_CHECK(nbl::FileChannel::open(apiKeyPath, &channel, nbl::FileOpenFlag::WRITE | nbl::FileOpenFlag::CREATE | nbl::FileOpenFlag::CREATE_PARENTS));

    NBL_CHECK(nbl::ra::saveProfile(channel.writer(), _profile));
    tracker->finishTracking(true, {});

    return nbl::Result::SUCCESS;
}
nbl::Result _logout(nbl::PluginContext* context, u32 accountId, void* userData) {

    _profile.ulid = {};
    _profile.apiKey = {};
    _profile.username = {};
    _profile.pfp = {};

    _profileAlloc.popTo(0);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView apiKeyPath;

    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/retroachievements/auth.json").buildNullTerminated(&apiKeyPath));

    NBL_CHECK(nbl::deleteFile(apiKeyPath));
    return nbl::Result::SUCCESS;
}

nbl::Result _getAccount(nbl::PluginContext* context, u32 accountId, nbl::AccountInfo* info, void* userData) {

    if(accountId != 0 || !_profile.isValid()) return nbl::Result::NOT_FOUND;

    *info = {
        .userName = _profile.username,
        .icon = _profile.pfp,
        .id = 0,
        .iconNearestScaling = false,
    };
    return nbl::Result::SUCCESS;
}

nbl::Result _enumerateAccounts(nbl::PluginContext* context, nbl::AccountCallback callback, void* userData) {

    if(!_profile.isValid()) return nbl::Result::SUCCESS;
    return callback({
        .userName = _profile.username,
        .icon = _profile.pfp,
        .id = 0,
        .iconNearestScaling = false,
    });
}

nbl::AccountProvider _provider {
    .id = "ra",
    .name = "RetroAchievements",
    .type = nbl::AccountProvider::Type::BASIC,
    .maxAccounts = 1,
    .basicData = {
        .pfnLogin = _login,
        .pfnLogout = _logout,
        .userData = nullptr,
        .passwordText = "API Key"
    },
    .accountInfo = {
        .pfnGet = _getAccount,
        .pfnEnumerate = _enumerateAccounts
    }
};


struct _ProviderData {
    u64 latestUnlock;
    u64 numPlayers;
    char completionPercents[16];
    bool foundRealName;
    bool completed;
    bool hardcore;

    nbl::StringView userCompletion() {
        return nbl::StringView(completionPercents);
    }
    void setUserCompletion(nbl::StringView completion) {
        NBL_ASSERT(completion.length < 8);
        memcpy(completionPercents, completion.data, completion.length);
    }
    nbl::StringView hardcoreCompletion() {
        return nbl::StringView(completionPercents + 8);
    }
    void setHardcoreCompletion(nbl::StringView completion) {
        NBL_ASSERT(completion.length < 8);
        memcpy(completionPercents + 8, completion.data, completion.length);
    }
};

nbl::Result _updateAchievementState(nbl::PluginContext* context, const nbl::GameMeta& game, nbl::AchievementProviderStatus status, nbl::Span<nbl::FieldValue> values, nbl::AchievementHelper& helper, nbl::StringView externalId, void* userData) {

    if(status == nbl::AchievementProviderStatus::NOT_ADDED) {
        // TODO: Try to find the achievement set for this game. This will likely require obtaining an offline version of the database
        return helper.setProviderStatus(nbl::AchievementProviderStatus::REMOVED_BY_PROVIDER);
    }

    if(status != nbl::AchievementProviderStatus::ADDED_BY_USER 
    && status != nbl::AchievementProviderStatus::ADDED_BY_PROVIDER) return nbl::Result::SUCCESS;

    char* buffer[128];
    nbl::InPlaceAllocator alloc(buffer, 128);

    nbl::Array<nbl::StringView, 5> setNames{};
    usize index = 0;
    for(auto value : values[0].compositeValue) {
        NBL_CHECK(nbl::StringBuilder(&alloc).append(value.integerValue).buildNullTerminated(&setNames[index]));

        // TODO: Get this from an offline version of the database

        _ProviderData data{};
        NBL_CHECK(helper.getSetProviderData(setNames[index], &data));
        if(!data.foundRealName && _profile.isValid()) {

            nbl::ArenaFrame frame(nbl::Arena::threadLocal());

            nbl::StringView url;
            NBL_CHECK(nbl::StringBuilder(&frame)
                    .append(RA_API_URL)
                    .append("/API_GetGame.php?i=")
                    .append(setNames[index])
                    .append("&y=")
                    .append(_profile.apiKey)
                    .buildNullTerminated(&url));

            char* end = reinterpret_cast<char*>(frame.end());
            NBL_CHECK(_httpClient.awaitRequest(url, frame.writer()));

            nbl::StringView responseJson(end, reinterpret_cast<char*>(frame.end()) - end);
            nbl::JsonElement response{};
            NBL_CHECK(nbl::parseJson(nbl::StringReader(responseJson), &frame, &response));
            if(response.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

            nbl::StringView name = response.object.getString("Title");
            nbl::StringView gameTitle{};
            nbl::StringView subsetTitle{};

            data.foundRealName = true;

            NBL_CHECK(nbl::ra::parseTitle(name, [](nbl::StringView){}, &gameTitle, &subsetTitle)); 
            NBL_CHECK(helper.addOrUpdateAchievementSet(setNames[index], subsetTitle.empty() ? "Base Set" : subsetTitle, 0, data));

        } else {
            NBL_CHECK(helper.addAchievementSet(setNames[index], "RetroAchievements", 0));
        }

        index++;
    }

    NBL_CHECK(helper.dropUnknownSets(setNames));

    return nbl::Result::SUCCESS;
}

nbl::Result _updateAchievements(nbl::PluginContext* context, const nbl::AchievementSet& set, nbl::AchievementHelper& helper, void* userData) {

    if(!_profile.isValid()) return nbl::Result::NO_CHANGE;


    _ProviderData ud{};
    set.getProviderData(&ud);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView url;
    NBL_CHECK(nbl::StringBuilder(&frame)
        .append(RA_API_URL)
        .append("/API_GetGameInfoAndUserProgress.php?g=")
        .append(set.providerId)
        .append("&u=")
        .append(_profile.ulid)
        .append("&y=")
        .append(_profile.apiKey)
        .buildNullTerminated(&url));

    char* end = static_cast<char*>(frame.end());
    NBL_CHECK(_httpClient.awaitRequest(url, frame.writer()));

    nbl::StringView response(end, static_cast<char*>(frame.end()) - end);
    nbl::JsonElement responseJson{};

    NBL_CHECK(nbl::parseJson(nbl::StringReader(response), &frame, &responseJson));
    if(responseJson.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    u64 numPlayers = responseJson.object.getNumber("NumDistinctPlayers").asUint();

    nbl::JsonObject obj = responseJson.object.getObject("Achievements");
    if(obj.entryCount == 0) {
        nbl::log(nbl::LogLevel::DEBUG, "Achievement set: ", set.providerId, " has no achievements");
        return nbl::Result::NO_CHANGE;
    }

    bool updated = false;
    bool progressUpdated = false;

    u64 totalPlayers = responseJson.object.getNumber("NumDistinctPlayers").asUint();
    if(totalPlayers != ud.numPlayers) {

        updated = true;
        progressUpdated = true;
        ud.numPlayers = totalPlayers;

    } else {

        nbl::StringView userCompletion = responseJson.object.getString("UserCompletion");
        nbl::StringView hardcoreCompletion = responseJson.object.getString("UserCompletionHardcore");

        if(userCompletion != ud.userCompletion() || hardcoreCompletion != ud.hardcoreCompletion()) {
        
            updated = true;
            progressUpdated = true;
            ud.setUserCompletion(userCompletion);
            ud.setHardcoreCompletion(hardcoreCompletion);
        }
    }

    u64 numComplete = 0;
    u64 numHardcore = 0;

    u64 latestUpdate = 0;
    for(nbl::JsonObject::Entry& ent : obj) {

        nbl::StringView id = ent.key;
        if(ent.value.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        u64 time;
        NBL_CHECK(nbl::ra::parseDate(ent.value.object.getString("DateModified"), &time));

        if(latestUpdate < time) latestUpdate = time;

        nbl::StringView unlockDate = ent.value.object.getString("DateEarnedHardcore");
        bool hardcore = unlockDate.length > 0;
        bool unlocked = hardcore || (unlockDate = ent.value.object.getString("DateEarned")).length > 0;

        if(unlocked) numComplete++;
        if(hardcore) numHardcore++;

        u64 unlockTime = 0;
        if(unlockDate.length > 0) {
            NBL_CHECK(nbl::ra::parseDate(unlockDate, &unlockTime));
        }


        if(set.lastUpdated < time) {

            nbl::AchievementFlags flags = 0;
            nbl::StringView type = ent.value.object.getString("Type");
            if(type == "progression") {
                flags |= nbl::AchievementFlag::PROGRESSION;
            } else if(type == "win_condition") {
                flags |= nbl::AchievementFlag::WIN_CONDITION;
            } else if(type == "missable") {
                flags |= nbl::AchievementFlag::MISSABLE;
            }
            nbl::ArenaFrame frame(nbl::Arena::threadLocal());

            nbl::StringView badgeUrl{};
            nbl::StringBuilder(&frame)
                .append(RA_MEDIA_URL)
                .append("/Badge/")
                .append(ent.value.object.getString("BadgeName"))
                .append(".png")
                .buildNullTerminated(&badgeUrl);

            nbl::StringView descriptionWithPoints{};
            nbl::StringBuilder(&frame)
                .append(ent.value.object.getString("Description"))
                .append(" (").append(ent.value.object.getNumber("Points").asInt()).append(')')
                .buildNullTerminated(&descriptionWithPoints);

            helper.addOrUpdateAchievementWithProgress({
                .setId = set.id,
                .number = ent.value.object.getNumber("DisplayOrder").asUint(),
                .providerId = id,
                .title = ent.value.object.getString("Title"),
                .description = descriptionWithPoints,
                .icon = badgeUrl,
                .globalUnlockRate = 100.0 * ent.value.object.getNumber(hardcore ? "NumAwardedHardcore" : "NumAwarded").asFloat() / numPlayers,
                .progress = {},
                .dateUnlocked = unlockDate.length == 0 ? 0 : unlockTime, 
                .flags = flags,
                .borderColor = hardcore ? RA_BORDER_COLOR_HARDCORE : RA_BORDER_COLOR_CASUAL,
                .providerData = {}
            });
            updated = true;

        } else if(progressUpdated || ud.latestUnlock < unlockTime) { 

            ud.latestUnlock = unlockTime;

            helper.setAchievementProgress(
                    set.id, 
                    id,
                    100.0 * ent.value.object.getNumber(hardcore ? "NumAwardedHardcore" : "NumAwarded").asFloat() / numPlayers,
                    0, 0,
                    unlockDate.length == 0 ? 0 : unlockTime, 
                    hardcore ? RA_BORDER_COLOR_HARDCORE : RA_BORDER_COLOR_CASUAL);

            updated = true;
        }
    }

    if(numHardcore == obj.entryCount && !ud.hardcore) {
        ud.hardcore = true;
        ud.completed = true;
        updated = true;
        helper.setAwardColor(set.id, RA_BORDER_COLOR_HARDCORE);
    } else if(numComplete == obj.entryCount && !ud.completed) {
        ud.completed = true;
        updated = true;
        helper.setAwardColor(set.id, RA_BORDER_COLOR_CASUAL);
    }

    if(updated) {
        helper.updateSetProviderData(set.id, ud);
        helper.bumpSetUpdateTime(set.id, latestUpdate);
        return nbl::Result::SUCCESS;
    }

    return nbl::Result::NO_CHANGE;
}


nbl::Field _listTemplate = {
    .type = nbl::FieldType::INTEGER,
    .integerData = {
        .min = 0,
        .max = INT64_MAX
    }
};

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::LIST,
        .key = "ids",
        .name = "Game IDs",
        .listData = {
            .templateField = &_listTemplate,
            .min = 0,
            .max = 5
        }
    }
};


nbl::AchievementProvider _achProvider {
    .id = "ra",
    .name = "RetroAchievements",
    .requestedExternalId = "retro_achievements",
    .fields = _fields,
    .pfnUpdateAchievementState = _updateAchievementState,
    .pfnUpdateAchievements = _updateAchievements,
    .userData = nullptr
};

}


extern "C" i32 initPlugin(nbl::PluginInitContext* context) {  


    nbl::log(nbl::LogLevel::INFO, "Initializing RetroAchievements plugin");
    NBL_CHECK(context->registerObject(&_provider));
    NBL_CHECK(context->registerObject(&_achProvider));

    NBL_CHECK(nbl::HttpClient::create(1, 1, 0, 0, &_httpClient));

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView pluginFolder;
    nbl::StringView apiKeyPath;

    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/retroachievements").buildNullTerminated(&pluginFolder));
    NBL_CHECK(nbl::StringBuilder(&frame).append(pluginFolder).append("/auth.json").buildNullTerminated(&apiKeyPath));
    NBL_CHECK(nbl::makeDirectories(pluginFolder));

    nbl::FileChannel channel;
    nbl::Result res = nbl::FileChannel::open(apiKeyPath, &channel, nbl::FileOpenFlag::READ);

    if(res.error() && res != nbl::Result::NOT_FOUND) {
        return res;
    }

    if(res == nbl::Result::SUCCESS) {
        NBL_CHECK(nbl::ra::loadProfile(channel.reader(), &_profileAlloc, &_profile));
    }

    return 0; 
}
