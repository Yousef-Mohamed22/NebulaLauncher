#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QtQml>

struct NblGameRef {
    quint64 gId;
    QString title;
    QString poster;
    QString icon;
};


class NblGameModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool includeNewGame READ includeNewGame WRITE setIncludeNewGame NOTIFY includeNewGameChanged)

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:

    explicit NblGameModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mIncludeNewGame(false),
        _mQuery{},
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false)
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

    bool includeNewGame() const { return _mIncludeNewGame; }
    QString query() const { return _mQuery; }
    quint64 currentPage() const { return _mCurrentPage; }
    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }

    void setPageSize(int size) { _mPageSize = size; }

    void setQuery(QString query) {
        if(query == _mQuery) return;

        _mQuery = query.toUtf8();
        emit queryChanged();

        refresh();
    }

    void setIncludeNewGame(bool includeNewGame) {
        if(includeNewGame == _mIncludeNewGame) return;

        _mIncludeNewGame = includeNewGame;
        emit includeNewGameChanged();

        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void includeNewGameChanged();
    void queryChanged();
    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();

private:

    void _fetchPageData();

    QByteArray _mQuery;

    bool _mIncludeNewGame;
    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

    QList<NblGameRef> _mData;

};


