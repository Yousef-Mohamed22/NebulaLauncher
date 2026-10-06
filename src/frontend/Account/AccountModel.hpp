#pragma once

#include <QObject>
#include <QtQml>

#include "Models/NblFieldList.hpp"

#include <Data/OnlineAccountModel.hpp>
#include <Plugin/AccountProvider.hpp>

namespace nbl::frontend {

class AccountModel : public QAbstractListModel {

    Q_OBJECT
    QML_NAMED_ELEMENT(AccountModel)

    QML_UNCREATABLE("Obtain from AccountProviderModel")

    Q_PROPERTY(QString providerId READ providerId WRITE setProviderId NOTIFY providerIdChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(NblFieldList* fields READ fields CONSTANT)

public:

    explicit AccountModel(const AccountProvider* provider, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _pProvider(provider)
    { 
        refresh();
    }

    enum : int {
        IdRole,
        UsernameRole,
        IconRole,
        IconNearestScalingRole,
        StatusRole,
        FieldValuesRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return rowCount(); }

    NblFieldList* fields() const;

    QString providerId() const {
        if(_pProvider == nullptr) return {};
        return QString::fromUtf8(_pProvider->id.data, _pProvider->id.length);
    }
    void setProviderId(QString providerId);

    Q_INVOKABLE qint32 login(QVariantMap fields);
    Q_INVOKABLE void logout(qint64 accountId);
    Q_INVOKABLE void refresh();

signals:

    void loginStatus(QString message);
    void loginFinished(bool success, QString message);

    void countChanged();
    void providerIdChanged();

private:

    struct _Row {
        qint64 id;
        QByteArray encodedFields;
        QString username;
        QString icon;
        bool iconNearestScaling;
        OnlineAccountStatus status;
    };

    Result _startLogin(i64 accountId, QVariantMap fields);

    const AccountProvider* _pProvider;
    QList<_Row> _mData;

};

}
