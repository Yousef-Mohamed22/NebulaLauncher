#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Tag/TagRef.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class GameTagModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)

public:

    explicit GameTagModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(0)
    { }

    GameTagModel(i64 gameId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(gameId)
    { }

    enum Roles {
        IdRole,
        NameRole,
        ColorRole
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

    void setGameId(qint64 fid) {
        _mGameId = fid;
        emit gameIdChanged();
        refresh();
    }

signals:

    void gameIdChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    qint64 _mGameId;
    QList<TagRef> _mData;

};

}
