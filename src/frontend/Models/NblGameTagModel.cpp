#include <Models/NblGameTagModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblGameTagModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblTagRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.id;
        case NameRole:
            return ref.name;
        case ColorRole:
            return ref.color;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblGameTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


void NblGameTagModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblGameTagModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mCurrentPage = 1;
    _mAllLoaded = false;
    _fetchPageData();

    endResetModel();

}

void NblGameTagModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<NblTagRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::getGameTags(db, _mGameId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        NblTagRef ref(tag.id, QString::fromUtf8(tag.name.data, tag.name.length), QColor::fromRgb(tag.color)); 
        found.append(ref);
        return nbl::Result::SUCCESS;
    }).error()) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading tags!");
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
