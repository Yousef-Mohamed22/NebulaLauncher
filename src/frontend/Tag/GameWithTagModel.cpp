#include "Tag/GameWithTagModel.hpp"
#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::GameWithTagModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    const _Row& ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.gId);
        case TitleRole:
            return ref.title;
        case PosterRole:
            return ref.poster;
        case IconRole:
            return ref.icon;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::GameWithTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}



nbl::Result nbl::frontend::GameWithTagModel::_fetchPageData(qsizetype limit, qsizetype offset) {
 
    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<_Row> found = {};
    found.reserve(limit);

    _mCriteria.offset = offset;
    _mCriteria.limit = limit;

    NBL_CHECK(nbl::lookupGames(db, { .criteria = _mCriteria }, [&](const nbl::GameLookupResult& res) -> nbl::Result {
        _Row ref{
            res.id,
            fromNbl(res.title),
            res.poster,
            res.icon
        };
        found.append(ref);
        return nbl::Result::SUCCESS;
    }))

    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }

    return nbl::Result::SUCCESS;
}
