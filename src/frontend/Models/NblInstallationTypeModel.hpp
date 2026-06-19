#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>

#include <Models/NblFilterRef.hpp>
#include "ApplicationContext.hpp"

class NblInstallationTypeModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

public:

    explicit NblInstallationTypeModel(QObject* parent = nullptr) :
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
        return ctx->types().length;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(index.row() >= rowCount()) return QVariant();
        
        nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
        const nbl::InstallationType& type = ctx->types().data[index.row()];

        switch(role) {
            case IdRole:
                return QByteArray(type.id.data, type.id.length);
            case NameRole:
                return QByteArray(type.name.data, type.name.length);
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

};


