#include <Models/NblAllTabsModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

#include "NblTabRef.hpp"

QVariant NblAllTabsModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblTabRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.tabId();
        case NameRole:
            return ref.name();
        case IconRole:
            return ref.icon();
        case ColorizeRole:
            return ref.colorizeIcon();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblAllTabsModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    roles[ColorizeRole] = "colorizeIcon";
    return roles;
}


void NblAllTabsModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblAllTabsModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mCurrentPage = 1;
    _mAllLoaded = false;
    _fetchPageData();

    endResetModel();

}

void NblAllTabsModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    if(db == nullptr) {
        _mIsLoading = false;
        emit isLoadingChanged();
        return;
    }
    
    QList<NblTabRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::enumerateTabs(db, offset, limit, [&](const nbl::TabMeta& tab) -> nbl::Result {
        NblTabRef ref(
            tab.tabId,
            QString::fromUtf8(tab.name.data, tab.name.length),
            QString::fromUtf8(tab.icon.data, tab.icon.length),
            tab.colorizeIcon,
            tab.hidden
        );

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
