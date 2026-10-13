#pragma once

#include <QObject>
#include <QtQml>

#include "Field/FieldList.hpp"

namespace nbl::frontend {

class PluginModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:

    explicit PluginModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum Roles {
        IdRole,
        NameRole,
        VersionRole,
        DescriptionRole,
        FieldCountRole,
        FieldValuesRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE FieldList* fields(int index);
    Q_INVOKABLE void saveFields(QVariantMap fields, int index);

};

}

