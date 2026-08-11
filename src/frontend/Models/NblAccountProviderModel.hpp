#pragma once

#include <qtmetamacros.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/AccountProvider.hpp>

#include <QObject>
#include <QtQml>

#include "Models/NblAccountsModel.hpp"

class NblAccountProviderModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

public:

    explicit NblAccountProviderModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum Roles {
        IdRole,
        NameRole,
        MaxAccountsRole,
        AccountTypeRole,
        LoggingInRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void loginBasic(quint32 index, QString username, QString password);
    Q_INVOKABLE void loginOAuth(quint32 index);
    Q_INVOKABLE void logout(quint32 index, quint32 accountId);

    Q_INVOKABLE NblAccountsModel* accounts(quint32 index);

    QHash<int, QByteArray> roleNames() const override;

signals:

    void loginStatus(QString message);
    void loginFinished(bool success, QString message);

};


