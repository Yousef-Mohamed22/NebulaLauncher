#include <Models/NblTagSearchModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblTagSearchModel::data(const QModelIndex& index, int role) const {

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

QHash<int, QByteArray> NblTagSearchModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


void NblTagSearchModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading && !_mPaused) {
        _fetchPageData();
    }
}

void NblTagSearchModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mAllLoaded = false;
    _mExactMatch = false;
    exactMatchChanged();

    loadNextPage();

    endResetModel();
}

void NblTagSearchModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = _mData.size();
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    nbl::StringView query = nbl::StringView(_mSearchQuery.data(), _mSearchQuery.length());

    QList<NblTagRef> found;
    bool exactMatch;
    found.reserve(_mPageSize);

    nbl::Result res;
    if(query.empty()) {
        res = nbl::enumerateTags(db, limit, offset, [&](nbl::TagMeta tag) -> nbl::Result {

            NblTagRef ref(tag.id, QString::fromUtf8(tag.name.data, tag.name.length), QColor::fromRgb(tag.color)); 
            found.append(ref);
            return nbl::Result::SUCCESS;

        });
    } else {
        res = nbl::searchTags(db, query, limit, offset, [&](nbl::TagMeta tag) -> nbl::Result {

            if(query == tag.name) {
                exactMatch = true;
            }

            NblTagRef ref(tag.id, QString::fromUtf8(tag.name.data, tag.name.length), QColor::fromRgb(tag.color)); 
            found.append(ref);
            return nbl::Result::SUCCESS;

        });
    }

    if(res.error()) {

        nbl::log(nbl::LogLevel::ERROR, "An error occurred while loading tags! ", res.value);
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
