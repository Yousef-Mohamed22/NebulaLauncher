#pragma once

#include <qstringview.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>

#include <Models/NblFilterRef.hpp>
#include "ApplicationContext.hpp"

class NblFieldModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QByteArray typeId READ typeId WRITE setTypeId NOTIFY typeIdChanged)

public:

    explicit NblFieldModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mType(nullptr),
        _mTypeId("")
    { }

    enum Roles {
        TypeRole,
        KeyRole,
        NameRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        if(_mType == nullptr) return 0;
        return _mType->fields.length;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(index.row() >= rowCount()) return QVariant();
        
        const nbl::Field& field = _mType->fields[index.row()];

        switch(role) {
            case TypeRole:
                return static_cast<int>(field.type);
            case KeyRole:
                return QByteArray(field.key.data, field.key.length);
            case NameRole:
                return QByteArray(field.name.data, field.name.length);
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[TypeRole] = "type";
        roles[KeyRole] = "key";
        roles[NameRole] = "name";
        return roles;
    }

    QByteArray typeId() {
        return _mTypeId;
    }

    void setTypeId(QByteArray typeId) {
        _mTypeId = typeId;
        typeIdChanged();
        _reload();
    }

signals:

    void typeIdChanged();

private:

    void _reload() {
        beginResetModel();
        _mType = ApplicationContext::instance()->pluginContext()->getInstallationType(nbl::StringView(_mTypeId.length(), _mTypeId.data()));
        if(_mType->fields.length > 0) {
            beginInsertRows(QModelIndex(), 0, _mType->fields.length - 1);
            endInsertRows();
        }
        endResetModel();
    }

    const nbl::InstallationType* _mType;
    QByteArray _mTypeId;

};


