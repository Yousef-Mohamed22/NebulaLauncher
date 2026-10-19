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


/**
 * \file Data/AchievementModel.cpp
 * \author jwall64
 * \brief Partial implementation of Data/Model.hpp
 */

#include <Data/Model.hpp>
#include <Core/Math.hpp>
#include "Utility/StringBuilder.hpp"

nbl::Result nbl::addAchievement(DatabaseConnection* connection, const Achievement& achievement, i64* outId) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement (asId, providerId, title, description, icon, lockedIcon, globalUnlockRate, progressNumerator, progressDenominator, dateUnlocked, flags, borderColor, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13);
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievement.setId, 
        achievement.providerId, 
        achievement.title, 
        achievement.description, 
        achievement.icon, 
        achievement.lockedIcon, 
        achievement.globalUnlockRate, 
        achievement.progress.numerator, 
        achievement.progress.denominator, 
        achievement.dateUnlocked, 
        achievement.flags,
        achievement.borderColor, 
        achievement.providerData));

    return connection->execute(&_stmt, outId);
}


nbl::Result nbl::upsertAchievement(DatabaseConnection* connection, const Achievement& achievement, i64* outId) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement (asId, providerId, title, description, icon, lockedIcon, globalUnlockRate, progressNumerator, progressDenominator, dateUnlocked, flags, borderColor, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13)
ON CONFLICT(providerId, asId) DO
UPDATE SET title=?3, description=?4, icon=?5, lockedIcon=?6, globalUnlockRate=?7, progressNumerator=?8, progressDenominator=?9, dateUnlocked=?10, flags=?11, borderColor=?12, providerData=?13;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievement.setId, 
        achievement.providerId, 
        achievement.title, 
        achievement.description, 
        achievement.icon, 
        achievement.lockedIcon, 
        achievement.globalUnlockRate, 
        achievement.progress.numerator, 
        achievement.progress.denominator, 
        achievement.dateUnlocked, 
        achievement.flags,
        achievement.borderColor, 
        achievement.providerData));

    return connection->execute(&_stmt, outId);

}

nbl::Result nbl::upsertAchievementMeta(DatabaseConnection* connection, const Achievement& achievement, i64* outId) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement (asId, providerId, number, title, description, icon, lockedIcon, flags, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)
ON CONFLICT(providerId, asId) DO
UPDATE SET number=?3, title=?4, description=?5, icon=?6, lockedIcon=?7, flags=?8, providerData=?9;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievement.setId, 
        achievement.providerId, 
        achievement.number,
        achievement.title, 
        achievement.description, 
        achievement.icon, 
        achievement.lockedIcon, 
        achievement.flags,
        achievement.providerData));

    return connection->execute(&_stmt, outId);

}



nbl::Result nbl::updateAchievement(DatabaseConnection* connection, const Achievement& achievement) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
UPDATE achievement SET asId=?1, providerId=?2, title=?3, description=?4, icon=?5, lockedIcon=?6, globalUnlockRate=?7, progressNumerator=?8, progressDenominator=?9, dateUnlocked=?10, flags=?11, borderColor=?12, providerData=?13
WHERE aId=?13;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievement.setId, 
        achievement.providerId, 
        achievement.title, 
        achievement.description, 
        achievement.icon, 
        achievement.lockedIcon, 
        achievement.globalUnlockRate, 
        achievement.progress.numerator, 
        achievement.progress.denominator, 
        achievement.dateUnlocked, 
        achievement.flags,
        achievement.borderColor, 
        achievement.providerData,
        achievement.id));

    return connection->execute(&_stmt);
}

nbl::Result nbl::updateAchievementProgress(DatabaseConnection* connection, const Achievement& achievement) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
UPDATE achievement SET globalUnlockRate=?1, progressNumerator=?2, progressDenominator=?3, dateUnlocked=?4, borderColor=?5, providerData=?6
WHERE aId=?7;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievement.globalUnlockRate, 
        achievement.progress.numerator, 
        achievement.progress.denominator, 
        achievement.dateUnlocked, 
        achievement.borderColor, 
        achievement.providerData,
        achievement.id));

    return connection->execute(&_stmt);

}

nbl::Result nbl::deleteAchievement(DatabaseConnection* connection, i64 id) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
DELETE FROM achievement WHERE aId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, id));

    return connection->execute(&_stmt);
}

nbl::Result nbl::getAchievement(DatabaseConnection* connection, i64 id, AchievementCallback callback) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
SELECT aId, asId, providerId, title, description, icon, IFNULL(lockedIcon, icon) as lockedIcon, type, globalUnlockRate, progressNumerator, progressDenominator, dateUnlocked, flags, borderColor, providerData 
FROM achievement 
WHERE aId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, id));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        return callback(Achievement{
            .id = values[0].intValue,
            .setId = values[1].intValue,
            .providerId = values[2].textValue,
            .title = values[3].textValue,
            .description = values[4].textValue,
            .icon = values[5].intValue,
            .lockedIcon = values[6].intValue,
            .progress = {
                .numerator = static_cast<u32>(values[7].uintValue()),
                .denominator = static_cast<u32>(values[8].uintValue())
            },
            .dateUnlocked = values[9].uintValue(),
            .flags = static_cast<AchievementFlags>(values[10].uintValue()),
            .borderColor = static_cast<u32>(values[11].uintValue()),
            .providerData = values[12].blobValue
        });
    });
}

nbl::Result nbl::enumerateSetAchievements(DatabaseConnection* connection, i64 limit, i64 offset, AchievementSortBy sortBy, SortOrder order, i64 setId, AchievementCallback callback) {

    static PreparedStatement _stmts[10]{};

    usize index = static_cast<u32>(sortBy) + (5 * static_cast<u32>(order));
    if(index > 9) {
        nbl::log(nbl::LogLevel::DEBUG, "Got invalid index: ", index);
        return nbl::Result::INVALID_ARGUMENT;
    }

    if(!_stmts[index].isValid(connection)) {

        AllocationFrame frame(Arena::threadLocal());
        StringBuilder builder(&frame);

        builder.append(R"(
SELECT aId, asId, number, providerId, title, description, icon, IFNULL(lockedIcon, icon) as lockedIcon, globalUnlockRate, progressNumerator, progressDenominator, dateUnlocked, flags, borderColor, providerData, 
(iif(flags & 4 AND dateUnlocked == 0, '?', title)) AS sortTitle, 
(iif(flags & 4 AND dateUnlocked == 0, 0, progressDenominator)) AS sortDenominator
FROM achievement 
WHERE asId=?1
ORDER BY )");
        switch(sortBy) {
            case AchievementSortBy::NUMBER:
                builder.append("number");
                break;
            case AchievementSortBy::NAME:
                builder.append("sortTitle");
                break;
            case AchievementSortBy::DATE_UNLOCKED:
                builder.append("dateUnlocked");
                break;
            case nbl::AchievementSortBy::PROGRESS:
                builder.append("iif(dateUnlocked > 0, 1.0, (SELECT iif(sortDenominator > 0, CAST(progressNumerator AS REAL)/sortDenominator, 0.0)))");
                break;
            case AchievementSortBy::GLOBAL_UNLOCK_RATE:
                builder.append("globalUnlockRate");
                break;
        }
        switch(order) { 
            case SortOrder::ASCENDING:
                builder.append(" ASC");
                break;
            case SortOrder::DESCENDING:
                builder.append(" DESC");
                break;
        }
        if(sortBy == nbl::AchievementSortBy::PROGRESS) {
            builder.append(", dateUnlocked");
            switch(order) { 
                case SortOrder::ASCENDING:
                    builder.append(" ASC");
                    break;
                case SortOrder::DESCENDING:
                    builder.append(" DESC");
                    break;
            }
        }

        builder.append(R"(
LIMIT ?2
OFFSET ?3;
        )");

        StringView statementStr;
        NBL_CHECK(builder.buildNullTerminated(&statementStr));
        NBL_CHECK(connection->prepare(statementStr, &_stmts[index]));
    }

    NBL_CHECK(_stmts[index].bindAll(1, setId, limit, offset));
    return connection->execute(&_stmts[index], [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        return callback(Achievement{
            .id = values[0].intValue,
            .setId = values[1].intValue,
            .number = values[2].uintValue(),
            .providerId = values[3].textValue,
            .title = values[4].textValue,
            .description = values[5].textValue,
            .icon = values[6].intValue,
            .lockedIcon = values[7].intValue,
            .globalUnlockRate = values[8].floatValue,
            .progress = {
                .numerator = static_cast<u32>(values[9].uintValue()),
                .denominator = static_cast<u32>(values[10].uintValue())
            },
            .dateUnlocked = values[11].uintValue(),
            .flags = static_cast<AchievementFlags>(values[12].uintValue()),
            .borderColor = static_cast<u32>(values[13].uintValue()),
            .providerData = values[14].blobValue
        });
    });
}

nbl::Result nbl::enumerateGameAchievements(DatabaseConnection* connection, i64 limit, i64 offset, AchievementSortBy sortBy, SortOrder order, i64 gameId, AchievementCallback callback) {

    static PreparedStatement _stmts[10]{};

    usize index = static_cast<u32>(sortBy) + (5 * static_cast<u32>(order));
    if(index > 9) return nbl::Result::INVALID_ARGUMENT;

    if(!_stmts[index].isValid(connection)) {

        AllocationFrame frame(Arena::threadLocal());
        StringBuilder builder(&frame);

        builder.append(R"(
SELECT aId, asId, number, providerId, title, description, icon, IFNULL(lockedIcon, icon) as lockedIcon, globalUnlockRate, progressNumerator, progressDenominator, dateUnlocked, flags, borderColor, providerData,
(iif(flags & 4 AND dateUnlocked == 0, '?', title)) AS sortTitle, 
(iif(flags & 4 AND dateUnlocked == 0, 0, progressDenominator)) AS sortDenominator 
FROM achievement 
JOIN achievement_set.asId = achievement.asId
WHERE achievement_set.gId=?1
ORDER BY )");
        switch(sortBy) {
            case AchievementSortBy::NUMBER:
                builder.append("number");
                break;
            case AchievementSortBy::NAME:
                builder.append("sortTitle");
                break;
            case AchievementSortBy::DATE_UNLOCKED:
                builder.append("dateUnlocked");
                break;
            case nbl::AchievementSortBy::PROGRESS:
                builder.append("iif(dateUnlocked > 0, 1.0, (SELECT iif(sortDenominator > 0, CAST(progressNumerator AS REAL)/sortDenominator, 0.0)))");
                break;
            case AchievementSortBy::GLOBAL_UNLOCK_RATE:
                builder.append("globalUnlockRate");
                break;
        }
        switch(order) { 
            case SortOrder::ASCENDING:
                builder.append(" ASC");
                break;
            case SortOrder::DESCENDING:
                builder.append(" DESC");
                break;
        }
        if(sortBy == nbl::AchievementSortBy::PROGRESS) {
            builder.append(", dateUnlocked");
            switch(order) { 
                case SortOrder::ASCENDING:
                    builder.append(" ASC");
                    break;
                case SortOrder::DESCENDING:
                    builder.append(" DESC");
                    break;
            }
        }

        builder.append(R"(
LIMIT ?2
OFFSET ?3;
        )");

        StringView statementStr;
        NBL_CHECK(builder.buildNullTerminated(&statementStr));

        connection->prepare(statementStr, &_stmts[index]);
    }

    NBL_CHECK(_stmts[index].bindAll(1, gameId, limit, offset));
    return connection->execute(&_stmts[index], [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

        return callback(Achievement{
            .id = values[0].intValue,
            .setId = values[1].intValue,
            .number = values[2].uintValue(),
            .providerId = values[3].textValue,
            .title = values[4].textValue,
            .description = values[5].textValue,
            .icon = values[6].intValue,
            .lockedIcon = values[7].intValue,
            .globalUnlockRate = values[8].floatValue,
            .progress = {
                .numerator = static_cast<u32>(values[9].uintValue()),
                .denominator = static_cast<u32>(values[10].uintValue())
            },
            .dateUnlocked = values[11].uintValue(),
            .flags = static_cast<AchievementFlags>(values[12].uintValue()),
            .borderColor = static_cast<u32>(values[13].uintValue()),
            .providerData = values[14].blobValue
        });
    });
}


nbl::Result nbl::countSetAchievements(DatabaseConnection* connection, i64 setId, u64* outNumAchievements) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare("SELECT COUNT(*) FROM achievement WHERE asId=?1;", &_stmt);
    }
    NBL_CHECK(_stmt.bind(1, setId));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 1) return nbl::Result::INVALID_ARGUMENT;
        *outNumAchievements = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::countUnlockedSetAchievements(DatabaseConnection* connection, i64 setId, u64* outNumAchievements) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare("SELECT COUNT(*) FROM achievement WHERE asId=?1 AND dateUnlocked > 0;", &_stmt);
    }
    NBL_CHECK(_stmt.bind(1, setId));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 1) return nbl::Result::INVALID_ARGUMENT;
        *outNumAchievements = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::countGameAchievements(DatabaseConnection* connection, i64 gameId, u64* outNumAchievements, bool includeHidden) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare("SELECT COUNT(*) FROM achievement JOIN achievement_set ON achievement_set.asId = achievement.asId WHERE achievement_set.gId=?1 AND (?2 = 1 OR achievement_set.hidden = 0);", &_stmt);
    }
    NBL_CHECK(_stmt.bindAll(1, gameId, includeHidden));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 1) return nbl::Result::INVALID_ARGUMENT;
        *outNumAchievements = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::countUnlockedGameAchievements(DatabaseConnection* connection, i64 gameId, u64* outNumAchievements, bool includeHidden) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare("SELECT COUNT(*) FROM achievement JOIN achievement_set ON achievement_set.asId = achievement.asId WHERE achievement_set.gId=?1 AND dateUnlocked > 0 AND (?2 = 1 OR achievement_set.hidden = 0);", &_stmt);
    }
    NBL_CHECK(_stmt.bindAll(1, gameId, includeHidden));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 1) return nbl::Result::INVALID_ARGUMENT;
        *outNumAchievements = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::addAchievementSet(DatabaseConnection* connection, const AchievementSet& achievementSet, i64* outId) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement_set (gId, lastUpdated, hidden, awardColor, name, provider, providerId, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievementSet.gameId,
        achievementSet.lastUpdated, 
        achievementSet.hidden, 
        achievementSet.awardColor,
        achievementSet.name,
        achievementSet.provider, 
        achievementSet.providerId,
        achievementSet.providerData
    ));

    return connection->execute(&_stmt, outId);

}

nbl::Result nbl::upsertAchievementSet(DatabaseConnection* connection, const AchievementSet& achievementSet, i64* outId) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement_set (gId, lastUpdated, hidden, awardColor, name, provider, providerId, providerData)
VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)
ON CONFLICT(gId, providerId, provider) DO 
UPDATE SET lastUpdated=?2, awardColor=?4, name=?5, providerData=?8;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievementSet.gameId,
        achievementSet.lastUpdated, 
        achievementSet.hidden,
        achievementSet.awardColor,
        achievementSet.name,
        achievementSet.provider, 
        achievementSet.providerId,
        achievementSet.providerData
    ));

    return connection->execute(&_stmt, outId);

}

nbl::Result nbl::updateAchivementSet(DatabaseConnection* connection, const AchievementSet& achievementSet) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
UPDATE achievement_set SET gId=?1, lastUpdated=?2, hidden=?3, awardColor=?4, name=?5, provider=?6, providerId=?7 providerData=?8
WHERE asId=?9;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        achievementSet.gameId,
        achievementSet.lastUpdated, 
        achievementSet.hidden,
        achievementSet.awardColor,
        achievementSet.name, 
        achievementSet.provider, 
        achievementSet.providerId, 
        achievementSet.providerData,
        achievementSet.id
    ));

    return connection->execute(&_stmt);
}

nbl::Result nbl::updateAchivementSetHidden(DatabaseConnection* connection, i64 id, bool hidden) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
UPDATE achievement_set SET hidden=?1
WHERE asId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, hidden, id));
    return connection->execute(&_stmt);
}



nbl::Result nbl::deleteAchievementSet(DatabaseConnection* connection, i64 id) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
DELETE FROM achievement_set WHERE asId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, id));

    return connection->execute(&_stmt);
}


nbl::Result nbl::deleteAchievementSetsForProvider(DatabaseConnection* connection, i64 gameId, StringView provider) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
DELETE FROM achievement_set WHERE provider=?1 AND gId=?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, provider, gameId));

    return connection->execute(&_stmt);
}

nbl::Result nbl::getAchievementSet(DatabaseConnection* connection, i64 id, AchievementSetCallback callback) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
SELECT asId, gId, lastUpdated, hidden, awardColor, name, provider, providerId, providerData 
FROM achievement_set 
WHERE asId=?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, id));
    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        AchievementSet set = {
            .id = values[0].intValue,
            .gameId = values[1].intValue,
            .lastUpdated = values[2].uintValue(),
            .hidden = values[3].intValue != 0,
            .awardColor = static_cast<u32>(values[4].uintValue()),
            .name = values[5].textValue,
            .provider = values[6].textValue,
            .providerId = values[7].textValue,
            .providerData = values[8].blobValue
        };
        return callback(set);
    });
}

nbl::Result nbl::enumerateGameAchievementSets(DatabaseConnection* connection, i64 limit, i64 offset, i64 gameId, AchievementSetCallback callback) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
SELECT asId, gId, lastUpdated, hidden, awardColor, name, provider, providerId, providerData 
FROM achievement_set
WHERE gId=?1
LIMIT ?2
OFFSET ?3;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, gameId, limit, offset));

    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        AchievementSet set = {
            .id = values[0].intValue,
            .gameId = values[1].intValue,
            .lastUpdated = values[2].uintValue(),
            .hidden = values[3].intValue != 0,
            .awardColor = static_cast<u32>(values[4].uintValue()),
            .name = values[5].textValue,
            .provider = values[6].textValue,
            .providerId = values[7].textValue,
            .providerData = values[8].blobValue
        };
        return callback(set);
    });
}


nbl::Result nbl::countGameAchivementSetsWithProvider(DatabaseConnection* connection, i64 gameId, StringView providerId, u64* outNumSets) {
    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
SELECT COUNT(asId) 
FROM achievement_set
WHERE gId=?1
  AND providerId = ?2;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, gameId, providerId));

    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        *outNumSets = values[0].uintValue();
        return nbl::Result::SUCCESS;
    });

}


nbl::Result nbl::enumerateAchievementStates(DatabaseConnection* connection, i64 gameId, AchievementStateCallback callback) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
SELECT gId, provider, fields, status
FROM achievement_state
WHERE gId = ?1;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindInt(1, gameId));

    return connection->execute(&_stmt, [&](i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        return callback({
            .gameId = values[0].intValue,
            .provider = values[1].textValue,
            .fields = values[2].textValue,
            .status = static_cast<AchievementProviderStatus>(values[3].uintValue())
        });
    });

}


nbl::Result nbl::upsertAchievementState(DatabaseConnection* connection, i64 gameId, const AchievementState& state) {

    static PreparedStatement _stmt{};
    if(!_stmt.isValid(connection)) {
        connection->prepare(R"(
INSERT INTO achievement_state (gId, provider, fields, status)
VALUES (?1, ?2, ?3, ?4)
ON CONFLICT(gId, provider) DO 
UPDATE SET fields=?3, status=?4;
        )", &_stmt);
    }

    NBL_CHECK(_stmt.bindAll(1, 
        state.gameId,
        state.provider, 
        state.fields,
        static_cast<u32>(state.status)
    ));

    return connection->execute(&_stmt);

}
