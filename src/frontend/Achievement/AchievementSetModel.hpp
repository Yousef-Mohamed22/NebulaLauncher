#pragma once

#include <qcolor.h>
#include <qtmetamacros.h>
#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QtQml>
#include "PagedListModel.hpp"


namespace nbl::frontend {

class AchievementSetModel : public PagedListModel {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 gameId READ gameId WRITE setGameId NOTIFY gameIdChanged)
    Q_PROPERTY(bool includeHidden READ includeHidden WRITE setIncludeHidden NOTIFY includeHiddenChanged)

public:

    AchievementSetModel(QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(0)
    { }

    AchievementSetModel(i64 setId, QObject* parent = nullptr) :
        PagedListModel(parent),
        _mGameId(setId)
    { }

    enum Roles {
        IdRole,
        GameIdRole,
        LastUpdatedRole,
        HiddenRole,
        AwardColorRole,
        NameRole,
        ProviderRole,
        ProviderIdRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return _mData.size();
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    qint64 gameId() const {
        return _mGameId;
    }

    void setGameId(qint64 id) {
        _mGameId = id;
        emit gameIdChanged();

        refresh();
    }

    bool includeHidden() const {
        return _bIncludeHidden;
    }

    void setIncludeHidden(bool includeHidden) {
        _bIncludeHidden = includeHidden;
        emit includeHiddenChanged();

        refresh();
    }

    Q_INVOKABLE quint64 getTotalAchievementCount();
    Q_INVOKABLE quint64 getTotalUnlockedAchievementCount();
    Q_INVOKABLE quint64 getSetAchievementCount(qsizetype setIndex);
    Q_INVOKABLE quint64 getSetUnlockedAchievementCount(qsizetype setIndex);

    Q_INVOKABLE bool hasAnyWinCondition();
    Q_INVOKABLE bool hasWinCondition(qsizetype setIndex);

signals:

    void gameIdChanged();
    void includeHiddenChanged();

protected:

    inline void _clear() override { _mData.clear(); }
    nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) override;

private:

    struct _Item {
        qint64 id;
        qint64 gameId;
        quint64 lastUpdated;
        bool hidden;
        QColor awardColor;
        QString name;
        QString provider;
        QString providerId;
    };

    qint64 _mGameId;
    bool _bIncludeHidden;
    QList<_Item> _mData;

};

}
