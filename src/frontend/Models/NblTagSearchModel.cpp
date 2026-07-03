#include <Models/NblTagSearchModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblTagSearchModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblTagRef ref = _mData.at(index.row());

    switch(role) {
        case NameRole:
            return ref.name;
        case ColorRole:
            return ref.color;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblTagSearchModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


void NblTagSearchModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblTagSearchModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mCurrentPage = 1;
    _mAllLoaded = false;
    _mExactMatch = false;
    exactMatchChanged();

    _fetchPageData();
    endResetModel();


}

void NblTagSearchModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    nbl::StringView query = nbl::StringView(_mSearchQuery.length(), _mSearchQuery.data());

    struct _Data {
        nbl::StringView query;
        QList<NblTagRef> found;
        bool exactMatch;
    } data = { query, {}, false };

    data.found.reserve(_mPageSize);

    nbl::Result res;
    if(query.empty()) {
        res = nbl::enumerateTags(db, limit, offset, [](void* userData, nbl::StringView name, u32 color) -> nbl::Result {

            _Data* data = static_cast<_Data*>(userData);

            NblTagRef ref(QString::fromUtf8(name.data, name.length), QColor::fromRgb(color)); 
            data->found.append(ref);
            return nbl::Result::SUCCESS;

        }, &data);
    } else {
        res = nbl::searchTags(db, query, limit, offset, [](void* userData, nbl::StringView name, u32 color) -> nbl::Result {

            _Data* data = static_cast<_Data*>(userData);
            if(data->query == name) {
                data->exactMatch = true;
            }

            NblTagRef ref(QString::fromUtf8(name.data, name.length), QColor::fromRgb(color)); 
            data->found.append(ref);
            return nbl::Result::SUCCESS;

        }, &data);
    }

    if(res.error()) {

        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading tags!");
        _mAllLoaded = true;
        _mIsLoading = false;
        emit isLoadingChanged();
        return;
    }


    if(data.found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + data.found.size() - 1);
        _mData.reserve(data.found.size());
        for(auto& ref : data.found) {
            _mData.append(ref);
        }
        endInsertRows();
    }

    if(data.exactMatch && !_mExactMatch) {
        _mExactMatch = true;
        exactMatchChanged();
    }

    if(data.found.size() < limit) {
        _mAllLoaded = true;
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
