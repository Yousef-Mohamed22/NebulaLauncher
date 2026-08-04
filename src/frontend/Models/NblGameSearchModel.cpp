#include <Models/NblGameSearchModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblGameSearchModel::data(const QModelIndex& index, int role) const {

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

QHash<int, QByteArray> NblGameSearchModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


void NblGameSearchModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblGameSearchModel::refresh() {

    beginResetModel();

    _mData.clear();

    if(_mIncludeNewGame) {
        _mData.append(_Row(0, "+ New Game", {}, {}));
    }

    _mCurrentPage = 1;
    _mAllLoaded = false;
    _mExactMatch = false;
    exactMatchChanged();

    _fetchPageData();
    endResetModel();


}

void NblGameSearchModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = _mData.size();
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    nbl::StringView query = nbl::StringView(_mSearchQuery.data(), _mSearchQuery.length());

    QList<_Row> found;
    bool exactMatch;
    found.reserve(_mPageSize);

    nbl::SearchCriteria criteria = {
        .sortBy = nbl::SortBy::TITLE,
        .offset = offset,
        .limit = limit,
        .titleFilter = query
    };

    nbl::Result res = nbl::lookupGames(db, criteria, [&](const nbl::GameLookupResult& meta) -> nbl::Result {

        _Row ref(
                meta.id,
                QByteArray(meta.title.data, meta.title.length), 
                QByteArray(meta.poster.data, meta.poster.length),
                QByteArray(meta.icon.data, meta.icon.length)
        );

        found.append(ref);
        return nbl::Result::SUCCESS;

    });

    if(res.error()) {

        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading tags!");
        _mAllLoaded = true;
        _mIsLoading = false;
        emit allLoadedChanged();
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

    if(exactMatch && !_mExactMatch) {
        _mExactMatch = true;
        exactMatchChanged();
    }

    if(found.size() < limit) {
        _mAllLoaded = true;
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
