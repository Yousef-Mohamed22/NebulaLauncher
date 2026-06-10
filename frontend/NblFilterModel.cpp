#include "NblFilterModel.hpp"

#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblFilterModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblInstallationRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.installationId());
        case PosterRole:
            return ref.poster();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblFilterModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[PosterRole] = "poster";
    return roles;
}


void NblFilterModel::loadNextPage() {

    if(_mData.size() < _mTotalRows && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblFilterModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mCurrentPage = 1;
    _fetchTotalCount();
    _fetchPageData();

    endResetModel();

}

void NblFilterModel::_fetchTotalCount() {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();

    u64 count;
    if(nbl::countFilters(db, &count).error()) {
        _mTotalRows = 0;
    }

    _mTotalRows = count;
}

void NblFilterModel::_fetchPageData() {

    if(_mIsLoading) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<NblInstallationRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::lookupInstallationsForFilter(db, _mFilterId, [](void* userData, const nbl::InstallationMeta& meta) -> nbl::Result {
        NblInstallationRef ref(meta.id, 
            QString::fromUtf8(meta.title.data, meta.title.length), 
            QString::fromUtf8(meta.poster.data, meta.poster.length)
        );
        static_cast<QList<NblInstallationRef>*>(userData)->append(ref);
        return nbl::Result::SUCCESS;
    }, offset, limit, &found).error()) {
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

    _mIsLoading = false;
    emit isLoadingChanged();
}
