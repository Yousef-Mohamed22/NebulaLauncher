#include <Models/NblInstallationTagModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblInstallationTagModel::data(const QModelIndex& index, int role) const {

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

QHash<int, QByteArray> NblInstallationTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


void NblInstallationTagModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblInstallationTagModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mCurrentPage = 1;
    _mAllLoaded = false;
    _fetchPageData();

    endResetModel();

}

void NblInstallationTagModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<NblTagRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::getInstallationTags(db, _mInstallationId, offset, limit, [&](nbl::StringView name, u32 color) -> nbl::Result {
        NblTagRef ref(QString::fromUtf8(name.data, name.length), QColor::fromRgb(color)); 
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
