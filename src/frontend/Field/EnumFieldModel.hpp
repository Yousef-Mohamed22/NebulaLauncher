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
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>

namespace nbl::frontend {

class EnumFieldModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtain from FieldRef")

public:

    explicit EnumFieldModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pField(nullptr)
    { }

    EnumFieldModel(const nbl::Field* field, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pField(field)
    { }

    EnumFieldModel(EnumFieldModel&& other) noexcept :
        _pField(other._pField)
    {
        other._pField = nullptr;
    }

    EnumFieldModel& operator=(EnumFieldModel&& other) noexcept {
        if(&other == this) return *this;
        _pField = other._pField;
        other._pField = nullptr;
        return *this;
    }

    enum Roles {
        NameRole,
        DisplayNameRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE quint32 indexOf(QByteArray name);

signals:

    void typeIdChanged();
    void fieldIndexChanged();

private:

    const nbl::Field* _pField;


};

}
