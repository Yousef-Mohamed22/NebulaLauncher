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

#include "Data/Model.hpp"
#include "Field/FieldList.hpp"

#include <Data/OnlineAccountModel.hpp>
#include <Plugin/AccountProvider.hpp>

namespace nbl::frontend {

class AccountModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString providerId READ providerId WRITE setProviderId NOTIFY providerIdChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(FieldList* fields READ fields CONSTANT)

public:


    explicit AccountModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pProvider(nullptr)
    { }



    explicit AccountModel(const AccountProvider* provider, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pProvider(provider)
    { 
        refresh();
    }

    enum : int {
        IdRole,
        UsernameRole,
        IconRole,
        IconIsImageRole,
        IconNearestScalingRole,
        StatusRole,
        FieldValuesRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return rowCount(); }

    FieldList* fields() const;

    QString providerId() const {
        if(_pProvider == nullptr) return {};
        return QString::fromUtf8(_pProvider->id.data(), _pProvider->id.length());
    }
    void setProviderId(QString providerId);

    Q_INVOKABLE qint32 login(QVariantMap fields);
    Q_INVOKABLE void logout(qint64 accountId);
    Q_INVOKABLE void refresh();

signals:

    void loginStatus(QString message);
    void loginFinished(bool success, QString message);

    void countChanged();
    void providerIdChanged();

private:

    struct _Row {
        qint64 id;
        QByteArray encodedFields;
        QString username;
        InlineIcon icon;
        OnlineAccountFlags flags;
        OnlineAccountStatus status;
    };

    Result _startLogin(i64 accountId, QVariantMap fields);

    const AccountProvider* _pProvider;
    QList<_Row> _mData;

};

}
