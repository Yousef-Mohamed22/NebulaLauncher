#pragma once

#include <Core/Types.hpp>
#include <Core/Array.hpp>
#include <Core/Span.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <qstringview.h>
#include <QObject>
#include <QColor>
#include <QtQml>

#include "NblInstallationRef.hpp"

class NblInstallationWithTagModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tag READ tag WRITE setTag NOTIFY tagChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool allLoaded READ allLoaded NOTIFY allLoadedChanged)

public:

    explicit NblInstallationWithTagModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mTag{},
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    NblInstallationWithTagModel(qint64 tag, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mTag(tag),
        _mCurrentPage(1),
        _mPageSize(10),
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

    quint64 currentPage() const { return _mCurrentPage; }
    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _mIsLoading; }
    bool allLoaded() const { return _mAllLoaded; }

    void setPageSize(int size) { _mPageSize = size; }

    qint64 tag() const {
        return _mTag;
    }

    void setTag(qint64 tag) {
        _mTag = tag;
        emit tagChanged();
        refresh();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void tagChanged();
    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();
    void allLoadedChanged();

private:

    void _fetchPageData();

    qint64 _mTag;
    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

    struct {
        nbl::Array<i64, 1> tags;
        nbl::Array<nbl::ConstSpan<i64>, 1> tagSets;
    } _mCriteriaData = {};
    nbl::InstallationSearchCriteria _mCriteria = {};

    QList<NblInstallationRef> _mData;

};


