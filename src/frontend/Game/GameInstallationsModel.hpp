#pragma once

#include <Core/Types.hpp>
#include <Core/Array.hpp>
#include <Core/Span.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Installation/InstallationRef.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class GameInstallationsModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:

    explicit GameInstallationsModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId{}
    { }

    GameInstallationsModel(qint64 gameId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(gameId)
    { }

    enum Roles {
        IdRole,
        TitleRole,
        GameTitleRole,
        PosterRole,
        IconRole,
        LastPlayedRole,
        ResolvedTitleRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 gameId() const {
        return _mGameId;
    }

    void setGameId(qint64 gameId) {
        _mGameId = gameId;
        emit gameIdChanged();
        refresh();
    }

    Q_INVOKABLE qint64 getId(qsizetype index) {
        return _mData[index].installationId();
    }

signals:

    void gameIdChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    qint64 _mGameId;
    QList<InstallationRef> _mData;

};

}
