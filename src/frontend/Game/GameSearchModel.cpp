#include "Game/GameSearchModel.hpp"
#include "ApplicationContext.hpp"

#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::GameSearchModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    _Row row = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return row.gId;
        case TitleRole:
            return row.title;
        case PosterRole:
            return row.poster;
        case IconRole:
            return row.icon;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::GameSearchModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


nbl::Result nbl::frontend::GameSearchModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    nbl::StringView query = nbl::StringView(_mSearchQuery.data(), _mSearchQuery.length());

    QList<_Row> found;
    bool exactMatch;
    found.reserve(limit);

    nbl::SearchCriteria criteria = {
        .sortBy = nbl::SortBy::TITLE,
        .offset = static_cast<u64>(offset),
        .limit = static_cast<u64>(limit),
        .titleFilter = query
    };

    NBL_CHECK(nbl::lookupGames(db, { .criteria = criteria }, [&](const nbl::GameLookupResult& meta) -> nbl::Result {

        _Row ref(
                meta.id,
                QByteArray(meta.title.data, meta.title.length), 
                meta.poster,
                meta.icon
        );

        found.append(ref);
        return nbl::Result::SUCCESS;
    }));


    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }

    if(exactMatch && !_bExactMatch) {
        _bExactMatch = true;
        exactMatchChanged();
    }

    return nbl::Result::SUCCESS;
}
