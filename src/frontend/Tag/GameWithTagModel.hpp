#pragma once

#include <Core/Types.hpp>
#include <Core/Array.hpp>
#include <Core/Span.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QColor>
#include <QtQml>

#include "PagedListModel.hpp"

namespace nbl::frontend {

class GameWithTagModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 tag READ tag WRITE setTag NOTIFY tagChanged)

public:

    explicit GameWithTagModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mTag{}
    {
        _mCriteriaData.tags[0] = 0;
        _mCriteriaData.tagSets[0] = _mCriteriaData.tags; 
    }

    GameWithTagModel(qint64 tag, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mTag(tag)
    { 
        _mCriteriaData.tags[0] = tag;
        _mCriteriaData.tagSets[0] = _mCriteriaData.tags;
        _mCriteria.tags = _mCriteriaData.tagSets;
    }

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

    qint64 tag() const {
        return _mTag;
    }

    void setTag(qint64 tag) {
        if(tag != _mTag) {
            
            _mTag = tag;
            
            _mCriteriaData.tags[0] = tag;
            _mCriteria.tags = _mCriteriaData.tagSets;

            emit tagChanged();
            refresh();
        }
    }

protected:

    virtual void _clear() override { _mData.clear(); }
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

signals:

    void tagChanged();

private:
    struct _Row {
        qint64 gId;
        QByteArray title;
        qint64 poster;
        qint64 icon;
    };

    qint64 _mTag;

    struct {
        nbl::Array<i64, 1> tags;
        nbl::Array<nbl::ConstSpan<i64>, 1> tagSets;
    } _mCriteriaData = {};
    nbl::SearchCriteria _mCriteria = {};

    QList<_Row> _mData;

};

}
