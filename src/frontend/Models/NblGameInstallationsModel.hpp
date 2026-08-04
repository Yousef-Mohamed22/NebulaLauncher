#pragma once

#include <Core/Types.hpp>
#include <Core/Array.hpp>
#include <Core/Span.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <qstringview.h>
#include <qtmetamacros.h>
#include <QObject>
#include <QColor>
#include <QtQml>

#include "NblInstallationRef.hpp"

class NblGameInstallationsModel : public QAbstractListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)
    Q_PROPERTY(quint64 currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(quint64 pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool allLoaded READ allLoaded NOTIFY allLoadedChanged)
    Q_PROPERTY(quint64 count READ count NOTIFY countChanged)

public:

    explicit NblGameInstallationsModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mGameId{},
        _mCurrentPage(0),
        _mPageSize(0),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    NblGameInstallationsModel(qint64 gameId, QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mGameId(gameId),
        _mCurrentPage(1),
        _mPageSize(10),
        _mIsLoading(false),
        _mAllLoaded(false)
    { }

    enum Roles {
        IdRole,
        TitleRole,
        GameTitleRole,
        PosterRole,
        IconRole,
        LastPlayedRole,
        ResolvedTitleRole
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
    quint64 count() const { return _mData.size(); }

    void setPageSize(int size) { _mPageSize = size; }

    qint64 gameId() const {
        return _mGameId;
    }

    void setGameId(qint64 gameId) {
        _mGameId = gameId;
        emit gameIdChanged();
        refresh();
    }

    Q_INVOKABLE qint64 getId(qsizetype index) {
        return _mData[index].installationId();
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void gameIdChanged();
    void currentPageChanged();
    void pageSizeChanged();
    void isLoadingChanged();
    void allLoadedChanged();
    void countChanged();

private:

    void _fetchPageData();

    qint64 _mGameId;
    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;
    QList<NblInstallationRef> _mData;

};


