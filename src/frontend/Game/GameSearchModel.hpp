#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QtQml>

#include "PagedListModel.hpp"

namespace nbl::frontend {

class GameSearchModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QByteArray searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool exactMatch READ exactMatch NOTIFY exactMatchChanged)
    Q_PROPERTY(bool includeNewGame READ includeNewGame WRITE setIncludeNewGame NOTIFY includeNewGameChanged)

public:

    explicit GameSearchModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _bExactMatch(false),
        _bIncludeNewGame(false),
        _mSearchQuery{}
    { }

    enum Roles {
        IdRole,
        TitleRole,
        PosterRole,
        IconRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QByteArray searchQuery() { return _mSearchQuery; }
    bool exactMatch() const { return _bExactMatch; }
    bool includeNewGame() const { return _bIncludeNewGame; }

    void setIncludeNewGame(bool includeNewGame) { 
        _bIncludeNewGame = includeNewGame; 
        includeNewGameChanged();
        refresh(); 
    }
    void setSearchQuery(QByteArray searchQuery) {
        _mSearchQuery = searchQuery;
        searchQueryChanged();
        refresh();
    }

signals:

    void searchQueryChanged();
    void exactMatchChanged();
    void includeNewGameChanged();

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    struct _Row {
        qint64 gId;
        QByteArray title;
        QByteArray poster;
        QByteArray icon;
    };

    void _fetchPageData();

    bool _bExactMatch;
    bool _bIncludeNewGame;
    QByteArray _mSearchQuery;

    QList<_Row> _mData;

};

}
