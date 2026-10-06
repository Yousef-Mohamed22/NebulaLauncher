#pragma once

#include <QObject>
#include <QtQml>

#include "AccountModel.hpp"

namespace nbl::frontend {

class AccountProviderModel : public QAbstractListModel {

    Q_OBJECT
    QML_NAMED_ELEMENT(AccountProviderModel)       

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
