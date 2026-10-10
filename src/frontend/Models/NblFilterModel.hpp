#pragma once

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QtQml>
#include "NblPagedListModel.hpp"

class NblFilterModel : public NblPagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId WRITE setFilterId NOTIFY filterIdChanged)
    Q_PROPERTY(bool searchInstallations READ searchInstallations WRITE setSearchInstallations NOTIFY searchInstallationsChanged)

public:

    explicit NblFilterModel(QObject* parent = nullptr) :
        NblPagedListModel(parent),
        _mFilterId(0),
        _mSearchInstallations(false)
    { }

    NblFilterModel(i64 filterId, QObject* parent = nullptr) :
        NblPagedListModel(parent),
        _mFilterId(filterId),
        _mSearchInstallations(false)
    { }

    enum Roles {
        IdRole,
        TitleRole,
        PosterRole,
        IconRole,
        FilterInfoRole,
        FilterInfoIconRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool searchInstallations() const { return _mSearchInstallations; }


    qint64 filterId() const {
        return _mFilterId;
    }

    void setFilterId(qint64 fid) {
        if(fid == _mFilterId) return;
        _mFilterId = fid;
        emit filterIdChanged();

        refresh();
    }

    void setSearchInstallations(bool value) {
        if(_mSearchInstallations == value) return;
        _mSearchInstallations = value;
        emit searchInstallationsChanged();

        refresh();
    }

signals:

    void filterIdChanged();
    void searchInstallationsChanged();

protected:

    void _clear() override {
        _mData.clear();
    }
    nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    struct _Item {
        qint64 id;
        QString title;
        QString poster;
        QString icon;
        QString filterInfo;
        QString filterInfoIcon;
    };

    qint64 _mFilterId;
    bool _mSearchInstallations;

    QList<_Item> _mData;

};


