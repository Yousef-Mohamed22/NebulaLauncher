#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>
#include <qjsvalue.h>
#include <qstringview.h>
#include <qtmetamacros.h>

#include <Models/NblFilterRef.hpp>
#include <Models/NblEnumFieldModel.hpp>
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
        NameRole,
        MinRole,
        MaxRole,
        DefaultRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        if(_mType == nullptr) return 0;
        return _mType->fields.length;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if(!index.isValid() || index.row() >= rowCount()) return QVariant();
        
        const nbl::Field* field = &_mType->fields[index.row()];
        if(index.column() > 0) {
            if(field->type == nbl::FieldType::LIST) {
                field = field->listData.templateField;
            } else if(field->type == nbl::FieldType::STRUCT) {
                if(index.column() >= field->structData.fields.length) return QVariant();
                field = &field->structData.fields[index.column()];
            }
        }


        switch(role) {
            case TypeRole:
                return static_cast<int>(field->type);
            case KeyRole:
                return QByteArray(field->key.data, field->key.length);
            case NameRole:
                return QByteArray(field->name.data, field->name.length);
            case MinRole: {
                if(field->type == nbl::FieldType::INTEGER) return static_cast<qint64>(field->integerData.min);
                if(field->type == nbl::FieldType::FLOAT) return field->floatData.min;
                return QVariant();
            }
            case MaxRole: {
                if(field->type == nbl::FieldType::INTEGER) return static_cast<qint64>(field->integerData.max);
                if(field->type == nbl::FieldType::FLOAT) return field->floatData.max;
                return QVariant();
            }
            case DefaultRole: {
                switch(field->type) {
                    case nbl::FieldType::STRING:
                        return QString::fromUtf8(field->defaultValue.stringValue.data, field->defaultValue.stringValue.length);
                    case nbl::FieldType::INTEGER:
                        return static_cast<qint64>(field->defaultValue.integerValue);
                    case nbl::FieldType::FLOAT:
                        return field->defaultValue.floatValue;
                    case nbl::FieldType::BOOLEAN:
                        return field->defaultValue.booleanValue;
                    case nbl::FieldType::ENUM: {
                        nbl::StringView name = field->enumData.name(field->defaultValue.enumValue);
                        return QString::fromUtf8(name.data, name.length);
                    }
                    default:
                        return QVariant();
                }
            }
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = {};
        roles[TypeRole] = "type";
        roles[KeyRole] = "key";
        roles[NameRole] = "name";
        roles[MinRole] = "min";
        roles[MaxRole] = "max";
        roles[DefaultRole] = "defaultValue";
        return roles;
    }

    Q_INVOKABLE NblEnumFieldModel* enumModel(quint32 index) {

        return new NblEnumFieldModel(index, _mTypeId, _mType);
    }

    Q_INVOKABLE void suggest(qsizetype index, QByteArray current, QJSValue callback) {

        if(!callback.isCallable()) return;

        const nbl::Field& field = _mType->fields[index];
        if(field.type != nbl::FieldType::STRING) return;

        nbl::StringView nblCurrent(current.data(), current.length());

        field.stringData.suggest(nblCurrent, [&](nbl::StringView suggestion) {
            QJSValueList args{};
            args << QJSValue(QString::fromUtf8(suggestion.data, suggestion.length));
            callback.call(args);
        });

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
        _mType = ApplicationContext::instance()->pluginContext()->getInstallationType(nbl::StringView(_mTypeId.data(), _mTypeId.length()));
        if(_mType && _mType->fields.length > 0) {
            beginInsertRows(QModelIndex(), 0, _mType->fields.length - 1);
            endInsertRows();
        }
        endResetModel();
    }

    const nbl::InstallationType* _mType;
    QByteArray _mTypeId;

};


