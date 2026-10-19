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

#include <QObject>
#include <QtQml>

#include "Field/FieldList.hpp"

namespace nbl::frontend {

class AchievementStateModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:

    explicit AchievementStateModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum Roles {
        IdRole,
        NameRole,
        StatusRole,
        FieldCountRole,
        FieldValuesRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE FieldList* fields(quint32 index);

    QHash<int, QByteArray> roleNames() const override;

    qint64 gameId() const { return _mGameId; }
    void setGameId(qint64 id) { 
        _mGameId = id; 
        emit gameIdChanged();
        _refresh();
    }

signals:

    void gameIdChanged();

private:

    struct _Item {
        const nbl::AchievementProvider* provider;
        nbl::AchievementProviderStatus status;
        QByteArray encodedFields;
    };

    void _refresh();

    qint64 _mGameId;
    QList<_Item> _mData;

};

}
