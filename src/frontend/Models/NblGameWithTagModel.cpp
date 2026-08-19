#include <Models/NblGameWithTagModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblGameWithTagModel::data(const QModelIndex& index, int role) const {

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

QHash<int, QByteArray> NblGameWithTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


void NblGameWithTagModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblGameWithTagModel::refresh() {

    beginResetModel();

    _mCriteriaData.tags[0] = static_cast<i64>(_mTag);
    _mCriteriaData.tagSets[0] = _mCriteriaData.tags;
    _mCriteria.tags = _mCriteriaData.tagSets.constSpan();
    _mCriteria.deviceId = ApplicationContext::instance()->deviceId();

    _mData.clear();
    _mAllLoaded = false;
    _mCurrentPage = 1;
    _fetchPageData();

    endResetModel();

}

void NblGameWithTagModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<_Row> found = {};
    found.reserve(_mPageSize);

    _mCriteria.offset = offset;
    _mCriteria.limit = limit;

    if(nbl::lookupGames(db, _mCriteria, [&](const nbl::GameLookupResult& res) -> nbl::Result {
        _Row ref{
            res.id,
            fromNbl(res.title),
            fromNbl(res.poster),
            fromNbl(res.icon)
        };
        found.append(ref);
        return nbl::Result::SUCCESS;
    }).error()) {
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
