#pragma once

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Core/Result.hpp>

#include <QObject>
#include <QtQml>

class NblPagedListModel : public QAbstractListModel {

    Q_OBJECT

    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool allLoaded READ allLoaded NOTIFY allLoadedChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:

    explicit NblPagedListModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mPageSize(10),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }
    bool allLoaded() const { return _mAllLoaded; }

    void setPageSize(int size) { 
        _mPageSize = nbl::max(1, size);
    }

    int count() {
        return rowCount();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void pageSizeChanged();
    void isLoadingChanged();
    void allLoadedChanged();
    void countChanged();

protected:

    virtual void _clear() = 0;
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) = 0;

private:

    virtual void _fetchMore();

    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

};
