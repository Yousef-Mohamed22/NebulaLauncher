#pragma once

#include <QObject>
#include <QtQml>

#include "Field/FieldList.hpp"

namespace nbl::frontend {

class AchievementStateModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:

    explicit AchievementStateModel(QObject* parent = nullptr) :
        QAbstractListModel(parent)
    { }

    enum Roles {
        IdRole,
        NameRole,
        StatusRole,
        FieldCountRole,
        FieldValuesRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE FieldList* fields(quint32 index);

    QHash<int, QByteArray> roleNames() const override;

    qint64 gameId() const { return _mGameId; }
    void setGameId(qint64 id) { 
        _mGameId = id; 
        emit gameIdChanged();
        _refresh();
    }

signals:

    void gameIdChanged();

private:

    struct _Item {
        const nbl::AchievementProvider* provider;
        nbl::AchievementProviderStatus status;
        QByteArray encodedFields;
    };

    void _refresh();

    qint64 _mGameId;
    QList<_Item> _mData;

};

}
