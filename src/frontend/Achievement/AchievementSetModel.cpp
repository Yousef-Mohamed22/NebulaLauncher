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

#include "Achievement/AchievementSetModel.hpp"
#include "ApplicationContext.hpp"
#include "Data/AchievementModel.hpp"

#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::AchievementSetModel::data(const QModelIndex& index, int role) const {

    qsizetype itemIndex = index.row();
    if(itemIndex > _mData.length()) return QVariant{};

    const _Item& item = _mData[itemIndex];

    switch(role) {
        case IdRole:
            return item.id;
        case GameIdRole:
            return item.gameId;
        case LastUpdatedRole:
            return item.lastUpdated;
        case HiddenRole:
            return item.hidden;
        case AwardColorRole:
            return item.awardColor;
        case NameRole:
            return item.name;
        case ProviderRole:
            return item.provider;
        case ProviderIdRole:
            return item.providerId;
        default:
            return QVariant{};
    }

}

QHash<int, QByteArray> nbl::frontend::AchievementSetModel::roleNames() const {
    QHash<int, QByteArray> out{};
    out[IdRole] = "id";
    out[GameIdRole] = "gameId";
    out[LastUpdatedRole] = "lastUpdated";
    out[HiddenRole] = "hidden";
    out[AwardColorRole] = "awardColor";
    out[NameRole] = "name";
    out[ProviderRole] = "provider";
    out[ProviderIdRole] = "providerId";
    return out;
}

nbl::Result nbl::frontend::AchievementSetModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    if(db == nullptr) return nbl::Result::INVALID_STATE;
    
    QList<_Item> found = {};
    found.reserve(limit);
    NBL_CHECK(nbl::AchievementSet::enumerateForGame(db, limit, offset, _mGameId, [&](const nbl::AchievementSet& set) -> nbl::Result {
        if(!_bIncludeHidden && set.hidden) return nbl::Result::SUCCESS;
        _Item item {
            .id = set.id,
            .gameId = set.gameId,
            .lastUpdated = set.lastUpdated,
            .hidden = set.hidden,
            .awardColor = QColor::fromRgba(set.awardColor),
            .name = fromNbl(set.name),
            .provider = fromNbl(set.provider),
            .providerId = fromNbl(set.providerId)
        };

        found.append(item);
        return nbl::Result::SUCCESS;
    }));

    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }
    return nbl::Result::SUCCESS;
}

quint64 nbl::frontend::AchievementSetModel::getTotalAchievementCount() {
    u64 out = 0;
    if(nbl::Achievement::countForGame(ApplicationContext::instance()->databaseConnection(), _mGameId, &out).error()) {
        return 0;
    }
    return static_cast<quint64>(out);
}
quint64 nbl::frontend::AchievementSetModel::getTotalUnlockedAchievementCount() {
    u64 out = 0;
    if(nbl::Achievement::countUnlockedForGame(ApplicationContext::instance()->databaseConnection(), _mGameId, &out).error()) {
        return 0;
    }
    return static_cast<quint64>(out);

}
quint64 nbl::frontend::AchievementSetModel::getSetAchievementCount(qint64 setId) {
    u64 out = 0;
    if(nbl::Achievement::countInSet(ApplicationContext::instance()->databaseConnection(), setId, &out).error()) {
        return 0;
    }
    return static_cast<quint64>(out);

}
quint64 nbl::frontend::AchievementSetModel::getSetUnlockedAchievementCount(qint64 setId) {
    u64 out = 0;
    if(nbl::Achievement::countUnlockedInSet(ApplicationContext::instance()->databaseConnection(), setId, &out).error()) {
        return 0;
    }
    return static_cast<quint64>(out);
}

bool nbl::frontend::AchievementSetModel::hasAnyWinCondition() {

    nbl::DatabaseConnection* conn = ApplicationContext::instance()->databaseConnection();
    static nbl::PreparedStatement _stmt{};
    if(!_stmt.isValid(conn)) {
        conn->prepare("SELECT EXISTS (SELECT 1 FROM achievement JOIN achievement_set ON achievement_set.asId = achievement.asId WHERE achievement_set.gId = ?1 AND dateUnlocked > 0 AND (flags & 2) != 0);", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, static_cast<i64>(_mGameId)));
    bool output = false;
    if(conn->execute(&_stmt, [&](i32 numResults, nbl::DataValue* values, const char** names, i64 lastInsertRowId) -> nbl::Result {
        output = numResults > 0 && values[0].uintValue() == 1;
        return nbl::Result::SUCCESS;
    }).error()) return false;

    return output;
}
bool nbl::frontend::AchievementSetModel::hasWinCondition(qint64 setId) {
 
    nbl::DatabaseConnection* conn = ApplicationContext::instance()->databaseConnection();
    static nbl::PreparedStatement _stmt{};
    if(!_stmt.isValid(conn)) {
        conn->prepare("SELECT EXISTS (SELECT 1 FROM achievement WHERE asId = ?1 AND dateUnlocked > 0 AND (flags & 2) != 0);", &_stmt);
    }

    NBL_CHECK(_stmt.bind(1, static_cast<i64>(setId)));
    bool output;
    if(conn->execute(&_stmt, [&](i32 numResults, nbl::DataValue* values, const char** names, i64 lastInsertRowId) -> nbl::Result {
        output = numResults > 0 && values[0].uintValue() == 1;
        return nbl::Result::SUCCESS;
    }).error()) return false;

    return output;

}
