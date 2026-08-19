#pragma once

#include <qstringview.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

class NblGameSearchModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool allLoaded READ allLoaded NOTIFY allLoadedChanged)
    Q_PROPERTY(QByteArray searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool exactMatch READ exactMatch NOTIFY exactMatchChanged)
    Q_PROPERTY(bool includeNewGame READ includeNewGame WRITE setIncludeNewGame NOTIFY includeNewGameChanged)

public:

    explicit NblGameSearchModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false),
        _mIncludeNewGame(false)
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

    quint64 currentPage() const { return _mCurrentPage; }
    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }
    bool allLoaded() const { return _mAllLoaded; }
    QByteArray searchQuery() { return _mSearchQuery; }
    bool exactMatch() const { return _mExactMatch; }
    bool includeNewGame() const { return _mIncludeNewGame; }

    void setPageSize(int size) { _mPageSize = size; }
    void setIncludeNewGame(bool includeNewGame) { 
        _mIncludeNewGame = includeNewGame; 
        includeNewGameChanged();
        refresh(); 
    }
    void setSearchQuery(QByteArray searchQuery) {
        _mSearchQuery = searchQuery;
        searchQueryChanged();
        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();
    void allLoadedChanged();
    void searchQueryChanged();
    void exactMatchChanged();
    void includeNewGameChanged();

private:

    struct _Row {
        qint64 gId;
        QByteArray title;
        QByteArray poster;
        QByteArray icon;
    };

    void _fetchPageData();

    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;
    bool _mExactMatch;
    bool _mIncludeNewGame;
    QByteArray _mSearchQuery;

    QList<_Row> _mData;

};


