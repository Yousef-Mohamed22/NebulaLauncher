#include <Plugin/AchievementProvider.hpp>
#include <Data/Model.hpp>
#include <Platform/Time.hpp>
#include "Data/MediaManager.hpp"


nbl::Result nbl::AchievementHelper::setProviderStatus(nbl::AchievementProviderStatus status) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare("INSERT INTO achievement_state (gId, provider, status) VALUES (?1, ?2, ?3) ON CONFLICT (gId, provider) DO UPDATE SET status=?3", &_stmt);
    }
    NBL_CHECK(_stmt.bindAll(1, 
        _mGameId,
        _mProvider,
        static_cast<u32>(status)
    ));
    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::hasAchievementSet(StringView id, bool* present) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
SELECT COUNT(asId) FROM achievement_set WHERE provider=?1 AND providerId=?2 LIMIT 1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, _mProvider, id));
    return _pConnection->execute(&_stmt, [&](i32 numResults, nbl::DataValue* values, const char** names, i64 lastInsertRowId) -> nbl::Result {
        *present = true;
        return nbl::Result::SUCCESS;
    });

}

nbl::Result nbl::AchievementHelper::addOrUpdateAchievementSet(StringView id, StringView name, u64 lastUpdated, ConstSpan<u8> providerData) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
INSERT INTO achievement_set (gId, lastUpdated, hidden, name, provider, providerId, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)
ON CONFLICT(gId, providerId, provider) DO UPDATE
SET lastUpdated=?2, name=?3, providerData=?7;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        _mGameId,
        lastUpdated, 
        false,
        name,
        _mProvider, 
        id,
        providerData
    ));

    return _pConnection->execute(&_stmt);}

nbl::Result nbl::AchievementHelper::addAchievementSet(StringView id, StringView name, u64 lastUpdated, ConstSpan<u8> providerData) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
INSERT INTO achievement_set (gId, lastUpdated, hidden, name, provider, providerId, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)
ON CONFLICT(gId, providerId, provider) DO NOTHING;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        _mGameId,
        lastUpdated, 
        false,
        name,
        _mProvider, 
        id,
        providerData
    ));

    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::getSetProviderData(StringView id, void* dest, usize destSize) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare("SELECT providerData FROM achievement_set WHERE provider=?1 AND gId=?2 AND providerId=?3;", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        _mGameId,
        _mProvider, 
        id
    ));

    nbl::Result res = nbl::Result::SUCCESS;
    NBL_CHECK(_pConnection->execute(&_stmt, [&](i32 rows, DataValue* values, const char** names, i64) -> nbl::Result {
        if(values[0].blobValue.length != destSize) {
            res = nbl::Result::NO_CHANGE;
        } else {
            memcpy(dest, values[0].blobValue.data, destSize);
        }
        return nbl::Result::SUCCESS;
    }));
    return res;
}


nbl::Result nbl::AchievementHelper::updateSetProviderData(StringView id, ConstSpan<u8> providerData) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement_set SET providerData=?1 WHERE gId=?2 AND providerId=?3;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        providerData,
        _mGameId,
        _mProvider
    ));

    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::updateSetProviderData(i64 setId, ConstSpan<u8> providerData) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement_set SET providerData=?1 WHERE asId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        providerData,
        setId
    ));

    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::setAwardColor(i64 setId, u32 awardColor) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement_set SET awardColor=?1 WHERE asId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        awardColor,
        setId
    ));

    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::dropAchievementSet(StringView id) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
DELETE FROM achievement_set WHERE provider=?1 AND providerId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, _mProvider, id));
    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::dropUnknownSets(Span<StringView> knownSets) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
DELETE FROM achievement_set WHERE provider=?1 AND gId=?2 AND providerId NOT IN (SELECT value FROM json_each(?3));
        )", &_stmt);
    }

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringBuilder builder(&frame);
    nbl::JsonWriter(builder).writeArray([&](auto& array) {
        for(StringView set : knownSets) array.add(set);
        return Result::SUCCESS;
    });
    nbl::StringView json{};
    NBL_CHECK(builder.buildNullTerminated(&json));

    NBL_CHECK(_stmt.bindAll(1, _mProvider, _mGameId, json));
    return _pConnection->execute(&_stmt);

}

nbl::Result nbl::AchievementHelper::dropAllAchievements(i64 setId) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
DELETE FROM achievement WHERE asId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, setId));
    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::dropStaleAchievements(i64 setId) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
DELETE FROM achievement WHERE asId=?1 AND (flags & 128) != 0;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, setId));
    return _pConnection->execute(&_stmt);
}

nbl::Result nbl::AchievementHelper::markAchievementsStale(i64 setId) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement SET flags = (flags | 128) WHERE asId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, setId));
    return _pConnection->execute(&_stmt);
}


nbl::Result nbl::AchievementHelper::addOrUpdateAchievement(i64 setId, u64 number, StringView id, StringView name, StringView description, StringView icon, StringView lockedIcon, AchievementFlags flags, Span<u8> providerData) {

    i64 iconId;
    i64 lockedIconId;
    _pMediaManager->reserveReference(icon, nbl::ImageClass::ACHIEVEMENT_ICON, &iconId);
    _pMediaManager->reserveReference(icon, nbl::ImageClass::ACHIEVEMENT_ICON, &lockedIconId);

    Achievement ach = {
        .setId = setId,
        .number = number,
        .providerId = id,
        .title = name,
        .description = description,
        .icon = iconId,
        .lockedIcon = lockedIconId,
        .flags = flags,
        .providerData = providerData,
    };
    return nbl::upsertAchievementMeta(_pConnection, ach, &ach.id);
}

nbl::Result nbl::AchievementHelper::addOrUpdateAchievementWithProgress(i64 setId, u64 number, StringView id, StringView name, StringView description, StringView icon, StringView lockedIcon, f64 globalUnlockRate, u32 progressNumerator, u32 progressDenominator, u64 dateUnlocked, AchievementFlags flags, u32 borderColor, Span<u8> providerData) {

    i64 iconId;
    i64 lockedIconId;
    _pMediaManager->reserveReference(icon, nbl::ImageClass::ACHIEVEMENT_ICON, &iconId);
    _pMediaManager->reserveReference(icon, nbl::ImageClass::ACHIEVEMENT_ICON, &lockedIconId);

    nbl::Achievement ach = {
        .setId = setId,
        .number = number,
        .providerId = id,
        .title = name,
        .description = description,
        .icon = iconId,
        .lockedIcon = lockedIconId,
        .globalUnlockRate = globalUnlockRate,
        .progress = {
            .numerator = progressNumerator,
            .denominator = progressDenominator
        },
        .dateUnlocked = dateUnlocked,
        .flags = flags,
        .borderColor = borderColor,
        .providerData = providerData
    };

    return nbl::upsertAchievement(_pConnection, ach, nullptr);
}


nbl::Result nbl::AchievementHelper::setAchievementProgress(i64 setId, StringView id, f64 globalUnlockRate, u32 progressNumerator, u32 progressDenominator, u64 dateUnlocked, u32 borderColor) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement SET globalUnlockRate=?1, progressNumerator=?2, progressDenominator=?3, dateUnlocked=?4, borderColor=?5, flags = (flags & 127)
WHERE asId=?6 AND providerId=?7;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        globalUnlockRate,
        progressNumerator,
        progressDenominator,
        dateUnlocked,
        borderColor,
        setId,
        id));

    return _pConnection->execute(&_stmt);
}

u64 nbl::AchievementHelper::countAchievementSets() {
    u64 out = 0;
    nbl::countGameAchivementSetsWithProvider(_pConnection, _mGameId, _mProvider, &out);
    return out;
}

nbl::Result nbl::AchievementHelper::bumpSetUpdateTime(i64 setId, u64 time) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(_pConnection)) {
        _pConnection->prepare(R"(
UPDATE achievement_set SET lastUpdated=?1 WHERE asId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, time, setId));
    return _pConnection->execute(&_stmt);
}
