#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/InstallationType.hpp>

#include <QObject>
#include <QtQml>

#include "Filter/FilterRef.hpp"

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
