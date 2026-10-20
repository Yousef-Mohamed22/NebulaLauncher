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
#include <QtQml>

#include "Filter/FilterMeta.hpp"
#include "PagedListModel.hpp"


namespace nbl::frontend {

class TabFilterModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tabId READ tabId WRITE setTabId NOTIFY tabIdChanged)

public:

    explicit TabFilterModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mTabId(0)
    { }

    TabFilterModel(i64 tabId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mTabId(tabId)
    { }

    enum Roles {
        IdRole,
        NameRole,
        IconRole,
        ColorizeRole,
        MultiLineRole,
        ShowTitlesRole,
        UseIconsRole,
        ShowInstallationsRole,
        ShowFilterInfoRole,
        IconIsImageRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 tabId() const {
        return _mTabId;
    }

    void setTabId(qint64 fid) {
        _mTabId = fid;
        emit tabIdChanged();
        refresh();
    }

signals:

    void tabIdChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    void _fetchPageData();

    qint64 _mTabId;

    QList<FilterMeta> _mData;

};

}
