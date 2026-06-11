#include "NblTabModel.hpp"

#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblTabModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblFilterRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.filterId());
        case NameRole:
            return ref.name();
        case IconRole:
            return ref.icon();
        case MultiLineRole:
            return ref.multiLine();
        case UseIconsRole:
            return ref.useIcons();
        case ShowTitlesRole:
            return ref.showTitles();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblTabModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    roles[MultiLineRole] = "multiLine";
    roles[ShowTitlesRole] = "showTitles";
    roles[UseIconsRole] = "useIcons";
    return roles;
}


void NblTabModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblTabModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mAllLoaded = false;
    _mCurrentPage = 1;
    _fetchPageData();

    endResetModel();

}

void NblTabModel::_fetchPageData() {

    if(_mIsLoading || _mAllLoaded) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<NblFilterRef> found = {};
    found.reserve(_mPageSize);
    if(nbl::getFiltersForTab(db, _mTabId, offset, limit, [](void* userData, const nbl::FilterMeta& meta) -> nbl::Result {
        NblFilterRef ref(meta.filterId, 
            QString::fromUtf8(meta.name.data, meta.name.length), 
            QString::fromUtf8(meta.icon.data, meta.icon.length),
            meta.multiLine,
            meta.showTitles,
            meta.useIcons
        );
        static_cast<QList<NblFilterRef>*>(userData)->append(ref);
        return nbl::Result::SUCCESS;
    }, &found).error()) {
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
    if(found.size() < _mPageSize) {
        _mAllLoaded = true;
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
