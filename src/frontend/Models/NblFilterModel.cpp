#include <Models/NblFilterModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include "Core/Result.hpp"

QVariant NblFilterModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    _Item item = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(item.id);
        case TitleRole:
            return item.title;
        case PosterRole:
            return item.poster;
        case IconRole:
            return item.icon;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblFilterModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


nbl::Result NblFilterModel::_fetchPageData(qsizetype limit, qsizetype offset) {


    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<_Item> found = {};
    found.reserve(limit);
    
    if(_mSearchInstallations) {
        NBL_CHECK(nbl::lookupInstallationsForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {
            _Item item(res.installationId,
                fromNbl(res.title),
                fromNbl(res.poster),
                fromNbl(res.icon)
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        }));    
    } else {
        NBL_CHECK(nbl::lookupGamesForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::GameLookupResult& res) -> nbl::Result {
            _Item item(res.id,
                fromNbl(res.title),
                fromNbl(res.poster),
                fromNbl(res.icon)
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        }));   
    }

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
