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


void NblFilterModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblFilterModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mAllLoaded = false;
    _mCurrentPage = 1;
    _fetchPageData();

    endResetModel();

}

void NblFilterModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<_Item> found = {};
    found.reserve(_mPageSize);
    
    nbl::Result res;

    if(_mSearchInstallations) {
        res = nbl::lookupInstallationsForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {
            _Item item(res.installationId,
                fromNbl(res.title),
                fromNbl(res.poster),
                fromNbl(res.icon)
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        });    
    } else {
        res = nbl::lookupGamesForFilter(db, _mFilterId, ApplicationContext::instance()->deviceId(), offset, limit, [&](const nbl::GameLookupResult& res) -> nbl::Result {
            _Item item(res.id,
                fromNbl(res.title),
                fromNbl(res.poster),
                fromNbl(res.icon)
            );
            found.append(item);
            return nbl::Result::SUCCESS;
        });   
    }

    if(res.error()) {
        _mAllLoaded = true;
        emit allLoadedChanged();
        _mIsLoading = false;
        emit isLoadingChanged();
        return;
    }


    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }

    if(found.size() < limit) {
        _mAllLoaded = true;
        emit allLoadedChanged();
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
