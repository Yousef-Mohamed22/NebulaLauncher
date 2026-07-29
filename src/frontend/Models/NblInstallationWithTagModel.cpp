#include <Models/NblInstallationWithTagModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblInstallationWithTagModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    NblInstallationRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.installationId());
        case TitleRole:
            return ref.title();
        case PosterRole:
            return ref.poster();
        case IconRole:
            return ref.icon();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblInstallationWithTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    return roles;
}


void NblInstallationWithTagModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblInstallationWithTagModel::refresh() {

    beginResetModel();

    _mCriteriaData.tag = nbl::JsonValue::fromString(nbl::StringView(_mTag.length(), _mTag.data()));
    _mCriteriaData.innerArray = nbl::JsonValue::fromArray(nbl::JsonArray(&_mCriteriaData.tag, 1));
    _mCriteriaData.outerArray = nbl::JsonArray(&_mCriteriaData.innerArray, 1);
    _mCriteria.tags = _mCriteriaData.outerArray;

    _mData.clear();
    _mAllLoaded = false;
    _mCurrentPage = 1;
    _fetchPageData();

    endResetModel();

}

void NblInstallationWithTagModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->connection();
    
    QList<NblInstallationRef> found = {};
    found.reserve(_mPageSize);

    _mCriteria.offset = offset;
    _mCriteria.limit = limit;

    if(nbl::lookupInstallations(db, _mCriteria, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {
        NblInstallationRef ref(res.installationId,
            fromNbl(res.title),
            fromNbl(res.poster),
            fromNbl(res.icon)
        );
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
