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
#include <Core/Array.hpp>
#include <Core/Span.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Installation/InstallationRef.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class GameInstallationsModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:

    explicit GameInstallationsModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId{}
    { }

    GameInstallationsModel(qint64 gameId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(gameId)
    { }

    enum Roles {
        IdRole,
        TitleRole,
        GameTitleRole,
        PosterRole,
        IconRole,
        LastPlayedRole,
        ResolvedTitleRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 gameId() const {
        return _mGameId;
    }

    void setGameId(qint64 gameId) {
        _mGameId = gameId;
        emit gameIdChanged();
        refresh();
    }

    Q_INVOKABLE qint64 getId(qsizetype index) {
        return _mData[index].installationId();
    }

signals:

    void gameIdChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    qint64 _mGameId;
    QList<InstallationRef> _mData;

};

}
