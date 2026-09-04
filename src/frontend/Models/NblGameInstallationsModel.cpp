#include <Models/NblGameInstallationsModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant NblGameInstallationsModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    const NblInstallationRef& ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.installationId());
        case TitleRole:
            return ref.title();
        case GameTitleRole:
            return ref.gameTitle();
        case PosterRole:
            return ref.poster();
        case IconRole:
            return ref.icon();
        case LastPlayedRole:
            return static_cast<quint64>(ref.lastPlayed());
        case ResolvedTitleRole:
            return ref.resolvedTitle();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblGameInstallationsModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "installationId";
    roles[TitleRole] = "title";
    roles[GameTitleRole] = "gameTitle";
    roles[PosterRole] = "poster";
    roles[IconRole] = "icon";
    roles[LastPlayedRole] = "lastPlayed";
    roles[ResolvedTitleRole] = "resolvedTitle";
    return roles;
}


void NblGameInstallationsModel::loadNextPage() {

    if(!_mAllLoaded && !_mIsLoading) {
        _mCurrentPage++;
        emit currentPageChanged();
        _fetchPageData();
    }
}

void NblGameInstallationsModel::refresh() {

    beginResetModel();

    _mData.clear();
    _mAllLoaded = false;
    _mCurrentPage = 1;
    _fetchPageData();

    endResetModel();

}

void NblGameInstallationsModel::_fetchPageData() {

    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;

    emit isLoadingChanged();

    u64 offset = (_mCurrentPage - 1) * _mPageSize;
    u64 limit = _mPageSize;

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<NblInstallationRef> found = {};
    found.reserve(_mPageSize);

    if(nbl::getInstallationsOfGame(db, _mGameId, ApplicationContext::instance()->deviceId(), limit, offset, [&](const nbl::InstallationLookupResult& res) -> nbl::Result {
        NblInstallationRef ref(res.installationId,
            fromNbl(res.title),
            fromNbl(res.gameTitle),
            fromNbl(res.poster),
            fromNbl(res.icon),
            res.lastPlayed
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
        emit countChanged();
    }

    if(found.size() < limit) {
        _mAllLoaded = true;
        emit allLoadedChanged();
    }

    _mIsLoading = false;
    emit isLoadingChanged();
}
