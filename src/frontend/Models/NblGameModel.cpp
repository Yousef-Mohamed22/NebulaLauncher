#include <Models/NblGameModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblGameModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblGameRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.gId;
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

QHash<int, QByteArray> NblGameModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


void NblGameModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblGameModel::refresh() {

    beginResetModel();

    _mData.clear();
    if(_mIncludeNewGame) {
        beginInsertRows(QModelIndex(), 0, 0);
        _mData.append(NblGameRef(0, "New Game", "", ""));
        endInsertRows();
    }

    _mCurrentPage = 1;
    _mAllLoaded = false;
    _fetchPageData();

    endResetModel();

}

void NblGameModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    nbl::GameSearchCriteria criteria = {
        .sortBy = nbl::GameSortBy::TITLE,
        .offset = offset,
        .limit = limit,
        .titleFilter = nbl::StringView(_mQuery.length(), _mQuery.data())
    };

    QList<NblGameRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::lookupGames(db, criteria, [](void* userData, const nbl::GameLookupResult& res) -> nbl::Result {
        NblGameRef ref(
                res.id,
                fromNbl(res.title),
                fromNbl(res.poster),
                fromNbl(res.icon)
        ); 
        static_cast<QList<NblGameRef>*>(userData)->append(ref);
        return nbl::Result::SUCCESS;
    }, &found).error()) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading games!");
        _mAllLoaded = true;
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
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
