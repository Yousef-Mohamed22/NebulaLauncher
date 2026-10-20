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

#pragma once

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>
#include <Data/AchievementModel.hpp>

#include <QObject>
#include <QtQml>

#include "PagedListModel.hpp"


namespace nbl::frontend {


class AchievementModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 setId READ setId WRITE setSetId NOTIFY setIdChanged)
    Q_PROPERTY(quint32 sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    Q_PROPERTY(quint32 sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

public:

    AchievementModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mSetId(0),
        _mSortBy(nbl::AchievementSortBy::NUMBER),
        _mSortOrder(nbl::SortOrder::ASCENDING)
    { }

    enum Roles {
        IdRole,
        NumberRole,
        TitleRole,
        DescriptionRole,
        IconRole,
        LockedIconRole,
        GlobalUnlockRateRole,
        NumeratorRole,
        DenominatorRole,
        DateUnlockedRole,
        FlagsRole,
        BorderColorRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 setId() const {
        return _mSetId;
    }

    quint32 sortBy() const {
        return static_cast<quint32>(_mSortBy);
    }

    quint32 sortOrder() const {
        return static_cast<quint32>(_mSortOrder);
    }

    void setSetId(qint64 id) {
        _mSetId = id;
        emit setIdChanged();
        refresh();
    }

    void setSortBy(quint32 sortBy) {
        if(sortBy > 4) return;
        _mSortBy = static_cast<nbl::AchievementSortBy>(sortBy);
        emit sortByChanged();
        refresh();
    }

    void setSortOrder(quint32 sortOrder) {
        if(sortOrder > 1) return;
        _mSortOrder = static_cast<nbl::SortOrder>(sortOrder);
        emit sortOrderChanged();
        refresh();
    }

signals:

    void setIdChanged();
    void sortByChanged();
    void sortOrderChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    struct _Item {
        qint64 id;
        quint64 number;
        QString title;
        QString description;
        qint64 icon;
        qint64 lockedIcon;
        f64 globalUnlockRate;
        struct {
            u32 numerator;
            u32 denominator;
        } progress;
        u64 dateUnlocked;
        nbl::AchievementFlags flags;
        u32 borderColor;
    };

    qint64 _mSetId;
    nbl::AchievementSortBy _mSortBy;
    nbl::SortOrder _mSortOrder;
    QList<_Item> _mData;

};

}
