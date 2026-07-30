#pragma once

#include <qstringview.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "NblTagRef.hpp"

class NblTagSearchModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QByteArray searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool exactMatch READ exactMatch NOTIFY exactMatchChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

public:

    explicit NblTagSearchModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false),
        _mPaused(true)
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

    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }
    bool exactMatch() const { return _mExactMatch; }
    bool paused() const  { return _mPaused; }

    void setPageSize(int size) { 
        _mPageSize = size; 
        pageSizeChanged();
    }

    void setPaused(bool paused) { 
        _mPaused = paused; 
        pausedChanged(); 
        if(!_mPaused) {
            loadNextPage();
        }
    }

    QByteArray searchQuery() {
        return _mSearchQuery;
    }

    void setSearchQuery(QByteArray searchQuery) {
        _mSearchQuery = searchQuery;
        searchQueryChanged();
        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

    Q_INVOKABLE inline qint64 id(qsizetype index) {
        return _mData[index].id;
    }
    Q_INVOKABLE inline QString name(qsizetype index) {
        return _mData[index].name;
    }
    Q_INVOKABLE inline QColor color(qsizetype index) {
        return _mData[index].color;
    }

signals:

    void pageSizeChanged();
    void isLoadingChanged();
    void searchQueryChanged();
    void exactMatchChanged();
    void pausedChanged();

private:

    void _fetchPageData();

    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;
    bool _mExactMatch;
    bool _mPaused;
    QByteArray _mSearchQuery;

    QList<NblTagRef> _mData;

};


