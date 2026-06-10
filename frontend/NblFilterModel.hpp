#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QtQml>

#include "NblInstallationRef.hpp"

class NblFilterModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId WRITE setFilterId NOTIFY filterIdChanged)
    Q_PROPERTY(int totalRows READ totalRows NOTIFY totalRowsChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:

    explicit NblFilterModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mFilterId(0),
        _mTotalRows(0),
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false)
    { }

    NblFilterModel(i64 filterId, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mFilterId(filterId),
        _mTotalRows(0),
        _mCurrentPage(1),
        _mPageSize(10),
        _mIsLoading(false)
    { }

    enum Roles {
        IdRole,
        PosterRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    quint64 totalRows() const { return _mTotalRows; }
    quint64 currentPage() const { return _mCurrentPage; }
    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }

    void setPageSize(int size) { _mPageSize = size; }

    qint64 filterId() const {
        return _mFilterId;
    }

    void setFilterId(qint64 fid) {
        _mFilterId = fid;
        emit filterIdChanged();

        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void filterIdChanged();
    void totalRowsChanged();
    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();

private:

    void _fetchTotalCount();
    void _fetchPageData();

    qint64 _mFilterId;

    quint64 _mTotalRows;
    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;

    QList<NblInstallationRef> _mData;

};


