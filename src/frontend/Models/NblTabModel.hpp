#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QtQml>

#include <Models/NblFilterRef.hpp>

class NblTabModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tabId READ tabId WRITE setTabId NOTIFY tabIdChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:

    explicit NblTabModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mTabId(0),
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    NblTabModel(i64 tabId, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mTabId(tabId),
        _mCurrentPage(1),
        _mPageSize(10),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    enum Roles {
        IdRole,
        NameRole,
        IconRole,
        ColorizeRole,
        MultiLineRole,
        ShowTitlesRole,
        UseIconsRole,
        ShowInstallationsRole,
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
    }

    qint64 tabId() const {
        return _mTabId;
    }

    void setTabId(qint64 fid) {
        _mTabId = fid;
        emit tabIdChanged();
        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void tabIdChanged();
    void totalRowsChanged();
    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();

private:

    void _fetchPageData();

    qint64 _mTabId;

    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

    QList<NblFilterRef> _mData;

};


