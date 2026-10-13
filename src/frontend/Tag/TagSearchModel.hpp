#pragma once

#include <Core/Types.hpp>
#include <Platform/Logging.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "Tag/TagRef.hpp"
#include "PagedListModel.hpp"

namespace nbl::frontend {

class TagSearchModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QByteArray searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool exactMatch READ exactMatch NOTIFY exactMatchChanged)

public:

    explicit TagSearchModel(QObject* parent = nullptr) :
        PagedListModel(parent)
    { 
        setPaused(true);
    }

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

    bool exactMatch() const { return _bExactMatch; }

    QByteArray searchQuery() {
        return _mSearchQuery;
    }

    void setSearchQuery(QByteArray searchQuery) {
        _mSearchQuery = searchQuery;
        searchQueryChanged();
        refresh();
    }

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

    void searchQueryChanged();
    void exactMatchChanged();


protected:

    virtual void _clear() override { 
        _mData.clear(); 
        if(_bExactMatch) {
            _bExactMatch = false;
            emit exactMatchChanged();
        }
    }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    bool _bExactMatch;
    QByteArray _mSearchQuery;

    QList<TagRef> _mData;

};

}
