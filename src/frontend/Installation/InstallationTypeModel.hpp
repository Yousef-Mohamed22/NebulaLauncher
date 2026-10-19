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

#include "ApplicationContext.hpp"

namespace nbl::frontend {

class InstallationTypeModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

public:

    explicit InstallationTypeModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum Roles {
        IdRole,
        NameRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
        if(ctx == nullptr) return 0;
        return ctx->registry<nbl::InstallationType>().length;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(index.row() >= rowCount()) return QVariant();
        
        nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
        const nbl::InstallationType* type = ctx->registry<nbl::InstallationType>()[index.row()];

        switch(role) {
            case IdRole:
                return QByteArray(type->id.data, type->id.length);
            case NameRole:
                return QByteArray(type->name.data, type->name.length);
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[IdRole] = "id";
        roles[NameRole] = "name";
        return roles;
    }

    Q_INVOKABLE FieldList* fields(qsizetype index) const {
        nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
        ConstSpan<const InstallationType*> types = ctx->registry<nbl::InstallationType>();
        if(index < 0 || index >= types.length) return nullptr;
        return new FieldList(types[index]->fields);
    }

};

}
