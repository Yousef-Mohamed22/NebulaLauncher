#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "NblTabRef.hpp"

class NblAllTabsModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:

    explicit NblAllTabsModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    enum Roles {
        IdRole,
        NameRole,
        IconRole,
        ColorizeRole
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

    void setPageSize(int size) { 
        _mPageSize = size; 
        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();

private:

    void _fetchPageData();

    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

    QList<NblTabRef> _mData;

};


