#pragma once

#include <qabstractitemmodel.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>

#include <Models/NblFilterRef.hpp>
#include "ApplicationContext.hpp"
#include "Core/String.hpp"

class NblEnumFieldModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint32 fieldIndex READ fieldIndex WRITE setFieldIndex NOTIFY fieldIndexChanged)
    Q_PROPERTY(QByteArray typeId READ typeId WRITE setTypeId NOTIFY typeIdChanged)

public:

    explicit NblEnumFieldModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mIndex(0),
        _mTypeId(""),
        _mType(nullptr)
    { }

    NblEnumFieldModel(quint32 index, QByteArray typeId, const nbl::InstallationType* type, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mIndex(index),
        _mTypeId(typeId),
        _mType(type) {

        _reload(); 
    }

    enum Roles {
        NameRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        if(_mType == nullptr || _mIndex >= _mType->fields.length) return 0;

        const nbl::Field& field = _mType->fields[_mIndex];
        
        return static_cast<int>(field.enumData.count());
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(!index.isValid() || index.row() >= rowCount() || _mType == nullptr) return QVariant();

        const nbl::Field& field = _mType->fields[_mIndex];
        if(field.type != nbl::FieldType::ENUM) return QVariant();

        switch(role) {
            case NameRole: {
                nbl::StringView name = field.enumData.name(static_cast<u32>(index.row()));
                return QByteArray(name.data, name.length);
            }
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[NameRole] = "name";
        return roles;
    }

    quint32 fieldIndex() const { return _mIndex; }
    QByteArray typeId() const { return _mTypeId; }

    Q_INVOKABLE quint32 indexOf(QByteArray name) {
        nbl::StringView nameView = nbl::StringView(name.data(), name.length());
        if(_mType == nullptr) return 0;
        return _mType->fields[_mIndex].enumData.index(nameView);
    }

    void setFieldIndex(quint32 index) {
        _mIndex = index;
        fieldIndexChanged();
        _reload();
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
    void fieldIndexChanged();

private:

    void _reload() {
        beginResetModel();
        _mType = ApplicationContext::instance()->pluginContext()->getInstallationType(nbl::StringView(_mTypeId.data(), _mTypeId.length()));
        int count = rowCount();
        if(count > 0) {
            beginInsertRows(QModelIndex(), 0, count - 1);
            endInsertRows();
        }
        endResetModel();
    }

    quint32 _mIndex;
    QByteArray _mTypeId;
    
    const nbl::InstallationType* _mType;

};


