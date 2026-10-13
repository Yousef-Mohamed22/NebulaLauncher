#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "PagedListModel.hpp"
#include "Tab/TabRef.hpp"

namespace nbl::frontend {

class TabModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

public:

    explicit TabModel(QObject* parent = nullptr) :
        PagedListModel(parent),
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

private:

    void _clear() override {
        _mData.clear();
    }
    nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

    quint64 _mCurrentPage;
    quint64 _mPageSize;
    bool _mIsLoading;
    bool _mAllLoaded;

    QList<TabRef> _mData;

};

}

