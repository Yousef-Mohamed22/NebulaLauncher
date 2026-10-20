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

#include "Achievement/AchievementModel.hpp"
#include "ApplicationContext.hpp"

#include <Data/Model.hpp>

QVariant nbl::frontend::AchievementModel::data(const QModelIndex& index, int role) const {

    qsizetype itemIndex = index.row();
    if(itemIndex > _mData.length()) return QVariant{};

    const _Item& item = _mData[itemIndex];

    switch(role) {
        case IdRole:
            return item.id;
        case NumberRole:
            return item.number;
        case TitleRole:
            return item.title;
        case DescriptionRole:
            return item.description;
        case IconRole:
            return nbl::frontend::mediaUrl(item.icon);
        case LockedIconRole:
            return nbl::frontend::mediaUrl(item.lockedIcon);
        case GlobalUnlockRateRole:
            return item.globalUnlockRate;
        case NumeratorRole:
            return item.progress.numerator;
        case DenominatorRole:
            return item.progress.denominator;
        case DateUnlockedRole:
            return static_cast<quint64>(item.dateUnlocked);
        case FlagsRole:
            return item.flags;
        case BorderColorRole:
            return QColor::fromRgba(item.borderColor);
        default:
            return QVariant{};
    }

}

QHash<int, QByteArray> nbl::frontend::AchievementModel::roleNames() const {
    QHash<int, QByteArray> out{};
    out[IdRole] = "id";
    out[NumberRole] = "number";
    out[TitleRole] = "title";
    out[DescriptionRole] = "description";
    out[IconRole] = "icon";
    out[LockedIconRole] = "lockedIcon";
    out[GlobalUnlockRateRole] = "globalUnlockRate";
    out[NumeratorRole] = "numerator";
    out[DenominatorRole] = "denominator";
    out[DateUnlockedRole] = "dateUnlocked";
    out[FlagsRole] = "flags";
    out[BorderColorRole] = "borderColor";
    return out;
}

nbl::Result nbl::frontend::AchievementModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    if(db == nullptr) return nbl::Result::INVALID_STATE;
    
    QList<_Item> found = {};
    found.reserve(limit);
    NBL_CHECK(nbl::Achievement::enumerateInSet(db, limit, offset, _mSortBy, _mSortOrder, _mSetId, [&](const nbl::Achievement& a) -> nbl::Result {
        _Item item {
            .id = a.id,
            .number = a.number,
            .title = fromNbl(a.title),
            .description = fromNbl(a.description),
            .icon = a.icon,
            .lockedIcon = a.lockedIcon,
            .globalUnlockRate = a.globalUnlockRate,
            .progress = {
                .numerator = a.progress.numerator,
                .denominator = a.progress.denominator,
            },
            .dateUnlocked = a.dateUnlocked,
            .flags = a.flags,
            .borderColor = a.borderColor
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
