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
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Tag/TagMeta.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class InstallationTagModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 installationId READ installationId WRITE setInstallationId NOTIFY installationIdChanged)

public:

    explicit InstallationTagModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mInstallationId(0)
    { }

    InstallationTagModel(i64 installationId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mInstallationId(installationId)
    { }

    enum Roles {
        IdRole,
        NameRole,
        ColorRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 installationId() const {
        return _mInstallationId;
    }

    void setInstallationId(qint64 id) {
        if(id != _mInstallationId) {
            _mInstallationId = id;
            emit installationIdChanged();
            refresh();
        }
    }

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

signals:

    void installationIdChanged();

private:

    void _fetchPageData();
    qint64 _mInstallationId;
    QList<TagMeta> _mData;

};

}
