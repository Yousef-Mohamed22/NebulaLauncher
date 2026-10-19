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

#include "AccountModel.hpp"

namespace nbl::frontend {

class AccountProviderModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT       

    Q_PROPERTY(int count READ count CONSTANT)

public:

    explicit AccountProviderModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum : int {
        IdRole,
        NameRole,
        LoginTextRole,
        MaxAccountsRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return rowCount(); }

    Q_INVOKABLE AccountModel* accountModel(int row);

};

}
